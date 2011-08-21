/*
 * pen_data_transform.h
 *
 * Pen data transformation between the native and usb tablet
 *
 *  Created on: 2011-08-09
 *      Author: Bernard
 */

#ifndef PEN_DATA_TRANSFORM_H_
#define PEN_DATA_TRANSFORM_H_

#include "pen_event.h"

namespace PenDataTransform
{
	struct XformedData
	{
		uint16_t x;	// "usb" tablet coordinates
		uint16_t y;	// "usb" tablet coordinates
		uint16_t pressure; // "usb" tablet pressure value

		int16_t tilt_x; // this is signed
		int16_t tilt_y; // this is signed

		int16_t rotation_z; // this is signed
	};

	void init();

	void transform_pen_data(Pen::PenEvent& penEvent, XformedData& xformed, bool withtilt);
}

#endif /* PEN_DATA_TRANSFORM_H_ */
