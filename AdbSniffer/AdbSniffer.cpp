// Arduino core libs
#include "WProgram.h"

#include <avr/interrupt.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


#define BITV(bit, val)  (val << bit)

#define BUFSIZE 		100
#define TIMINGBUFSIZE 		50

// keep PRESCALER_DIVIDER and PRESCALER_SETTING in sync
#define PRESCALER_DIVIDER	8
#define PRESCALER_SETTING	(BITV(CS10, 0) | BITV(CS11, 1) | BITV(CS12, 0))

#define CPUCLOCK		F_CPU    // 16000000

#define TIMER1DIVISION_FOR_US	(CPUCLOCK / 1000000 / PRESCALER_DIVIDER)

#define TOLERANCE_US		20
//------------------------------------------
// Apple Desktop Bus Specifications:

#define	ATTENTION_TIME_MIN_US	(560 - TOLERANCE_US) // support shorter times
#define	ATTENTION_TIME_MAX_US	1040

#define SYNC_TIME_MIN_US	(60 - TOLERANCE_US)
#define SYNC_TIME_MAX_US	70

#define	BIT_TIME_MIN_US		70
#define	BIT_TIME_MAX_US		130

#define	STOP_BIT_TIME_MAX_US	(BIT_TIME_MAX_US*70/100)

#define STOP2START_TIME_MIN_US	(140 - TOLERANCE_US)
#define STOP2START_TIME_MAX_US	260

#define SRQ_MAX_US		330
//------------------------------------------


namespace ADB
{
	enum decoderState
	{
		waitingForAttention,
		attention,
		sync,
		header,
		header_stop,
		stopToStart,
		start,
		data
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

	struct AdbPacket
	{
		union
		{
			struct
			{
				unsigned int serviceRequest:1;	// true if this command packet has a serviceRequest
				unsigned int errorCode:5;	// this is used only when the sniffer could not understand the ADB bus
			};
			byte statusRawByte;
		};

		union
		{
			struct
			{
				// order seems reversed, but it works
				unsigned int parameter:2;
				unsigned int command:2;
				unsigned int address:4;
			};
			byte headerRawByte;
		};
		byte data[8];
		byte datalen;
	} adbPacket;

	AdbPacket captureBuffer[BUFSIZE];
	int captureBufferIndex = 0;
	int captureBufferReadIndex = 0;

	bool recordtimings = false;
	uint16_t timingBufferUs[TIMINGBUFSIZE];
	int timingBufNdx = -30;

	decoderState state = waitingForAttention;

	#define RISING_EDGE		true
	#define FALLING_EDGE	false

	// To enhance precision, account for the time between when
	// the capture occurred and "now".
	// NOTE: this is disabled for now since I am not sure if it
	// could cause problems in extreme cases(?), so I left the
	// "simpler" implementation there. To try when other things
	// work nicely.
	#define ENHANCEDPRECISION

	//---------------------------------------------------------
	// ADB bus state
	bool waitingOn = FALLING_EDGE;
	bool pulselevel;  // level of the pulse that just finished and that we are analyzing
	uint16_t pulseTime;


	//---------------------------------------------------------
	// toggle led activity indicator
	int toggle_delay = 0;
	int toggle = 0;

	void pulseLed()
	{
		digitalWrite(13, HIGH);	// set the LED on
		digitalWrite(13, LOW);	// set the LED off
	}

	void toggleLed(int delay)
	{
		toggle_delay++;

		if(toggle_delay < delay)
			return;

		toggle_delay = 0;

		// make the led toggle on activity
		if(toggle)
		{
			digitalWrite(13, HIGH);	// set the LED on
		}
		else
		{
			digitalWrite(13, LOW);	// set the LED off
		}

		toggle = !toggle;
	}

	//---------------------------------------------------------

	void serialPrint8bit(byte value)
    {
    	if(value < 0x10)
    		Serial.print('0');

    	Serial.print(value, HEX	);
    }

	//---------------------------------------------------------

	void pushPacket(AdbPacket &adbPacket)
	{
		captureBuffer[captureBufferIndex] = adbPacket;
		captureBufferIndex++;
		if(captureBufferIndex == BUFSIZE)
			captureBufferIndex = 0;

		if(timingBufNdx < 0)
			timingBufNdx++;
	}

	void signalError(byte errorCode)
	{
		toggleLed(0);
		AdbPacket errorPacket;

		errorPacket.errorCode = errorCode;

		pushPacket(errorPacket);
	}

	//---------------------------------------------------------
	// decode ADB byte
	int8_t decodeBitNumber;
	byte* decodedByte;
	uint16_t decodeBitLowTime;

	void initDecodeByte(byte* bits)
	{
		decodedByte = bits;
		*decodedByte = 0;
		decodeBitNumber = 8;
	}

	ErrorStatus decodeByte()
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

	void recyclePacket(AdbPacket &adbPacket)
	{
		// recycle packet
		adbPacket.headerRawByte = 0;
		adbPacket.datalen = 0;
		adbPacket.statusRawByte = 0;
	}

	//---------------------------------------------------------
	// decode ADB 1..8 bytes data packet

	byte decodeData_byte;
	AdbPacket* decodeData_packet;

	void initDecodeData(AdbPacket* adbPacket)
	{
		adbPacket->datalen = 0;
		decodeData_packet = adbPacket;
		initDecodeByte(&decodeData_byte);
	}

	ErrorStatus decodeData()
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

		if(decodeData_packet->datalen >= 8)
		{
			signalError(19);

			// too much data!
			return failure;
		}

		decodeData_packet->data[decodeData_packet->datalen] = decodeData_byte;
		decodeData_packet->datalen++;

		initDecodeByte(&decodeData_byte);

		return unfinished;
	}

	void finishDecodeData()
	{
		// this is called after a "timeout" -- 
		// essentially the stop bit without any activity afterwards

		if(decodeData_packet->datalen > 0 && decodeBitNumber == 8)
			pushPacket(*decodeData_packet);

		state = waitingForAttention;
	}

	// only call this within an interrupt (where no other interrupts can occur)
	// (this is to avoid breaking the "temporary" 8 bit register when writing
	// the OCR1A 16 bit value
	void setTimeout(uint16_t us)
	{
		// "clear" interrupt flag (by setting it)
		sbi(TIFR1,OCF1A);

		if(us == 0)
		{
			// disable timeout interrupt
			cbi(TIMSK1,OCIE1A);
		}
		else
		{
			OCR1A = us * TIMER1DIVISION_FOR_US;
			// enable timeout interrupt
			sbi(TIMSK1,OCIE1A);
		}
	}

	void timeoutInterrupt()
	{
		// stop timeout
		setTimeout(0);

		// reset timer value
		TCNT1 = 0;

		uint16_t timeoutCounter = OCR1A;

		switch(state)
		{
		case stopToStart:
			// it appears that there is no data portion of the packet

			// dump what we have so far (the header only).
			pushPacket(adbPacket);

			break;
		case data:
			finishDecodeData();

		default:
			break;
		}

		state = waitingForAttention;

		if(recordtimings)
		{
			if(timingBufNdx < 0 || timingBufNdx == TIMINGBUFSIZE)
				return;

			timingBufferUs[timingBufNdx] = timeoutCounter/TIMER1DIVISION_FOR_US;

			timingBufNdx++;
		}
	}

	void inputCaptureInterrupt()
	{
#ifndef ENHANCEDPRECISION
		// simple reset of the timer
		// (not accounting for past time)
		TCNT1 = 0;
#endif

		if(waitingOn == FALLING_EDGE)
		{
			waitingOn = RISING_EDGE;
			sbi(TCCR1B, ICES1);
			pulselevel = HIGH;
		}
		else
		{
			waitingOn = FALLING_EDGE;
			cbi(TCCR1B, ICES1);
			pulselevel = LOW;
		}

		uint16_t capturedCounter = ICR1;

#ifdef ENHANCEDPRECISION
		// account for the time spent since the input capture
		TCNT1 = TCNT1 - capturedCounter;
#endif

		// adjust

		// time in microseconds (us)
		pulseTime = capturedCounter/TIMER1DIVISION_FOR_US;

		switch(state)
		{
			case waitingForAttention:
				if(pulselevel == HIGH)
				{
					state = attention;
				}

				break;

			case attention:
				if(pulselevel == HIGH)
				{
					state = waitingForAttention;	// not an attention (but not signaling an error here).
					break;
				}

				if(	pulseTime < ATTENTION_TIME_MIN_US ||
					pulseTime > ATTENTION_TIME_MAX_US)
				{
					state = waitingForAttention;	// not an attention (but not signaling an error here).
					break;
				}

				state = sync;
				break;

			case sync:
				if(pulselevel == LOW ||
					pulseTime < SYNC_TIME_MIN_US ||
					pulseTime > SYNC_TIME_MAX_US)
				{
	//				signalError(11);

					state = waitingForAttention;	// not a sync
					break;
				}

				recyclePacket(adbPacket);

				initDecodeByte(&adbPacket.headerRawByte);
				state = header;
				break;

			case header:
			{
				ErrorStatus status = decodeByte();

				switch(status)
				{
					case success: state = header_stop; break;
					case failure: 	state = waitingForAttention; break;
					case unfinished: break;
				}

				break;
			}
			case header_stop:
				if(pulseTime > SRQ_MAX_US)
				{
					signalError(12);
					state = waitingForAttention;	// invalid stop or SRQ
					break;
				}

				if(pulseTime > STOP_BIT_TIME_MAX_US)
				{
					// SRQ !!
					adbPacket.serviceRequest = true;
				}

				state = stopToStart;
				setTimeout(STOP2START_TIME_MAX_US); // set a timeout in case the device does not responds
				break;

			case stopToStart:
				if(pulseTime < STOP2START_TIME_MIN_US)
				{
					signalError(13);
					state = waitingForAttention;	// invalid stop2start
					break;
				}

				if(pulseTime > STOP2START_TIME_MAX_US)
				{
					// this "case" should be catch by the timeout interrupt
					// instead of here
					signalError(14);
					state = waitingForAttention;
					break;
				}

				state = start;
				break;

			case start:
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

					state = waitingForAttention;	// invalid start bit
				}

				if(pulseTime < decodeBitLowTime)
				{
					signalError(16);
					state = waitingForAttention;	// invalid start bit
				}

				state = data;
				initDecodeData(&adbPacket);
				break;
			}
			case data:
			{
				ErrorStatus status = decodeData();

				switch(status)
				{
					case success: state = waitingForAttention; break;
					case failure: 	state = waitingForAttention; break;
					case unfinished: break;
				}

				// the stop bit should trigger a timeout
				setTimeout(BIT_TIME_MAX_US);

				break;
			}
		}

/*		if(recordtimings)
		{
			if(timingBufNdx < 0 || timingBufNdx == TIMINGBUFSIZE)
				return;

			timingBufferUs[timingBufNdx] = capturedCounter/2;

			timingBufNdx++;
		}
*/	}

	int linepos = 0;

	void checknewline()
	{
		linepos++;

		if(linepos > 10)
		{
			Serial.println();
			linepos = 0;
		}
	}

	void setup()
	{
		// debug messages for the PC
		Serial.begin(115200);

		// LED
		pinMode(13, OUTPUT);

		// ADB data
		digitalWrite(8, LOW);
		pinMode(8, INPUT);

		// Setup the Timer1 and enable the Input Capture functionality

		TCCR1A =	BITV(WGM10, 0) |
				BITV(WGM11, 0) |
				BITV(COM1B0, 0) |
				BITV(COM1B1, 0) |
				BITV(COM1A0, 0) |
				BITV(COM1A1, 0);

		TCCR1B =	PRESCALER_SETTING |	// CS10, CS11, CS12
				BITV(WGM12, 0) |
				BITV(WGM13, 0) |
				BITV(ICES1, 0) |	// initial Input Capture Edge Select
				BITV(ICNC1, 0);		// Input Capture Noise Canceler

		TCCR1C =	BITV(FOC1B, 0) |
				BITV(FOC1A, 0);

		// enable interrupts

		TIMSK1 = 	BITV(TOIE1, 0) |	// Overflow Interrupt Enable
				BITV(OCIE1A, 0) |	// Output Compare A Interrupt Enable
				BITV(OCIE1B, 0) |	// Output Compare B Interrupt Enable
				BITV(ICIE1, 1);		// Input Capture Interrupt Enable
	}

	AdbPacket lastPacket;
	int lastPacketCount = 0;

	bool equalsLastPacket(AdbPacket &p)
	{
		if(p.headerRawByte != lastPacket.headerRawByte)
			return false;

		if(p.datalen != lastPacket.datalen)
			return false;

		if(p.statusRawByte != lastPacket.statusRawByte)
			return false;

		for(int i=0; i<p.datalen; i++)
		{
			if(p.data[i] != lastPacket.data[i])
				return false;
		}
		return true;
	}

	void dumpPacket(AdbPacket &p)
	{
		toggleLed(0);

		if(p.errorCode > 0)
		{
			Serial.print(" *Err:");

			Serial.print(p.errorCode, DEC);
			Serial.print("* ");

			checknewline();
			return;
		}

		Serial.print('[');
		switch(p.command)
		{
		case 0b11:
			Serial.print(p.address, DEC);
			Serial.print('T');
			Serial.print(p.parameter, HEX);
			break;
		case 0b10:
			Serial.print(p.address, DEC);
			Serial.print('L');
			Serial.print(p.parameter, HEX);
			break;
		case 0b00:
			if(p.parameter == 0b01)
			{
				Serial.print(p.address, DEC);
				Serial.print("F");
			}
			else if(p.parameter == 0b00)
			{
				Serial.print("Reset");
			}
			else
			{
				Serial.print('?');
				serialPrint8bit(p.headerRawByte);
			}
			break;
		case 0b01:
		default:
			Serial.print('?');
			serialPrint8bit(p.headerRawByte);
			break;
		}

		if(p.serviceRequest)
			Serial.print("SRQ");

		if(p.datalen > 0)
		    Serial.print(':');

		for(int i=0;i<p.datalen;i++)
		{
			serialPrint8bit(p.data[i]);
		}

		Serial.print(']');
		checknewline();
	}

	void dumpCount()
	{
		Serial.print("x");
		Serial.print(lastPacketCount, DEC);
		Serial.print(' ');
		checknewline();
	}

	void mainloop()
	{
		if(captureBufferIndex != captureBufferReadIndex)
		{
			AdbPacket p;
			p = captureBuffer[captureBufferReadIndex];
			captureBufferReadIndex++;
			if(captureBufferReadIndex == BUFSIZE)
				captureBufferReadIndex = 0;

			if(lastPacketCount > 0)
			{
				if(equalsLastPacket(p))
				{
					if(lastPacketCount == 100)
					{
						dumpCount();
						lastPacketCount = 0;
					}
					lastPacketCount++;
				}
				else
				{
					if(lastPacketCount > 1)
						dumpCount();
					dumpPacket(p);
					lastPacket = p;
					lastPacketCount = 1;
				}
			}
			else // lastPacketCount == 0
			{
				lastPacket = p;
				lastPacketCount = 1;

				dumpPacket(p);
			}
		}

		if(recordtimings && timingBufNdx == TIMINGBUFSIZE)
		{
			linepos = 0;

			Serial.println();
			Serial.println();
			Serial.print('[');

			for(int i=0;i<TIMINGBUFSIZE;i++)
			{
				Serial.print(timingBufferUs[i], DEC);
				if(i&1)
					Serial.print('/');
				else
					Serial.print('\\');
				checknewline();
			}

			Serial.print(']');
			Serial.println();

			timingBufNdx = -10;
		}
	}
}

//---------------------------------------------------------

//---------------------------------------------------------
// MAIN HOOKS

void setup()
{
	ADB::setup();
}

void loop()
{
	ADB::mainloop();
}

//---------------------------------------------------------
// Input Capture Interrupt

ISR(TIMER1_CAPT_vect)
{
	ADB::inputCaptureInterrupt();
}

ISR(TIMER1_COMPA_vect)
{
	ADB::timeoutInterrupt();
}
