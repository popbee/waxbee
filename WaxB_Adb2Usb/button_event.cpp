/*
 * button_event.cpp
 *
 *  Created on: 2010-12-26
 *      Author: Bernard
 */

#include <stdint.h>

#include "console.h"
#include "extdata.h"
#include "button_event.h"
#include "pen_event.h"
#include "intuos2_button.h"
#include "strings.h"

/** Input tablet common logic and 'PenEvent' structure
 *
 * NOTE: Button::init() needs to be called at startup
 */
namespace Button
{
	static uint8_t	buttonEncoding;

	void init()
	{
		buttonEncoding = extdata_getValue8(EXTDATA_USB_BUTTON_ENCODING);
	}

	void output_button_event(Button::ButtonEvent& buttonEvent)
	{
		switch(buttonEncoding)
		{
			default:
			case EXTDATA_USB_BUTTON_ENCODING_NONE:
				// do nothing.
				break;

			case EXTDATA_USB_BUTTON_ENCODING_WACOM_INTUOS2_1218:
				Intuos2Button::send_intuos2_button_event(buttonEvent);
				break;
		}

		if(console::console_enabled)
		{
			console::printP(STR_BUTTON_NO);
			console::printNumber(buttonEvent.buttonNumber);
			console::print(" : ");
			switch(buttonEvent.state)
			{
				case Button::in:		console::printlnP(STR_IN); break;
				case Button::hovering:		console::printlnP(STR_HOVERING); break;
				case Button::down:		console::printlnP(STR_DOWN); break;
				case Button::held_down:		console::printlnP(STR_HELD_DOWN); break;
				case Button::up:		console::printlnP(STR_UP); break;
				case Button::out:		console::printlnP(STR_OUT); break;
			}
		}
	}
}
