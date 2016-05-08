/*
 * button_event.h
 *
 * Represents a "button" event.
 *
 *  Created on: 2011-08-10
 *      Author: Bernard
 */

#ifndef BUTTON_EVENT_H_
#define BUTTON_EVENT_H_

namespace Button
{
	enum ButtonState
	{
		in,
		hovering,
		down,
		held_down,
		up,
		out
	};

	struct ButtonEvent
	{
		uint8_t	buttonNumber;
		enum ButtonState state;

		/** X position within the rectangle area */
		float	x; // 0..1 (defaults to 0.5 if position not known)
		/** Y position within the rectangle area */
		float	y; // 0..1 (defaults to 0.5 if position not known)
	};
}

#endif /* BUTTON_EVENT_H_ */
