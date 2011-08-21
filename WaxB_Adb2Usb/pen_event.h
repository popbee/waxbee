/*
 * pen_event.h
 *
 * This is the "universal" pen event structure and api.
 *
 *  Created on: 2010-12-26
 *      Author: Bernard
 */

#ifndef PEN_EVENT_H_
#define PEN_EVENT_H_

namespace Pen
{
	struct PenEvent
	{
		bool proximity:1;
		bool eraser:1;
		bool is_mouse:1; // 0=pen, 1=mouse
		bool button0:1;
		bool button1:1;

		bool touch:1; // pen is touching the surface

		uint16_t x;	// "slave" tablet coordinates
		uint16_t y;	// "slave" tablet coordinates
		uint16_t pressure; // "slave" tablet pressure value

		int16_t tilt_x; // this is signed
		int16_t tilt_y; // this is signed

		int16_t rotation_z; // this is signed
	};

	void init();

	void input_pen_event(Pen::PenEvent& penEvent);
}

#endif /* PEN_EVENT_H_ */
