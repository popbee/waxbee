/*
 * pen_events.h
 *
 * This is the "universal" pen event structure and api.
 *
 *  Created on: 2010-12-26
 *      Author: Bernard
 */

#ifndef PEN_EVENTS_H_
#define PEN_EVENTS_H_

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

		uint16_t x;
		uint16_t y;
		uint16_t pressure;

		int16_t tilt_x; // this is signed
		int16_t tilt_y; // this is signed

		int16_t rotation_z;
	};

	void init();

	// helpers
	uint16_t compute_x_position(uint16_t sourcevalue);
	uint16_t compute_y_position(uint16_t sourcevalue);
	uint16_t compute_pressure(uint16_t sourcevalue);

	void send_pen_event(Pen::PenEvent& penEvent);
}

#endif /* PEN_EVENTS_H_ */
