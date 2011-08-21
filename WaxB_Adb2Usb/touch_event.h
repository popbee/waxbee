/*
 * touch_event.h
 *
 * Represents a "finger touch" event.
 *
 *  Created on: 2011-08-10
 *      Author: Bernard
 */

#ifndef TOUCH_EVENT_H_
#define TOUCH_EVENT_H_

namespace Touch
{
	struct TouchEvent
	{
		bool touch;
		uint16_t f1_x;
		uint16_t f1_y;
		uint16_t f2_x;
		uint16_t f2_y;
	};

	void output_touch_event(Touch::TouchEvent& touchEvent);
	bool touchEnabled();

}

#endif /* TOUCH_EVENTS_H_ */
