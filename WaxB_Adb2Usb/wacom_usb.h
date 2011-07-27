/*
 * wacom.h
 *
 *  Created on: 2011-01-20
 *      Author: Bernard
 */

#ifndef WACOM_USB_H_
#define WACOM_USB_H_

#include "pen_events.h"

namespace WacomUsb
{
//	void send_protocol4_init_packet();
	bool send_protocol4_packet(Pen::PenEvent& penEvent);

	bool send_bamboo_pen_packet(Pen::PenEvent& penEvent);
	void send_bamboo_touch_packet(Pen::TouchEvent& touchEvent);

//	void send_protocol5_init_packet();
	bool send_protocol5_packet(Pen::PenEvent& penEvent);
}

#endif /* WACOM_USB_H_ */
