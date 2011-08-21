/*
 * protocol5_serial.cpp
 *
 * PROTOCOL5 protocol over serial port
 *
 *  Created on: 2011-06-05
 *      Author: Bernard
 */

#include "extdata.h"
#include "avr_util.h"
#include "protocol5_serial.h"
#include "serial.h"
#include "console.h"
#include "strings.h"
#include "pen_event.h"
#include "wacom_usb.h"
#include <util/delay.h>

#include <stdio.h>

#define WACOM_PKGLEN_PROTOCOL5 	 	9


#define TABLET_TYPE_UNSUPPORTED	0	// unknown/undefined protocol 5 serial tablet
#define TABLET_TYPE_INTUOS	1  	// GD-*
#define TABLET_TYPE_INTUOS2	2  	// XD-*

namespace protocol5_serial
{
	enum protocol5ControllerState
	{
		initial,	//
		modelversion,	// "~#GD-0912-R,V2.0\r"
		packet
	};

	uint8_t datalen = 0;
	uint8_t buffer[21];

	Pen::PenEvent penEvent;

	protocol5ControllerState itsCurState = packet;

	uint8_t tabletType;

	bool in_proximity;
	bool eraser_mode;
	bool ignoreIncomingBytes;

	void resetToolState()
	{
		in_proximity = false;
		eraser_mode = false;
	}

	void initialState()
	{
		// attempt to reset the board back to 9600 bauds

		serial::setNormalPortSpeed(); // try communicating at 38400 baud
		serial::sendString("\r\r\r\rBA96\r"); // send a few \r to make sure we are in sync

		_delay_ms(30);

		// if not already, we should now be at 9600 baud at this point

		serial::setInitialPortSpeed(); // switch to 9600 baud
		serial::sendString("\r\r\r\r#\r"); // do a factory reset

		_delay_ms(30);

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
		// "~#GD-0912-R,V2.0" (ends with /r)


		if(buffer[2] == 'G')
		{
			tabletType = TABLET_TYPE_INTUOS;
		}
		else if(buffer[2] == 'X')
		{
			tabletType = TABLET_TYPE_INTUOS2;
		}
		else
		{
			tabletType = TABLET_TYPE_UNSUPPORTED;
		}

		if(console::console_enabled)
		{
			console::printP(STR_SERIAL_TABLET);
			for(uint8_t i=2;i<datalen;i++)
				console::print(buffer[i]);
			console::println();

			switch(tabletType)
			{
				case TABLET_TYPE_UNSUPPORTED:
					console::printP(STR_UNSUPPORTED);
					break;
				case TABLET_TYPE_INTUOS:
					console::printP(STR_INTUOS);
					break;
				case TABLET_TYPE_INTUOS2:
					console::printP(STR_INTUOS2);
					break;
			}

			console::println();
		}

		// event consumed
		datalen = 0;

		if(tabletType == TABLET_TYPE_UNSUPPORTED)
			return false;

		return true;
	}

	/**
	 * BA38: set port speed to 38400,
	 * IT0: set max reporting speed.
	 * MT0: disable multimode (in case this was enabled)
	 * ID1: dunno what it does.
	 */
	void enableFeatures()
	{
		serial::sendString("MT0\rIT0\rID1\rBA38\r");
	}

	void onSerialByteReceived(uint8_t data)
	{
		if(ignoreIncomingBytes)
			return;

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
						// change port speed and other thingy
						enableFeatures();

						_delay_ms(20);

						serial::setNormalPortSpeed(); // set serial port speed to 38400

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
//					console::println();
//					console::print("*");

					if(datalen > 0)
						console::print("(?!)");
					datalen = 0;
				}
				else if(datalen == 0)
				{
					console::println("(*!)");
					return;		// wait for the first valid byte
				}

//				console::printHex(data, 2);

				if(datalen < WACOM_PKGLEN_PROTOCOL5)
				{
					buffer[datalen] = data;
					datalen++;
				}

				if(datalen == WACOM_PKGLEN_PROTOCOL5)
				{
					// event consumed
					datalen = 0;

//					console::println();

					if ((buffer[0] & 0xFC) == 0xC0)
					{
						// "in proximity"

						in_proximity = 1;

						uint16_t toolid =	(((uint16_t)buffer[1] & 0x7f) << 5) |
									(((uint16_t)buffer[2] & 0x7c) >> 2);

/*
						uint32_t serial =	(((uint32_t)buffer[2] & 0x03) << 30) |
									(((uint32_t)buffer[3] & 0x7f) << 23) |
									(((uint32_t)buffer[4] & 0x7f) << 16) |
									(((uint32_t)buffer[5] & 0x7f) <<  9) |
									(((uint32_t)buffer[6] & 0x7f) <<  2) |
									(((uint32_t)buffer[7] & 0x60) >>  5);
*/
//						console::printHex(toolid, 8);

						eraser_mode = 0;

						switch (toolid)
						{
							case 0x007: // 2D Mouse
							case 0x09C: // ?? Mouse
							case 0x094: // 4D Mouse
							case 0x096: // Lens cursor
								penEvent.is_mouse = 1;
								break;

							case 0x812: // Intuos2 ink pen XP-110-00A
							case 0x012: // Inking pen
							case 0x822: // Intuos Pen GP-300E-01H
							case 0x852: // Intuos2 Grip Pen XP-501E-00A
							case 0x842: // added from Cheng
							case 0x022:
							case 0x832: // Intuos2 stroke pen XP-120-00A
							case 0x032: // Stroke pen

							case 0x112: // Airbrush
							default: // Unknown tool
								penEvent.is_mouse = 0;
								break;

							case 0x82a:
							case 0x85a:
							case 0x91a:
							case 0x0fa: // Eraser
								eraser_mode = 1;
								break;
						}

						// do nothing. (at least for now)

						// TODO: adapt code to better reflect
						// intuos2 Serial -> intuos2 USB (pass the serial number/tool ids, etc.)
					}
					else if ((buffer[0] & 0xFE) == 0x80)
					{
						// out of proximity
						resetToolState();

						penEvent.proximity = 0;
						Pen::input_pen_event(penEvent);
					}
					else if (((buffer[0] & 0xB8) == 0xA0) || ((buffer[0] & 0xBE) == 0xB4))
					{
						// normal packet

						penEvent.proximity = 1;
						penEvent.eraser = eraser_mode;

						penEvent.x =	(((uint16_t)buffer[1] & 0x7f) <<  9) |
								(((uint16_t)buffer[2] & 0x7f) <<  2) |
								(((uint16_t)buffer[3] & 0x60) >>  5);

						penEvent.y =	(((uint16_t)buffer[3] & 0x1f) << 11) |
								(((uint16_t)buffer[4] & 0x7f) <<  4) |
								(((uint16_t)buffer[5] & 0x78) >>  3);

						// rotation   min  = -900, max = 899
						penEvent.rotation_z =  (((int16_t)buffer[6] & 0x0f) << 7) |
									((int16_t)buffer[7] & 0x7f);

						if (penEvent.rotation_z < 900)
							penEvent.rotation_z = -penEvent.rotation_z;
						else
							penEvent.rotation_z = 1799 - penEvent.rotation_z;

						// tilt

						penEvent.tilt_x = (buffer[7] & 0x3F);
						if (buffer[7] & 0x40)
							penEvent.tilt_x -= 0x40;

						penEvent.tilt_y = (buffer[8] & 0x3F);
						if (buffer[8] & 0x40)
							penEvent.tilt_y -= 0x40;

						// pen packet
						if ((buffer[0] & 0xB8) == 0xA0)
						{
							penEvent.pressure = 	(((uint16_t)buffer[5] & 0x07) << 7) |
										 ((uint16_t)buffer[6] & 0x7f);

							penEvent.touch = (penEvent.pressure > 10) ? 1 : 0;
							penEvent.button0 = (buffer[0] & 0x02) ? 1 : 0;
							penEvent.button1 = (buffer[0] & 0x04) ? 1 : 0;
						}
/*						else // 2nd airbrush packet
						{
							wheel = ((((int)buffer[5] & 0x07) << 7) |
									((int)buffer[6] & 0x7f));
						}
*/
						penEvent.touch = (buffer[3] & 0x08) ? 1:0; // shouldn't we look at the pressure instead?

						Pen::input_pen_event(penEvent);
					}

					// TODO: understand the extra logic from the linux driver (wacserial.c)

					/* check on previous proximity */
					/* we might have been fooled by tip and second
					 * sideswitch when it came into prox */

//					console::println();
				}
				break;
			}
		}
	}

	void init()
	{
		console::printlnP(STR_PROTOCOL5_SERIAL_INIT);

		resetToolState();

		ignoreIncomingBytes = true;
		serial::init(onSerialByteReceived);
		serial::setInitialPortSpeed();
		ignoreIncomingBytes = false;

		initialState();
	}
}
