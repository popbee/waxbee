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
#include <util/delay.h>

#define WACOM_PKGLEN_TPC 	 9
#define WACOM_PKGLEN_TPCCTL     11

namespace isdv4_serial
{
	enum isdv4ControllerState
	{
		halted,
		  		// "0" // wait ~250ms
				// "*"
		query_packet,
		 	   	// "1"
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

	void start_packets()
	{
		datalen = 0;
		serial::sendString("\r\r1\r"); // send a few \r to make sure we are in sync
		itsCurIsdv4State = packet;
	}

	void onSerialByteReceived(uint8_t data)
	{
		if(data & 0x80)
		{
			console::println();
		}

		console::print(' ');
		console::printHex(data, 2);

		switch(itsCurIsdv4State)
		{
			case halted:
				return;

			case query_packet:
				if(datalen < WACOM_PKGLEN_TPCCTL)
				{
					buffer[datalen] = data;
					datalen++;
				}

				if(datalen == WACOM_PKGLEN_TPCCTL)
				{
					uint16_t maxZ = ( buffer[5] | ((buffer[6] & 0x07) << 7) );
					uint16_t maxX = ( (buffer[1] << 9) |
						(buffer[2] << 2) | ( (buffer[6] & 0x60) >> 5) );
					uint16_t maxY = ( (buffer[3] << 9) | (buffer[4] << 2 )
						| ( (buffer[6] & 0x18) >> 3) );

					uint16_t maxtiltX = 0;
					uint16_t maxtiltY = 0;
					bool hasTilt = false;


					if (buffer[7] && buffer[8])
					{
						maxtiltX = buffer[7] + 1;
						maxtiltY = buffer[8] + 1;
						hasTilt = true;
					}

					uint16_t version = ( buffer[10] | (buffer[9] << 7) );

					console::print("\nPressure Max=");
					console::printNumber(maxZ);
					console::print(", X Max=");
					console::printNumber(maxX);
					console::print(", Y Max=");
					console::printNumber(maxY);

					if(hasTilt)
					{
						console::print(", Has Tilt: Tilt X Max=");
						console::printNumber(maxtiltX);
						console::print(", Tilt Y Max=");
						console::printNumber(maxtiltY);
					}
					else
						console::print(", No Tilt");

					console::print(", Version=");
					console::printNumber(version);
					console::println("");

					start_packets();
				}

				break;
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

				if(datalen < WACOM_PKGLEN_TPC)
				{
					buffer[datalen] = data;
					datalen++;
				}

				if(datalen == WACOM_PKGLEN_TPC)
				{
					// event consumed
					datalen = 0;

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

					// not sure if this bit is looked at anyways
					penEvent.touch = penEvent.pressure > 10;

					penEvent.button0 = buffer[0] & 0x02;
					penEvent.button1 = buffer[0] & 0x04;

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

					if(eraser_mode)
					{
						// no buttons with eraser
						penEvent.button0 = 0;
						penEvent.button1 = 0;
					}

					penEvent.is_mouse = 0;

					// TODO: understand the extra logic from the linux driver (wcmISDV4.c)

					/* check on previous proximity */
					/* we might have been fooled by tip and second
					 * sideswitch when it came into prox */

					// no tilt data?
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

		itsCurIsdv4State = halted;

		if(console::console_enabled)
		{
			serial::sendString("\r\r0\r"); // send a few \r to make sure we are in sync
			_delay_ms(250);
			serial::flush();
			datalen = 0;
			serial::sendString("*");
			itsCurIsdv4State = query_packet;
			console::println("Sending *...");
		}
		else
		{
			start_packets();
		}
	}
}
