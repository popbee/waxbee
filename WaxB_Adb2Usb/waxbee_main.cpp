/* This software contains portions from PJRC.COM. The following copyright
 * notice is attached to it:
 *-----------------------------------------------------------------------------
 * Copyright (c) 2008, 2010 PJRC.COM, LLC
 * http://www.pjrc.com/teensy/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *-----------------------------------------------------------------------------
 */

#include <math.h>
#include <avr/io.h>

#include <avr/interrupt.h>

#include "avr_util.h"
#include "waxbee_main.h"
#include "adb_codec.h"
#include "adb_controller.h"

#include "tables.h"
#include "usb.h"
#include "led.h"
#include "console.h"
#include "extdata.h"
#include "serial.h"
#include "pen_events.h"
#include "isdv4_serial.h"
#include "protocol4_serial.h"
#include "protocol5_serial.h"
#include "gpioinit.h"
#include "pen_events.h"
#include "frequency.h"

#include <avr/pgmspace.h>

#define TIMER0_PRESCALER_DIVIDER	1024
#define TIMER0_PRESCALER_SETTING	(BITV(CS10, 1) | BITV(CS11, 0) | BITV(CS12, 1))

void send_outofrange_event()
{
	Pen::PenEvent penEvent;

	penEvent.proximity = 0;
	penEvent.is_mouse = 0;

	send_pen_event(penEvent);
}

volatile uint8_t timer0_10ms;

bool adb_tablet;
bool serial_tablet;

void error_condition(uint8_t code)
{
	LED_OFF;

	while (1)
	{
		Frequency::delay_ms(2500);

		for(uint8_t i = code;i>0;i--)
		{
			LED_ON;
			Frequency::delay_ms(700);
			LED_OFF;
			Frequency::delay_ms(700);
		}
	}
}

int main(void)
{
	Frequency::poweronCpuFrequency(); // set default 8Mhz

	// make sure the LED is off to start with
	LED_CONFIG;
	LED_OFF;

	if(!extdata_init())
		error_condition(2);

	Frequency::setupCpuFrequency(); // set to 16Mhz if configured to do so

	bool gpio_error = false;

	//------------------------------------
	// custom GPIO initialization
	//------------------------------------
	if(!GpioInit::customGpioInit(false))
	{
		// note that it failed.
		gpio_error = true;
	}

	serial_tablet = extdata_getValue8(EXTDATA_SERIAL_PORT) == EXTDATA_SERIAL_PORT_SLAVE_DIGITIZER;
	adb_tablet = extdata_getValue8(EXTDATA_ADB_PORT) == EXTDATA_ADB_PORT_SLAVE_DIGITIZER;

	if(adb_tablet)
		ADB::setup();

	// initialize the USB
	usb_init();

	//------------------------------------
	// Enable Interrupts
	sei();
	//------------------------------------

	//------------------------------------------
	// wait for USB subsystem to be initialized
	//------------------------------------------

	while (!usb_configured())
	{
		/* blink fast while waiting */
		LED_TOGGLE;
		Frequency::delay_ms(30);
	}

	LED_ON;
	Frequency::delay_ms(1000); // 1 second delay to let the host react
	LED_OFF;

	console::init();

	//------------------------------------
	// custom GPIO initialization
	//------------------------------------
	if(gpio_error )
	{
		// rerun the execution just for the sake of printing messages
		GpioInit::customGpioInit(true);
		// halt and indefinitely blink led 3 times
		error_condition(3);
	}

	Pen::init();

	if(serial_tablet)
	{
		switch(extdata_getValue8(EXTDATA_SLAVE_PROTOCOL))
		{
			case EXTDATA_SLAVE_PROTOCOL_WACOM_ISDV4:
				isdv4_serial::init();
				break;
			case EXTDATA_SLAVE_PROTOCOL_WACOM_PROTOCOL4:
				protocol4_serial::init();
				break;
			case EXTDATA_SLAVE_PROTOCOL_WACOM_PROTOCOL5:
				protocol5_serial::init();
				break;
		}
	}
//        static double rad = 0;

	// send a first packet to signify that there is no pen in proximity
	send_outofrange_event();

        // Configure timer 0 to generate a timer overflow interrupt (100Hz)
        timer0_10ms = 0;
        TCCR0A = 0x00;
        TCCR0B = TIMER0_PRESCALER_SETTING;
        TIMSK0 = (1<<TOIE0);

        //------------------------
        // Main Loop
        //------------------------

        while (1)
	{
        	if(adb_tablet)
        		ADB::adb_controller();

        	if(serial_tablet)
        		serial::serialPortProcessing();

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
*/	}
}

ISR(TIMER0_OVF_vect)
{
	// setup the timer0 to trigger again in 10ms or 100 times per second (100Hz)
	TCNT0 = (0x100 - ((CPUCLOCK / TIMER0_PRESCALER_DIVIDER) / 100));
	timer0_10ms++;
}

