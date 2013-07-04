/*
 * wacom.h
 *
 *  Created on: 2011-01-20
 *      Author: Bernard
 */

#ifndef WACOM_USB_H_
#define WACOM_USB_H_

#include "pen_event.h"
#include "touch_event.h"

namespace WacomUsb
{
	void init();

//	void send_protocol4_init_packet();
	void send_protocol4_packet(Pen::PenEvent& penEvent);

	void send_bamboo_pen_packet(Pen::PenEvent& penEvent);
	void send_bamboo_touch_packet(Touch::TouchEvent& touchEvent);

//	void send_protocol5_init_packet();
	void send_protocol5_packet(Pen::PenEvent& penEvent);
	void begin_special_operation_protocol5();
	void end_special_operation_protocol5();
}

#endif /* WACOM_USB_H_ */
