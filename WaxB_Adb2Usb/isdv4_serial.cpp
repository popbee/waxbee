/*
 * isdv4_serial.cpp
 *
 * ISDV4 protocol over serial port
 *
 *  Created on: 2011-03-20
 *      Author: Bernard
 */

#include "extdata.h"
#include "avr_util.h"
#include "isdv4_serial.h"
#include "serial.h"
#include "console.h"
#include "pen_events.h"
#include "wacom_usb.h"

#define WACOM_PKGLEN_TPC 	 9
#define WACOM_PKGLEN_TPCCTL     11

namespace isdv4_serial
{
	enum isdv4ControllerState
	{
		unknown,
		stop,  		// "0" // wait ~250ms
		query,		// "*"
		query_packet,
		start, 	   	// "1"
		packet
	};

	uint8_t datalen = 0;
	uint8_t buffer[11];

	Pen::PenEvent penEvent;

	isdv4ControllerState itsCurIsdv4State = packet;


	bool in_proximity;
	bool eraser_mode;

	void resetToolState()
	{
		in_proximity = false;
		eraser_mode = false;
	}

	void onSerialByteReceived(uint8_t data)
	{
		console::print(' ');
		console::printHex(data, 2);

		switch(itsCurIsdv4State)
		{
			case unknown:
			case stop:
			case query:
			case query_packet:
			case start:
			case packet:
			{
				if(data & 0x80)
				{
					console::println();

/*					if(datalen > 0)
						console::println("(?!)");
*/					datalen = 0;
				}
				else if(datalen == 0)
				{
//					console::println("(*!)");
					return;		// wait for the first valid byte
				}

				if(datalen < WACOM_PKGLEN_TPC)
				{
					buffer[datalen] = data;
					datalen++;
				}

				if(datalen == WACOM_PKGLEN_TPC)
				{
					// event consumed
					datalen = 0;

					console::println();

					penEvent.proximity = (buffer[0] & 0x20)?1:0;

					penEvent.x =    (((uint16_t)(buffer[6] & 0x60)) >> 5) |
							(((uint16_t) buffer[2]        ) << 2) |
							(((uint16_t) buffer[1]        ) << 9);

					penEvent.y =	(((uint16_t)(buffer[6] & 0x18)) >> 3) |
							(((uint16_t) buffer[4]        ) << 2) |
							(((uint16_t) buffer[3]        ) << 9);

					/* pressure */
					penEvent.pressure = (((uint16_t)(buffer[6] & 0x07)) << 7) |
									 buffer[5];

					/* buttons */
					uint8_t buttons = (buffer[0] & 0x07);

//					console::print(" - buttons: 0x");
//					console::printHex(buttons,1);

					/* check which device has been reported */
					bool curEvent_eraser = (buffer[0] & 4)? 1:0;

					// detect tool state
					if(!in_proximity)
					{
						if(penEvent.proximity)
						{
							// entering proximity
							in_proximity = true;

							if(curEvent_eraser)
							{
								eraser_mode = true;
							}
							else
							{
								eraser_mode = false;
							}

/*							console::print(" ->Entering proximity. Mode=");
							if(eraser_mode)
								console::println("Eraser");
							else
								console::println("Pen");
*/						}
					}
					else if(!penEvent.proximity)
					{
/*						console::println(" ->Exiting proximity.");
*/						resetToolState();
					}

					penEvent.eraser = eraser_mode;

					//TODO :  how to map the following???
					penEvent.touch = 0;
					penEvent.button0 = 0;
					penEvent.button1 = 0;
					penEvent.is_mouse = 0;

					// TODO: understand the extra logic from the linux driver (wcmISDV4.c)

					/* check on previous proximity */
					/* we might have been fooled by tip and second
					 * sideswitch when it came into prox */

					/* don't send button 3 event for eraser
					 * button 1 event will be sent by testing pressure level
					 */

					// tilt data?
					penEvent.tilt_x = 0;
					penEvent.tilt_y = 0;

					penEvent.rotation_z = 0;

					Pen::send_pen_event(penEvent);
				}
				break;
			}
		}
	}

	void init()
	{
		console::print("isdv4_serial::init()\n");

		resetToolState();

		serial::init(onSerialByteReceived);
		serial::setNormalPortSpeed();
	}
}
