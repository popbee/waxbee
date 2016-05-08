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
#include "wacom_usb.h"

/** Input tablet common logic and 'PenEvent' structure
 *
 * NOTE: Pen::init() needs to be called at startup
 */
namespace Intuos2Button
{
	static uint16_t usbPressureMax;

	void init()
	{
		usbPressureMax = extdata_getValue16(EXTDATA_USB_PRESSURE_MAX);
	}

	static void computeIntuos2ButtonXYPenPosition(Button::ButtonEvent& buttonEvent, Pen::PenEvent& penEvent)
	{
		//TODO: find the right "formula" to compute intuos2 buttons locations and how many buttons there is...

		penEvent.y = ((uint16_t)(1000 * buttonEvent.y)) + 100;
		penEvent.x = (buttonEvent.buttonNumber-1) * 1200 + (((uint16_t)(1000 * buttonEvent.x)) + 100);
	}

	/**
	 * Caller must properly track the button states. State must go through "in" before any other states and
	 * invoke "out" when done.
	 **/
	void send_intuos2_button_event(Button::ButtonEvent& buttonEvent)
	{
		Pen::PenEvent penEvent;

		penEvent.eraser = false;
		penEvent.is_mouse = false;
		penEvent.button0 = false;
		penEvent.button1 = false;
		penEvent.tilt_x = 0;
		penEvent.tilt_y = 0;
		penEvent.rotation_z = 0;

		switch(buttonEvent.state)
		{
			case Button::in:  // in button
				// block current (protocol V) pen operation if any
				// and disallow any other pen operation while focusing on the button (take out of proximity)
				WacomUsb::begin_special_operation_protocol5();

				// set stylus (pen) in proximity
				computeIntuos2ButtonXYPenPosition(buttonEvent, penEvent);
				penEvent.proximity = true;
				penEvent.pressure = 0;
				penEvent.touch = false;
				input_pen_event(penEvent);
				break;
			case Button::hovering: // pen moving over button (unpressed)
				computeIntuos2ButtonXYPenPosition(buttonEvent, penEvent);
				penEvent.pressure = 0;
				penEvent.touch = false;
				input_pen_event(penEvent);
				break;
			case Button::down: // pen pressing down button
				computeIntuos2ButtonXYPenPosition(buttonEvent, penEvent);
				penEvent.pressure = usbPressureMax;
				penEvent.touch = true;
				input_pen_event(penEvent);
				break;
			case Button::held_down: // pen moving on button while being pressed
				computeIntuos2ButtonXYPenPosition(buttonEvent, penEvent);
				penEvent.pressure = usbPressureMax;
				penEvent.touch = true;
				input_pen_event(penEvent);
			case Button::up:  // pen releasing (up) button
				computeIntuos2ButtonXYPenPosition(buttonEvent, penEvent);
				penEvent.pressure = 0;
				penEvent.touch = false;
				input_pen_event(penEvent);
				break;
			case Button::out:  // pen getting out of button
				penEvent.proximity = false;
				input_pen_event(penEvent);

				// return to "normal" operation
				WacomUsb::end_special_operation_protocol5();
				break;
		}
	}
}
