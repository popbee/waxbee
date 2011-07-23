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
#include "debugproc.h"

#include <avr/pgmspace.h>

void send_initial_event()
{
	Pen::PenEvent penEvent;
	Pen::TouchEvent touchEvent;

	penEvent.proximity = 0;
	penEvent.is_mouse = 0;

	Pen::send_pen_event(penEvent);

	if(Pen::touchEnabled())
	{
		touchEvent.touch = false;
		Pen::send_touch_event(touchEvent);
	}
}

bool adb_tablet;
bool serial_tablet;
bool debug_processing;

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

	debug_processing = DebugProc::init();


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
	send_initial_event();

        //------------------------
        // Main Loop
        //------------------------

        while (1)
	{
        	if(adb_tablet)
        		ADB::adb_controller();

        	if(serial_tablet)
        		serial::serialPortProcessing();

        	if(debug_processing)
        		DebugProc::processing();
	}
}

