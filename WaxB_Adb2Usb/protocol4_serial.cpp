/*
 * protocol4_serial.cpp
 *
 * PROTOCOL4 protocol over serial port
 *
 *  Created on: 2011-06-05
 *      Author: Bernard
 */

#include "extdata.h"
#include "avr_util.h"
#include "protocol4_serial.h"
#include "serial.h"
#include "console.h"
#include "pen_events.h"
#include "wacom_usb.h"
#include <util/delay.h>

#include <stdio.h>

#define WACOM_PKGLEN_PROTOCOL4 	 	7
#define WACOM_PKGLEN_PROTOCOL4_TILT	9

#define TABLET_TYPE_UNSUPPORTED	0	// unknown/undefined protocol 4 serial tablet
#define TABLET_TYPE_ULTRAPAD	1  	// UD-*
					// Note: other tablets with Protocol 4 includes:  PL-*,ET-*,CT-*,KT-*

#define ROM_VERSION_UNSUPPORTED	0	// unknown/undefined/V1.1 or earlier
#define ROM_VERSION_1_2		2  	// seen a UD-1218-R version 1.2
#define ROM_VERSION_1_3		3  	//
#define ROM_VERSION_1_4		4  	//
#define ROM_VERSION_1_5		5 	// tested with V1.5-4

namespace protocol4_serial
{
	enum protocol4ControllerState
	{
		initial,	//
		modelversion,	// "~#UD-1212-R V1.5-4\r"
		packet
	};

	uint8_t datalen = 0;
	uint8_t buffer[21];

	uint8_t packet_len;

	Pen::PenEvent penEvent;

	protocol4ControllerState itsCurState = packet;

	uint8_t tabletType;
	uint8_t romVersion;

	bool in_proximity;
	bool eraser_mode;

	void resetToolState()
	{
		in_proximity = false;
		eraser_mode = false;
	}

	void initialState()
	{
		itsCurState = modelversion;
		serial::sendString("\r\r\r\r~#"); // send a few \r to make sure we are in sync
		datalen = 0;
	}

	bool parse_modelversion()
	{
		if(datalen != 20)
		{
			// retry !
			initialState();
			return false;
		}

		// For now, only supports this "rigid" format:
		//
		// "~#UD-1212-R00 V1.5-4" (ends with /r)


		if(buffer[2] == 'U')
		{
			tabletType = TABLET_TYPE_ULTRAPAD;
		}
		else
		{
			tabletType = TABLET_TYPE_UNSUPPORTED;
		}

		if(buffer[15] != '1')
		{
			tabletType = TABLET_TYPE_UNSUPPORTED;
		}

		if(buffer[17] == '2')
			romVersion = ROM_VERSION_1_2;
		else if(buffer[17] == '3')
			romVersion = ROM_VERSION_1_3;
		else if(buffer[17] == '4')
			romVersion = ROM_VERSION_1_4;
		else if(buffer[17] == '5')
			romVersion = ROM_VERSION_1_5;
		else
			romVersion = TABLET_TYPE_UNSUPPORTED;

		if(console::console_enabled)
		{
			console::print("Serial Tablet - ");
			for(uint8_t i=2;i<datalen;i++)
				console::print(buffer[i]);
			console::println();

			switch(tabletType)
			{
				case TABLET_TYPE_UNSUPPORTED:
					console::print("Unsupported");
					break;
				case TABLET_TYPE_ULTRAPAD:
					console::print("UltraPad");
					break;
			}

			console::print(" - ROM Version ");

			switch(romVersion)
			{
				case ROM_VERSION_UNSUPPORTED:
					console::print("Unsupported");
				case ROM_VERSION_1_3:
					console::print("1.3");
					break;
				case ROM_VERSION_1_4:
					console::print("1.4");
					break;
				case ROM_VERSION_1_5:
					console::print("1.5");
					break;
			}

			console::println();
		}

		// event consumed
		datalen = 0;

		if(romVersion == ROM_VERSION_UNSUPPORTED || tabletType == TABLET_TYPE_UNSUPPORTED)
			return false;

		return true;
	}

	/**
	 * set port speed to 19200, set max resolution, set Pnp Off. They t
	 */
	void enableFeatures()
	{
		/*** Explanation of the bits
		 *
		 * F    2    0    3    C    8    1    0
		 * 1111 0010 0000 0011 1100 1000 0001 0000
		 *
		 * 11 WACOM-IV
		 * 11 19200 bauds  (10 - 9600 bauds)
		 * 00 None
		 * 1  8 bits
		 * 0  1 stop bit
		 *
		 * 00 no CTS/DTR check (thus may not need to ground those pins, but it is "better" not to let input pins floating anyways.)
		 * 00 suppressed
		 * 0  binary output format
		 * 0  absolute mode
		 * 11 maximum  (10 = 100pps)
		 *
		 * 11 max resolution (1270,1270)
		 * 0  upper left origin
		 * 0  no out of range data
		 * 10 CRLF line terminator
		 * 0
		 * 0  PnP Off (1 by default)
		 *
		 * 0  firm pressure sensitivity
		 * 0  high reading height (8mm+)
		 * 0  multi-device mode disabled
		 * 1  tilt ON
		 * 0  no MM command set
		 * 0  Portrait  MM961 orientation
		 * 0  1234 - BitPad II cursor data
		 * 0  remote mode OFF
		 *
		 * 002 - increment value (IN)
		 * 00  - max pen reporting speed
		 * 1270,1270 - dpi settings
		 */
		serial::sendString("~*F203C810,002,00,2540,2540\r");

		if(romVersion >= ROM_VERSION_1_4)
			packet_len = WACOM_PKGLEN_PROTOCOL4_TILT;
		else
			packet_len = WACOM_PKGLEN_PROTOCOL4;
	}

	void onSerialByteReceived(uint8_t data)
	{
//		console::print(' ');
//		console::printHex(data, 2);
//		console::print((char)data);

		switch(itsCurState)
		{
			case initial:
			case modelversion:
				if(datalen == 0 && data != '~')
					break; // wait for first valid character

				if(datalen == 1 && data != '#')
				{
					// restart the wait
					datalen = 0;
					break;
				}

				if(data != '\r')
				{
					if(datalen < 20)
					{
						buffer[datalen] = data;
						datalen++;
					}
					else
					{
						// retry !
						initialState();
						break;
					}
				}
				else
				{
					if(parse_modelversion())
					{
						// change port speed, tilt and other thingy
						enableFeatures();

						// 10 ms delay
						_delay_ms(10);

						serial::setNormalPortSpeed(); // set serial port speed to 19200

						// test output max coords.
//						serial::sendString("\r\r~C\r");

						serial::sendString("\r\rST\r");  // Start transmitting data

						itsCurState = packet;
					}
				}
				break;
			case packet:
			{
				if(data & 0x80)
				{
					console::println();
					console::print("*");

					if(datalen > 0)
						console::print("(?!)");
					datalen = 0;
				}
				else if(datalen == 0)
				{
					console::println("(*!)");
					return;		// wait for the first valid byte
				}

				console::printHex(data, 2);

				if(datalen < packet_len)
				{
					buffer[datalen] = data;
					datalen++;
				}

				if(datalen == packet_len)
				{
					// event consumed
					datalen = 0;

					console::println();

					penEvent.proximity = (buffer[0] & 0x40)?1:0;

					penEvent.x =                 buffer[2] |
							(((uint16_t) buffer[1]        ) << 7) |
							(((uint16_t)(buffer[0] & 0x3) ) << 14);

					penEvent.y =    	     buffer[5] |
							(((uint16_t) buffer[4]        ) << 7) |
							(((uint16_t)(buffer[3] & 0x3) ) << 14);

//					console::print("...x=");
//					console::printNumber(penEvent.x);
//					console::print("y=");
//					console::printNumber(penEvent.y);


					/* pressure */

					penEvent.pressure = 	(((uint16_t)(buffer[3] & 0x04)) >> 2) |
								(((uint16_t)(buffer[6] & 0x3F)) << 1);

					penEvent.pressure |= (buffer[6] & 0x40) ? 0 : 0x80;

					penEvent.is_mouse = buffer[0] & 0x20 ? 0 : 1;  // == !stylus

					/* buttons */
					uint8_t buttons = (buffer[3] >> 3) & 0x0F;

					console::print(" - buttons: 0x");
					console::printHex(buttons,1);

					/* check which device (pen/eraser) is being reported */
					bool curEvent_eraser = (buffer[3] & 0x20);

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

							console::print(" - Enter prox =");
							if(eraser_mode)
								console::println("Eraser");
							else
								console::println("Pen");
						}
					}
					else if(!penEvent.proximity)
					{
						console::print(" - Exit prox");
						resetToolState();
					}

					penEvent.eraser = eraser_mode;

					if(eraser_mode)
					{
						// no sideswitches with eraser
						penEvent.button0 = 0;
						penEvent.button1 = 0;
					}
					else
					{
						penEvent.button0 = (buffer[3] & 0x10) ? 1:0;  // sideswitch 1
						penEvent.button1 = (buffer[3] & 0x20) ? 1:0;  // sideswitch 2
					}

					// tip
					penEvent.touch = (buffer[3] & 0x08) ? 1:0; // shouldn't we look at the pressure instead?

					// TODO: understand the extra logic from the linux driver (wacserial.c)

					/* check on previous proximity */
					/* we might have been fooled by tip and second
					 * sideswitch when it came into prox */

					/* tilt mode */
					if (packet_len == WACOM_PKGLEN_PROTOCOL4_TILT)
					{
						penEvent.tilt_x = buffer[7] & 0x3F;
						penEvent.tilt_y = buffer[8] & 0x3F;
						if (buffer[7] & 0x40) penEvent.tilt_x -= 64;
						if (buffer[8] & 0x40) penEvent.tilt_y -= 64;
					}

					penEvent.rotation_z = 0;

					console::println();

					Pen::send_pen_event(penEvent);
				}
				break;
			}
		}
	}

	void init()
	{
		console::print("protocol4_serial::init()\n");

		resetToolState();

		serial::init(onSerialByteReceived);
		serial::setInitialPortSpeed();

		initialState();
	}
}
