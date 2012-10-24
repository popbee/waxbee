#include <avr/io.h>
#include <avr/pgmspace.h>

#include <avr/interrupt.h>

#include "avr_util.h"
#include "debugproc.h"
#include "extdata.h"
#include "led.h"
#include "usb.h"
#include "console.h"
#include "pen_event.h"
#include "strings.h"

#define MAXPACKET	40

static uint8_t buffer[MAXPACKET];

volatile uint8_t timer_10ms;

namespace DebugProc
{
	static enum
	{
		stopped,
		delay_before_start,
		performing_activity

	} state = stopped;

	uint8_t interval_10ms = 1000/10;

	uint16_t debugdata_addr;
	uint16_t debugdata_length;

	uint16_t addr_current;

	bool trigOnProxOut = false;
	uint8_t endpoint;
	uint8_t startDelaySeconds;
	uint8_t nbIterations;
	uint8_t pauseDelayMs;
	uint8_t packet_size;

	uint8_t startCountdownSeconds;
	uint8_t iterationsCountdown;

	bool is5ms = false;

	void timer_5ms_intr()
	{
		is5ms = !is5ms;
		if(is5ms)
			timer_10ms++;
	}

	static void trigger(bool repeating)
	{
		console::printlnP(STR_TRIGGER);

		state = delay_before_start;

		startCountdownSeconds = startDelaySeconds;

		if(!repeating)
			iterationsCountdown = nbIterations;

		interval_10ms = 1000/10; // wait 1 second
	}

	static void reset()
	{
		state = stopped;
	}

	/** return true if debugging processing enabled */
	bool init()
	{
		timer_10ms = 0;

		if(extdata_getLength16(EXTDATA_DEBUG_DATA) == 0)
			return false;

		debugdata_addr = extdata_getAddress(EXTDATA_DEBUG_DATA);
		debugdata_length = extdata_getLength16(EXTDATA_DEBUG_DATA);

		reset();

		if( pgm_read_byte(debugdata_addr) == 1)
		{
			addr_current = debugdata_addr+1;

			trigOnProxOut = pgm_read_byte(addr_current++) == 1?true:false;
			endpoint = pgm_read_byte(addr_current++);
			startDelaySeconds = pgm_read_byte(addr_current++);
			nbIterations = pgm_read_byte(addr_current++);
			pauseDelayMs = pgm_read_byte(addr_current++);
			packet_size = pgm_read_byte(addr_current++);

			if(packet_size > MAXPACKET)
			{
				console::printP(STR_DEBUG_PACKET_SIZE_TOO_LARGE_MAX);
				console::printNumber(MAXPACKET);
				console::println();
				return false;
			}

			console::printP(STR_DEBUG_ACTIVITY_ENABLED_SECONDS);
			console::printNumber(startDelaySeconds);
			console::println();

			if(!trigOnProxOut)
			{
				// trigger right-away
				trigger(false);
			}

			return true;
		}

		return false;
	}
	
	void proxOutTrigger()
	{
//		console::printlnP(STR_PROXOUTTRIGGER);

		if(!trigOnProxOut)
			return;

		trigger(false);
	}

	static void initActivity()
	{
		addr_current = debugdata_addr + 7;
		interval_10ms = pauseDelayMs; // / 10;
		LED_ON;
	}

	/** return true when activity done */
	static bool doActivity()
	{
		// do activity
		for(int i=0; i<packet_size; i++)
			buffer[i] = pgm_read_byte(addr_current++);

		usb_send_packet(buffer, packet_size, endpoint, 50);

		if(addr_current > debugdata_addr + debugdata_length)
		{
			// done.
			LED_OFF;
			return true;
		}

		// activity not finished
		return false;
	}

	void processing()
	{
		if(state == stopped) // be quick when not running
			return;

		if(timer_10ms < interval_10ms)
			return;
		
		timer_10ms = 0;

		switch(state)
		{
			case stopped:
				break;
			case delay_before_start:
			{
				console::printP(STR_DELAY_BEFORE_START_SECONDS);
				console::printNumber(startCountdownSeconds);
				console::println();

				startCountdownSeconds--;

				if(startCountdownSeconds > 0)
					return;

				initActivity();
				state = performing_activity;
				// fallthrough state performing_activity immediately
			}
			case performing_activity: // do activity
			{
				if(doActivity())
				{
					// activity done
					console::printP(STR_ACTIVITY_DONE);

					iterationsCountdown--;

					if(iterationsCountdown > 0)
					{
						console::printP(STR_REPEATING);
						console::println();

						trigger(true); // repeat
					}
					else
					{
						console::printP(STR_STOP);
						console::println();

						reset(); // stop
					}
				}
			}
		}
	}
}

/*		// the following generates a series of "pressured" strokes.
 *
 * 		if (timer0_10ms >= 2000/10)
		{
			LED_TOGGLE;

			timer0_10ms = 0;

			rad+=0.1;

			double radslow = rad / 30;
			double x = (2.0+cos(rad) + cos(radslow)) / 4.0;
			double y = (2.0+sin(rad*0.9) + sin(radslow*0.68)) / 4.0;

			double pressure = sin(rad*1.7);

			if(pressure < 0)
				pressure = 0;

			pressure = 0;

//			console::println("Test!");
//			console::flush();
//			send_wacom_packet_double(pressure, x, y);
			// send a second packet "close" to the first one to be taken into consideration
			// (else it seems something is "filtering" the position).
			// Need not exceed ~500ms it appears (for the Graphire3 on Windows at least)
//			send_wacom_packet_double(pressure, x, y);

			// stroke end ?
//			send_outofrange_event();
		}
*/




