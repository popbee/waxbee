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

#include "featureinclusion.h"

#include "avr_util.h"
#include "adb_codec.h"
#include "adb_controller.h"

#include "usb.h"
#include "led.h"
#include "console.h"
#include "extdata.h"
#include "serial.h"
#include "pen_event.h"
#include "touch_event.h"
#include "isdv4_serial.h"
#include "protocol4_serial.h"
#include "protocol5_serial.h"
#include "topaz_serial.h"
#include "gpioinit.h"
#include "pen_data_transform.h"
#include "frequency.h"
#include "debugproc.h"
#include "usb_util.h"
#include "strings.h"
#include "wacom_usb.h"

#include <avr/pgmspace.h>

void send_initial_event()
{
	Pen::PenEvent penEvent;
#ifdef TOUCH_SUPPORT
	Touch::TouchEvent touchEvent;
#endif
	penEvent.proximity = 0;
	penEvent.is_mouse = 0;

	Pen::input_pen_event(penEvent);
#ifdef TOUCH_SUPPORT
	if(Touch::touchEnabled())
	{
		touchEvent.touch = false;
		Touch::output_touch_event(touchEvent);
	}
#endif
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

#define CPUCLOCK		F_CPU    // 16000000

#define TIMER0_PRESCALER_DIVIDER	1024
#define TIMER0_PRESCALER_SETTING	(BITV(CS10, 1) | BITV(CS11, 0) | BITV(CS12, 1))


/** Configure timer 0 to generate a timer overflow interrupt (200Hz)
 * (every 5ms)
 */
static void configureTimer0()
{
	TCCR0A = 0x00;
	TCCR0B = TIMER0_PRESCALER_SETTING;
	TIMSK0 = (1<<TOIE0);
}

ISR(TIMER0_OVF_vect)
{
	// setup the timer0 to trigger again in 5ms or 200 times per second (200Hz)
	TCNT0 = (0x100 - ((CPUCLOCK / TIMER0_PRESCALER_DIVIDER) / 200));

#ifdef DEBUG_SUPPORT
	DebugProc::timer_5ms_intr();
#endif
	UsbUtil::timer_5ms_intr();
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

#ifdef INITGPIO_SUPPORT
	bool gpio_error = false;
#endif

	configureTimer0();

#ifdef INITGPIO_SUPPORT
	//------------------------------------
	// custom GPIO initialization
	//------------------------------------
	if(!GpioInit::customGpioInit(false))
	{
		// note that it failed.
		gpio_error = true;
	}
#endif

	serial_tablet = extdata_getValue8(EXTDATA_SERIAL_PORT) == EXTDATA_SERIAL_PORT_SLAVE_DIGITIZER;
#ifdef ADB_SUPPORT
	adb_tablet = extdata_getValue8(EXTDATA_ADB_PORT) == EXTDATA_ADB_PORT_SLAVE_DIGITIZER;

	if(adb_tablet)
		ADB::setup();
#endif
	// initialize the USB
	usb_init();

	WacomUsb::init();

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

#ifdef DEBUG_SUPPORT
	console::init();

	CONSOLE_PRINTLNP(STR_WAXBEE_WELCOME);
#endif

#ifdef INITGPIO_SUPPORT
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
#endif

	Pen::init();
	PenDataTransform::init();

#ifdef DEBUG_SUPPORT
	debug_processing = DebugProc::init();
#endif

	if(serial_tablet)
	{
		switch(extdata_getValue8(EXTDATA_SLAVE_PROTOCOL))
		{
#ifdef SERIAL_ISDV4_SUPPORT
			case EXTDATA_SLAVE_PROTOCOL_WACOM_ISDV4:
				isdv4_serial::init();
				break;
#endif
#ifdef SERIAL_PROTOCOL_IV_SUPPORT
			case EXTDATA_SLAVE_PROTOCOL_WACOM_PROTOCOL4:
				protocol4_serial::init();
				break;
#endif
#ifdef SERIAL_PROTOCOL_V_SUPPORT
			case EXTDATA_SLAVE_PROTOCOL_WACOM_PROTOCOL5:
				protocol5_serial::init();
				break;
#endif
#ifdef SERIAL_TOPAZ_SUPPORT
			case EXTDATA_SLAVE_PROTOCOL_TOPAZ:
				topaz_serial::init();
				break;
#endif
		}
	}

	// send a first packet to signify that there is no pen in proximity
	send_initial_event();

        //------------------------
        // Main Loop
        //------------------------

        while (1)
	{
#ifdef ADB_SUPPORT
        	if(adb_tablet)
        		ADB::adb_controller();
#endif
        	if(serial_tablet)
        		serial::serialPortProcessing();

        	UsbUtil::main_loop(); // repeats usb packets to prevent idle time
#ifdef DEBUG_SUPPORT
        	if(debug_processing)
        		DebugProc::processing();
#endif
	}
}

