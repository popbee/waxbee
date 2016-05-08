/*
 * topaz_serial.cpp
 *
 * Topaz tablet protocol
 *
 *  Created on: 2011-07-25
 *      Author: Bernard
 */
#include "featureinclusion.h"

#include "extdata.h"
#include "avr_util.h"
#include "topaz_serial.h"
#include "serial.h"
#include "console.h"
#include "pen_event.h"
#include "wacom_usb.h"
#include "strings.h"

#include <util/delay.h>

#define TOPAZ_PACKET	5

#ifdef SERIAL_TOPAZ_SUPPORT
namespace topaz_serial
{
	enum topazControllerState
	{
		// simple state
		packet
	};

	uint8_t datalen = 0;
	uint8_t buffer[5];

	Pen::PenEvent penEvent;

	topazControllerState itsCurState;

	void onSerialByteReceived(uint8_t data)
	{
		if(data & 0x80)
		{
			console::println();
		}

		console::print(' ');
		console::printHex(data, 2);

		switch(itsCurState)
		{
			case packet:
			{
				if(data & 0x80)
				{
//					console::println();

/*					if(datalen > 0)
						console::println("(?!)");
*/					datalen = 0;
				}
				else if(datalen == 0)
				{
//					console::println("(*!)");
					return;		// wait for the first valid byte
				}

				if(datalen < TOPAZ_PACKET)
				{
					buffer[datalen] = data;
					datalen++;
				}

				if(datalen == TOPAZ_PACKET)
				{
					// event consumed
					datalen = 0;

					// packet sanity check
					if(buffer[2] & 0xE0 || buffer[4] & 0xE0)
						return;

					penEvent.proximity = (buffer[0] & 0x40)?1:0;

					penEvent.x =    (((uint16_t)(buffer[2] & 0x1f)) << 7) |
							(((uint16_t) buffer[1] & 0x7f));

					penEvent.y =    (((uint16_t)(buffer[4] & 0x1f)) << 7) |
							(((uint16_t) buffer[3] & 0x7f));

					/* no "pressure" levels, just 0 or 1 */
					penEvent.pressure = (buffer[0] & 0x01)?1:0;

					// not sure if this bit is looked at anyways
					penEvent.touch = (buffer[0] & 0x01)?1:0;;

					// no buttons, no eraser, no tilt, no rotation data, no mouse
					penEvent.eraser = 0;
					penEvent.button0 = 0;
					penEvent.button1 = 0;
					penEvent.is_mouse = 0;
					penEvent.tilt_x = 0;
					penEvent.tilt_y = 0;
					penEvent.rotation_z = 0;

					Pen::input_pen_event(penEvent);
				}
				break;
			}
		}
	}

	void init()
	{
		console::printlnP(STR_TOPAZ_SERIAL_INIT);

		serial::init(onSerialByteReceived);
		serial::setNormalPortSpeed();

		itsCurState = packet;
	}
}
#endif
