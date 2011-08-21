/*
 * pen_events.cpp
 *
 *  Created on: 2010-12-26
 *      Author: Bernard
 */

#include <stdint.h>

#include "console.h"
#include "extdata.h"
#include "button_event.h"
#include "pen_event.h"
#include "pen_button_handler.h"
#include "wacom_usb.h"
#include "debugproc.h"
#include "strings.h"

/** Input tablet common logic and 'PenEvent' structure
 *
 * NOTE: PenButtonHandler::init() needs to be called at startup
 */
namespace PenButtonHandler
{
	static uint8_t currentButtonNumber;

	void init()
	{
		currentButtonNumber = -1;
	}

	/** activeArea : out param
	 *
	 *  0xB0 x1x1 y1y1 x2x2 y2y2 btnNo
	 *
	 *  (16 bit numbers are in 'little endian' mode)
	 */
	bool findPenButton(Pen::PenEvent& penEvent, ActiveArea& activeArea)
	{
		uint8_t totalLen = extdata_getLength8(EXTDATA_SLAVE_ACTIVE_AREAS);

		if(totalLen == 0)
			return false;

		uint16_t curRecord = extdata_getAddress(EXTDATA_SLAVE_ACTIVE_AREAS);
		uint8_t curLen = 0;

		while(curLen == totalLen)
		{
			switch(pgm_read_byte(curRecord))
			{
				case 0xB0: // rectangle ButtOn area (0xB0)
				{
					curLen += 10;
					activeArea.x1 = pgm_read_word(curRecord+1);
					if(penEvent.x < activeArea.x1)
						continue;

					activeArea.y1 = pgm_read_word(curRecord+3);
					if(penEvent.y < activeArea.y1)
						continue;

					activeArea.x2 = pgm_read_word(curRecord+5);
					if(penEvent.x < activeArea.x2)
						continue;

					activeArea.y2 = pgm_read_word(curRecord+7);
					if(penEvent.y < activeArea.y2)
						continue;

					activeArea.buttonNumber = pgm_read_byte(curRecord+9);

					return true;
				}
				default:
					console::printlnP(STR_UNRECOGNIZED_ACTIVE_AREA_RECORD_TYPE);
			}
		}
		return false;
	}

	void abortButton()
	{
		// "cancels" any "button" operation (if any pending)
	}

	void handle_pen_button_event(Pen::PenEvent& penEvent, ActiveArea& activeArea)
	{
		int buttonNumber = activeArea.buttonNumber;

		if(currentButtonNumber != -1)
		{
			if(currentButtonNumber != buttonNumber)
			{
				// abort previous button operation
				abortButton();
			}
		}
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
				penEvent.pressure = extdata_getValue16(EXTDATA_USB_PRESSURE_MAX);
				penEvent.touch = true;
				input_pen_event(penEvent);
				break;
			case Button::held_down: // pen moving on button while being pressed
				computeIntuos2ButtonXYPenPosition(buttonEvent, penEvent);
				penEvent.pressure = extdata_getValue16(EXTDATA_USB_PRESSURE_MAX);
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
