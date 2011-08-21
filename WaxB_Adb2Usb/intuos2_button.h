/*
 * intuos2_button.h
 *
 * emulate pressing intuos2 buttons (menu strip)
 *
 * On the Intuos2, the buttons are on the active area. Contrary to the Ultrapad, the driver on the PC "knows" about
 * the various regions and interprets the raw pen data accordingly.
 *
 *  Created on: 2011-08-10
 *      Author: Bernard
 */

#ifndef INTUOS2_BUTTON_H_
#define INTUOS2_BUTTON_H_

#include "button_event.h"

namespace Intuos2Button
{
	void send_intuos2_button_event(Button::ButtonEvent& buttonEvent);
}

#endif /* INTUOS2_BUTTON_H_ */
