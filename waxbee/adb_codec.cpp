
/*
 * ADB Data line signaling interface.
 *
 * This emulates an ADB host.
 *
 * NOTE: Only meant to work with a single ADB device on the bus.
 */

//---------------------------------------------------------
// Usage:
//
// on startup the following needs to be called:
//
// ADB::setup()
//
// to initiate a transaction on the bus use:
//
// ADB::initiateAdbTransfer(AdbPacket* adbPacket, callbackfct);
//
// When finished, "callbackfct(uint8_t errorCode)" will be
// called
//
//---------------------------------------------------------

#include "featureinclusion.h"

#ifdef ADB_SUPPORT

#include <avr/interrupt.h>
#include "led.h"
#include "avr_util.h"
#include "console.h"
#include "adb_codec.h"
#include "strings.h"

#include <util/delay.h>

//---------------------------------------------------------
// General Mode of Operation - Single device ADB Host
//---------------------------------------------------------
//
// This emulates a ADB Host (like a Macintosh) but only
// to talk to a *single* ADB Device.
//
// SRQs are not implemented and might generate data errors.
// typically this won't harm much since the device is
// being queried over and over anyway.
//---------------------------------------------------------
// Hardware details:
//
// PC7 is connected to the single wire ADB bus.
//
// Timer 3 "Input Capture" is used to precisely read the port incoming "signal timings"
//
// Timer 3 "Output Capture A" is used to generate signals (internal interrupt only)
// and as a "timeout" when nothing occurs on the bus while waiting on input.
//
//---------------------------------------------------------
// Timing issue: The USB interrupt can delay the timer3 (output capture A) interrupt and create
// longer signal timings.  At first I thought of using Timer 4 "output compare" since
// it is driven on the same pin. This would have made the output timings rock solid.
// The problem is that "output compares" can only "drive" a port high or low.
// It does not control the direction bit to go in "input pull-up mode" instead
// of driving "high".
//
// In most cases, it is not a problem. The area where it could become an issue is
// when generating the bit timings, some logic may be added to compensate (when
// generating a 1, make sure the second pulse is effectively longer than the first).
//
// Another way of fixing it would be to schedule an interrupt *before* the maximum time
// the USB can block our interrupt (example 30us), then within the interrupt,
// wait for the right moment before toggling the port. This wastes quite some CPU
// time, but could be done only for the critical timings.
//---------------------------------------------------------


#define ADBDATA				(PORTC & (1<<7))

// keep PRESCALER_T3_DIVIDER and PRESCALER_T3_SETTING in sync
#define PRESCALER_T3_DIVIDER		8
#define PRESCALER_T3_SETTING		(BITV(CS30, 0) | BITV(CS31, 1) | BITV(CS32, 0))

#define CPUCLOCK			F_CPU     // 16000000 (or 8000000?)

#define TIMER3DIVISION_FOR_US		(CPUCLOCK / 1000000 / PRESCALER_T3_DIVIDER)

//------------------------------------------
// Apple Desktop Bus Specifications:
//
// these numbers are straight from the spec. document
//
#define	ATTENTION_TIME_MIN_US		560
#define	ATTENTION_TIME_MAX_US		1040

#define SYNC_TIME_MIN_US		60
#define SYNC_TIME_MAX_US		70

#define	BIT_TIME_MIN_US			70
#define	BIT_TIME_MAX_US			130

#define	STOP_BIT_TIME_MAX_US		(BIT_TIME_MAX_US*70/100)

#define STOP2START_TIME_MIN_US		85  // spec says 140 but GD tablet responds under 100us(!)
#define STOP2START_TIME_MAX_US		260

#define SRQ_MAX_US			330
//------------------------------------------

#define RESET_IDLE_US			300 // 10000 // 300
#define IDLE_TIME_US			50 // 10000 // 50
#define ATTENTION_TIME_US		800 // ATTENTION_TIME_MIN_US
#define SYNC_TIME_US			SYNC_TIME_MIN_US
#define BIT_0_LOW_TIME_US		65
#define BIT_0_HIGH_TIME_US		35
#define BIT_1_LOW_TIME_US		35
#define BIT_1_HIGH_TIME_US		65
#define STOP2START_TIME_US		150


namespace ADB
{
	enum codecState
	{
		inactive,
		idle,
		attention,
		sync,
		header,
		header_stop,
		// out
		stop_to_start_out,
		start_out,
		data_out,
		stop_out,
		// in
		header_stop_high_in,
		stop_to_start_in,
		start_in,
		data_in,
		finished
	};

	enum bitDecoderState
	{
		begin,
		end
	};

	enum ErrorStatus
	{
		unfinished,
		success,
		failure		// invalid signal -- reset decoder
	};

	bool proc_pending = false; // set to "true" to trigger processing

	codecState state = idle;

	/** current ADB packet being transfered (in/out) */
	struct AdbPacket* itsAdbPacket;

	#define RISING_EDGE	true
	#define FALLING_EDGE	false

	// To enhance precision, account for the time between when
	// the capture occurred and "now".
	// NOTE: this is disabled for now since I am not sure if it
	// could cause problems in extreme cases(?), so I left the
	// "simpler" implementation there. To try when other things
	// work nicely.
	#define ENHANCEDPRECISION

	#define COMPENSATION_DELAY_US	20

	//---------------------------------------------------------
	// ADB bus state //
	/** the "next" bus level the input capture is configured to trig on */
	bool waitingOn = FALLING_EDGE;
	/** level of the pulse that just finished and that we are analyzing */
	bool pulselevel;
	/** length of the last "pulse". Or the "captured" time on the previous edge. */
	uint16_t pulseTime;

	/** callback to invoke when done.  errorCode = 0 for success. */
	static void (*itsDoneCallback)(uint8_t errorCode) = 0;

	//---------------------------------------------------------
	// toggle led activity indicator
	int toggle_delay = 0;
	int toggle = 0;

	void toggleLed(int delay)
	{
		toggle_delay++;

		if(toggle_delay < delay)
			return;

		toggle_delay = 0;

		// make the led toggle on activity
		if(toggle)
		{
			LED_ON;
		}
		else
		{
			LED_OFF;
		}

		toggle = !toggle;
	}

	static inline void adb_businit()
	{
		cbi(PORTC,7);	// input
		cbi(DDRC,7);	// Hi-Z

		// DEBUG port
		sbi(DDRC,6);
		cbi(PORTC,6);
	}

	static inline void adb_forcelow()
	{
		cbi(PORTC,7);	// low
		sbi(DDRC,7); 	// output low
	}

	/** Hi-Z requires a ~1k pullup on the bus */
	static inline void adb_highimpedance()
	{
		cbi(PORTC,7);	// low
		cbi(DDRC,7); 	// input (Hi-Z)
	}

	static inline void debug_pulse()
	{
		sbi(PORTC,6);
		_delay_us(1);
		cbi(PORTC,6);
	}

	//---------------------------------------------------------

	static void resetDecoder();

	static void trigFinished()
	{
		if(itsDoneCallback)
			itsDoneCallback(0);

		resetDecoder();
	}

	static void signalError(byte errorCode)
	{
//		toggleLed(0);

//		console::print("Error code ");
//		console::printNumber(errorCode);
//		console::println();

		if(itsDoneCallback)
			itsDoneCallback(errorCode);
	}

	//---------------------------------------------------------

	static inline void stopTimeout()
	{
		// disable timeout interrupt
		cbi(TIMSK3,OCIE3A);
	}

	uint16_t timeout_target;

	static void setTimeout(uint16_t us)
	{
		timeout_target = us * TIMER3DIVISION_FOR_US;

		__BEGIN_CRITICAL_SECTION

		// "clear" interrupt flag (by setting it)
		sbi(TIFR3,OCF3A);

		// suppress interrupts to avoid breaking the
		// "temporary" 8 bit register when writing
		// the OCR3A 16 bit value

#ifdef COMPENSATION_DELAY_US
		if((timeout_target + 10) < (COMPENSATION_DELAY_US * TIMER3DIVISION_FOR_US))
		{
			OCR3A = 10;
		}
		else
		{
			OCR3A = timeout_target - (COMPENSATION_DELAY_US * TIMER3DIVISION_FOR_US);
		}
#else
		OCR3A = timeout_target;
#endif
		__END_CRITICAL_SECTION

		// enable timeout interrupt
		sbi(TIMSK3,OCIE3A);
	}

	//---------------------------------------------------------
	// encode ADB byte
	static int8_t encodedBits;
	static byte byteToEncode;

	static void initEncodeByte(byte bits)
	{
		byteToEncode = bits;
		encodedBits = 0;
		pulselevel = HIGH;
	}

	static ErrorStatus encodeByte()
	{
		if(encodedBits == 8)
		{
			return success;
		}

		if(pulselevel == HIGH) // level is currently HIGH
		{
			// start bit
			adb_forcelow();
			pulselevel = LOW;
			if(byteToEncode & 0x80)
				setTimeout(BIT_1_LOW_TIME_US);
			else
				setTimeout(BIT_0_LOW_TIME_US);
		}
		else // level is currently LOW
		{
			// bit phase 2
			adb_highimpedance();
			pulselevel = HIGH;

			if(byteToEncode & 0x80)
				setTimeout(BIT_1_HIGH_TIME_US);
			else
				setTimeout(BIT_0_HIGH_TIME_US);

			byteToEncode <<= 1;
			encodedBits++;
		}

		return unfinished;
	}


	//---------------------------------------------------------
	// encode ADB 1..8 bytes data packet

	static byte encodeData_byteindex;

	static void initEncodeData()
	{
		encodeData_byteindex = 0;

		initEncodeByte(itsAdbPacket->data[0]);
	}

	static ErrorStatus encodeData()
	{
		ErrorStatus byteStatus = encodeByte();

		switch(byteStatus)
		{
			case failure:
				return failure;
			case unfinished:
				return unfinished;
			case success:
				break;
		}

		encodeData_byteindex++;

		if(encodeData_byteindex == itsAdbPacket->datalen)
		{
			return success;
		}

		initEncodeByte(itsAdbPacket->data[encodeData_byteindex]);
		return encodeByte();
	}

	//---------------------------------------------------------
	// decode ADB byte
	static int8_t decodeBitNumber;
	static byte* decodedByte;
	static uint16_t decodeBitLowTime;

	static void initDecodeByte(byte* bits)
	{
		decodedByte = bits;
		*decodedByte = 0;
		decodeBitNumber = 8;
	}

	static ErrorStatus decodeByte()
	{
		if(pulselevel == LOW)
		{
			decodeBitLowTime = pulseTime;
			return unfinished;
		}

		uint16_t bittime = decodeBitLowTime + pulseTime;

		if(bittime < BIT_TIME_MIN_US)
		{
			signalError(17);	// bit time too small
			return failure;
		}

		if(bittime > BIT_TIME_MAX_US)
		{
			signalError(18);	// bit time too long
			return failure;
		}

		*decodedByte <<= 1;

		if(decodeBitLowTime < pulseTime)
			*decodedByte |= 1;

		decodeBitNumber--;

		if(decodeBitNumber == 0)
		{
			return success;
		}

		return unfinished;
	}

	//---------------------------------------------------------
	// decode ADB 1..8 bytes data packet

	static byte decodeData_byte;

	static void initDecodeData()
	{
		itsAdbPacket->datalen = 0;
		initDecodeByte(&decodeData_byte);
	}

	static ErrorStatus decodeData()
	{
		ErrorStatus byteStatus = decodeByte();
		switch(byteStatus)
		{
		case failure:
			return failure;
		case unfinished:
			return unfinished;
		case success:
			break;
		}

		if(itsAdbPacket->datalen >= 8)
		{
			signalError(19);

			// too much data!
			return failure;
		}

		itsAdbPacket->data[itsAdbPacket->datalen] = decodeData_byte;
		itsAdbPacket->datalen++;

		initDecodeByte(&decodeData_byte);

		return unfinished;
	}

	static void resetDecoder()
	{
		// reset whatever requires reseting at
		// any point in the ADB subsystem

		__BEGIN_CRITICAL_SECTION

		// reset Timer 3 counter
		TCNT3 = 0;

		__END_CRITICAL_SECTION

		// abort/disable input capture interrupt
		cbi(TIMSK3, ICIE3);
		// configure "falling edge" for the next capture
		cbi(TCCR3B, ICES3);

		// abort/disable timeout timer
		stopTimeout();
	}

	/**
	 public entry point to initiate an ADB transaction on the bus
	*/
	void initiateAdbTransfer(AdbPacket* adbPacket, void (*done_callback)(uint8_t errorCode))
	{
		// reset
		resetDecoder();

		// keep reference to the packet
		itsAdbPacket = adbPacket;

		itsDoneCallback = done_callback;

		// keep the line idle for at least IDLE_TIME_US
		// TODO: remember how much time left since last packet
		// and adjust setTimeout accordingly
		state = idle;
		adb_highimpedance();
		setTimeout(IDLE_TIME_US);
	}

	static void restartAdbTransaction()
	{
		adb_forcelow();
		state = attention;
		setTimeout(RESET_IDLE_US);
	}

	static void errorAndResetDecoder(byte value)
	{
		resetDecoder();

		signalError(value);

		state = inactive;
	}


	static void timeoutInterrupt()
	{
#ifdef COMPENSATION_DELAY_US
		// hold interrupts until the "real" timeout time has been reached
		while(TCNT3 < timeout_target) { ; }
#endif

#ifndef ENHANCEDPRECISION
		// simple reset of the timer
		// (not accounting for past time)
		TCNT3 = 0;
#endif
		// "clear" interrupt flag (by setting it)
		sbi(TIFR3,OCF3A);

		// disable timeout interrupt
		cbi(TIMSK3,OCIE3A);

#ifdef ENHANCEDPRECISION
		// account for the time spent since the interrupt occurred
		TCNT3 = TCNT3 - timeout_target;
#endif

		switch(state)
		{
			case inactive:
				// nothing to do here
				break;
			case idle:
				// check if bus appears "ready" (high)
				//
/*				if(ADBDATA == 0)
				{
					debug_pulse();
					// bus not ready.
					// try later on.
					setTimeout(50);
					break;
				}
*/
				// begin attention
				adb_forcelow();
				setTimeout(ATTENTION_TIME_US);
				state = attention;
				break;

			case attention:
				// begin sync
				adb_highimpedance();
				setTimeout(SYNC_TIME_US);
				initEncodeByte(itsAdbPacket->headerRawByte);
				state = sync;
				break;
			case sync:
				state = header;
				// fall through header state
			case header:
			{
				ErrorStatus status = encodeByte();

				if(status == success)
				{
					// begin header stop

					// TODO: handle SRQs: (probably not needed)

					adb_forcelow();
					setTimeout(BIT_0_LOW_TIME_US);
					state = header_stop;
					break;
				}
				else if(status == failure)
				{
					restartAdbTransaction();
				}
				break;
			}
			case header_stop:
			{
				adb_highimpedance(); // release bus

				if(itsAdbPacket->command == ADB_COMMAND_TALK)
				{
					// prepare for reading:

					// wait till the end of the stop bit
					// before starting probing the bus

					setTimeout(BIT_0_HIGH_TIME_US); // wait a little before switching on the input capture
					state = header_stop_high_in;
					break;
				}
				else
				{
					// start outputting (encoding) data bytes

					setTimeout(STOP2START_TIME_US);
					state = stop_to_start_out;
					break;
				}
			}
			case stop_to_start_out:
			{
				// begin start bit
				adb_forcelow();
				setTimeout(BIT_1_LOW_TIME_US);
				state = start_out;
				break;
			}
			case start_out:
				// continue start bit
				adb_highimpedance();
				setTimeout(BIT_1_HIGH_TIME_US);
				initEncodeData();
				state = data_out;
				break;
			case data_out:
			{
				ErrorStatus status = encodeData();

				if(status == success)
				{
					// begin stop bit
					adb_forcelow();
					setTimeout(BIT_0_LOW_TIME_US);
					state = stop_out;
				}
				else if(status == failure)
				{
					restartAdbTransaction();
				}

				break;
			}
			case stop_out:
			{
				adb_highimpedance();
				setTimeout(BIT_0_HIGH_TIME_US);
				state = finished;
				break;
			}
			case finished:
			{
				state = inactive;
				trigFinished();
				break;
			}

			// =======================================
			// "_in" states: reading bits from device
			// or more specifically inactivity timeouts
			// =======================================

			case header_stop_high_in:
				//---------------------------
				// Turn on the Input Capture
				//---------------------------

				// setup to trig on the next falling edge
				cbi(TCCR3B, ICES3);
				waitingOn = FALLING_EDGE;

				// clear Input Capture Flag by setting it
				sbi(TIFR3, ICF3);

				// enable input capture interrupt
				sbi(TIMSK3, ICIE3);

				setTimeout(STOP2START_TIME_MAX_US);
				state = stop_to_start_in;
				break;
			case stop_to_start_in:
				// it appears that there is no response from the device
				trigFinished();
				break;
			case data_in:
				// this is called essentially after the stop bit without
				// any activity afterwards. This denotes the end of a packet

				if(itsAdbPacket->datalen > 0 && decodeBitNumber == 8)
				{
					// all ok
					trigFinished();
				}
				else
				{
					// error condition
					errorAndResetDecoder(20); // no activity timeout while receiving data
				}

				state = inactive;
				// reset timer value
				break;
			default:
				state = idle;
			break;
		}

	}

	static void inputCaptureInterrupt()
	{
#ifndef ENHANCEDPRECISION
		// simple reset of the timer
		// (not accounting for past time)
		TCNT3 = 0;
#endif
		//------------------------------------------------
		// toggle input config to wait for the next "edge"
		//------------------------------------------------

		if(waitingOn == FALLING_EDGE)
		{
			waitingOn = RISING_EDGE;
			sbi(TCCR3B, ICES3);
			pulselevel = HIGH;
		}
		else
		{
			waitingOn = FALLING_EDGE;
			cbi(TCCR3B, ICES3);
			pulselevel = LOW;
		}

		uint16_t capturedCounter = ICR3;

#ifdef ENHANCEDPRECISION
		// account for the time spent since the input capture
		TCNT3 = TCNT3 - capturedCounter;
#endif

		// adjust

		// time in microseconds (us)
		pulseTime = capturedCounter/TIMER3DIVISION_FOR_US;

		switch(state)
		{
			default:
				signalError(21); // invalid internal state on input capture interrupt
				break;
			case stop_to_start_in:
				if(pulseTime < STOP2START_TIME_MIN_US)
				{
					errorAndResetDecoder(13); // invalid stop2start (too small)
#ifdef DEBUG_SUPPORT
					CONSOLE_PRINTP(STR_STOP2START_TIME_TOO_SMALL);
					console::printNumber(pulseTime);
					console::println(" us");
#endif
					break;
				}

				if(pulseTime > STOP2START_TIME_MAX_US)
				{
					// the timeout interrupt should catch this "case" instead
					// of here, but it is safer to handle it here as well
					errorAndResetDecoder(14); // invalid stop2start (too large)
					break;
				}

				state = start_in;
				break;

			case start_in:
			{
				if(pulselevel == LOW)
				{
					decodeBitLowTime = pulseTime;
					break;
				}

				uint16_t bittime = decodeBitLowTime + pulseTime;

				if(bittime < BIT_TIME_MIN_US ||
						bittime > BIT_TIME_MAX_US)
				{
					signalError(15);
					state = idle;	// invalid start bit
				}

				if(pulseTime < decodeBitLowTime)
				{
					signalError(16);
					state = idle;	// invalid start bit
				}

				state = data_in;
				initDecodeData();
				break;
			}
			case data_in:
			{
				ErrorStatus status = decodeData();

				switch(status)
				{
					case success:	state = idle; break;
					case failure: 	state = idle; break;
					case unfinished: break;
				}

				// the stop bit should trigger a timeout
				setTimeout(BIT_TIME_MAX_US);

				break;
			}
		}
	}

	void setup()
	{
		adb_businit();
		adb_highimpedance();

		cbi(MCUCR, PUD);	// enable pull-ups

		// Setup the Timer3

		TCCR3A =	BITV(WGM30, 0) |
				BITV(WGM31, 0) |
				BITV(COM3C0, 0) |
				BITV(COM3C1, 0) |
				BITV(COM3B0, 0) |
				BITV(COM3B1, 0) |
				BITV(COM3A0, 0) |
				BITV(COM3A1, 0);

		TCCR3B =	PRESCALER_T3_SETTING |	// CS30, CS31, CS32
				BITV(WGM32, 0) |
				BITV(WGM33, 0) |
				BITV(ICES3, 0) |	// initial Input Capture Edge Select (0=falling edge)
				BITV(ICNC3, 0);		// Input Capture Noise Canceler

		TCCR3C =	BITV(FOC3B, 0) |
				BITV(FOC3A, 0);

		// timer3 input capture additional config:

		sbi(TIFR3, ICF3); // clear interrupt flag (by writing a one)
		cbi(TIMSK3, ICIE3); // disable input capture interrupt
	}
}

//---------------------------------------------------------


//---------------------------------------------------------
// Input Capture Interrupt

ISR(TIMER3_CAPT_vect)
{
	ADB::inputCaptureInterrupt();
}

ISR(TIMER3_COMPA_vect)
{
	ADB::timeoutInterrupt();
}
#endif
