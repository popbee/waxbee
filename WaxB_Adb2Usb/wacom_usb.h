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
	void send_protocol4_packet(Pen::PenEvent& penEvent);

	void send_bamboo_packet(Pen::PenEvent& penEvent);

//	void send_protocol5_init_packet();
	void send_protocol5_packet(Pen::PenEvent& penEvent);
}

#endif /* WACOM_USB_H_ */
