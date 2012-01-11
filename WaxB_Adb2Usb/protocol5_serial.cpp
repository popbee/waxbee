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
		// attempt to reset the board back to 9600 bauds. If the board is already at 9600 baud the following
		// will be ignored by the board. This case happens when you restart the teensy software without
		// rebooting the tablet (occurs after a firmware update).

		serial::setNormalPortSpeed(); // try communicating at the configured normal baud rate (19200 or 38400)
		serial::sendString("\r\r\r\rBA96\r"); // send a few extra \r to make sure we are in sync

		_delay_ms(30);

		// if not already, we should now be at 9600 baud at this point

		serial::setInitialPortSpeed(); // switch internal serial port to 9600 baud

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
	 * IT0: set max reporting speed.
	 * MT0: disable multimode (in case this was enabled)
	 * ID1: dunno what it does.
	 */
	void enableFeatures()
	{
		serial::sendString("MT0\rIT0\rID1\r");
	}

	void onSerialByteReceived(uint8_t data)
	{
		if(ignoreIncomingBytes)
			return;

//		console::print(' ');
//		console::printHex(data, 2);
		//console::print("Rcvd serial data: ");
		//console::print((char)data);

		switch(itsCurState)
		{
			case initial:
			case modelversion:
				console::print((char)data);

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
						enableFeatures();

						// Tell the tablet to run at the normal configured speed

						uint8_t baud = extdata_getValue8(EXTDATA_SERIAL_PORT_SPEED);

						if(baud == EXTDATA_SERIAL_PORT_SPEED_BAUD_38400)
							serial::sendString("BA38\r");
						else if(baud == EXTDATA_SERIAL_PORT_SPEED_BAUD_19200)
							serial::sendString("BA19\r");
						else if(baud == EXTDATA_SERIAL_PORT_SPEED_BAUD_9600)
							serial::sendString("BA96\r");

						_delay_ms(20);

						serial::setNormalPortSpeed(); // set our serial port speed as well

						// test output max coords.
//						serial::sendString("\r\r~C\r");

						serial::sendString("\r\rST\r");  // STart transmitting data

						itsCurState = packet;
					}
				}
				break;
			case packet:
			{
				// On my GD-1218-R, the first packet from the 4D mouse always seems to be truncated after 6 bytes.
				// I've reproduced this many times.  I'm guessing it's a firmware bug maybe caused by the buttons
				// not being initialized or something?
				// The (?!) means it found HEADER_BIT in a byte that should have been part of the first 9-byte packet.
				// AA begins the 4D mouse "second packet".  I've verified in a serial driver that any byte after the
				// first that contains HEADER_BIT makes the packet invalid and it should be ignored, which is what we
				// do.
				// RS: A8 43 48 00 00 00
				// RS: (?!)AA 43 60 2B 74 48 00 00 00
				// 
				// Actually, the same thing happens when a stylus comes into range:
				// RS: C2 41 08 00 10 0C 7F 30 00
				// Serial Tool ID: 0822
				// Tool serial #: 001019FD
				// SU: 02 C2 85 20 01 01 9F D0 00 00
				// RS: A0 27 70 00 00 00
				// RS: (?!)E0 27 1C 6A 59 70 03 07 07
				if(data & HEADER_BIT)
				{
					#ifdef DEBUG_SERIAL_DATA
						console::println();
						console::print("RS: "); // Receiving serial data.  Keep these debug statements short or it seems to overflow the USB buffer
					#endif

					if(datalen > 0) {
						console::print("(?!)"); // UNEXPECTED HEADER_BIT IN DATA PACKET
					}
					datalen = 0;
				}
				else if(datalen == 0)
				{
					console::printHex(data, 2);
					console::println("(*!)"); // FIRST BYTE IN DATA PACKET DID NOT CONTAIN HEADER_BIT
					return;		// wait for the first valid byte
				}

				#ifdef DEBUG_SERIAL_DATA
					console::printHex(data, 2);
					console::print(" ");
				#endif

				if(datalen < WACOM_PKGLEN_PROTOCOL5)
				{
					buffer[datalen] = data;
					datalen++;
				}

				if(datalen == WACOM_PKGLEN_PROTOCOL5)
				{
					// Packet complete.  Act on it.
					penEvent.serial_packet_first_byte = buffer[0];

					datalen = 0;

					#ifdef DEBUG_SERIAL_DATA
						console::println();
					#endif

					if ((buffer[0] & 0xFC) == 0xC0)
					{
						// "in proximity"

						in_proximity = 1;

						uint16_t tool_id =	(((uint16_t)buffer[1] & 0x7f) << 5) |
									(((uint16_t)buffer[2] & 0x7c) >> 2);


						uint32_t tool_serial_num =	(((uint32_t)buffer[2] & 0x03) << 30) |
									(((uint32_t)buffer[3] & 0x7f) << 23) |
									(((uint32_t)buffer[4] & 0x7f) << 16) |
									(((uint32_t)buffer[5] & 0x7f) <<  9) |
									(((uint32_t)buffer[6] & 0x7f) <<  2) |
									(((uint32_t)buffer[7] & 0x60) >>  5);

						console::print("Serial Tool ID: ");
						console::printHex(tool_id, 6);
						console::println();
						console::print("Tool serial #: ");
						console::printHex(tool_serial_num, 8);
						console::println();

						eraser_mode = 0;
						uint32_t usb_tool_id = 0;

						// This bit of code comes from the linux serial driver and it basically makes
						// us discard the first packet received for the 4D mouse.  Technically it sets
						// discard_first = 1 for all tools that aren't in the 0x800 range of pens, but
						// discard_first is only used when looking at the first 4D mouse packet.
						// I assume this has to do with the invalid first packet we get with
						// the 4D mouse because discard_first is only checked when interpreting that 
						// packet and only when device type is 4D mouse and discard_first is only cleared 
						// when the 4D mouse "second packet" is received.
						if ((tool_id & 0xf06) != 0x802) {
							penEvent.discard_first = 1;
						}

						switch (tool_id)
						{
							case 0x007: // 2D Mouse (Intuos1 or 2)
							case 0x09C: // ?? Mouse (maybe "3D mouse", a 4D mouse without scroll wheel?)
							case 0x094: // 4D Mouse (Intuos1 or 2)
							case 0x096: // Lens cursor (Intuos1 or 2)
								penEvent.is_mouse = 1;

								// Oddly, 0x094 seems to represent both Intuos1 and Intuos2 4D mice.  I also tried 0x09C and
								// the Wacom control panel shows the same picture of an Intuos2 4D mouse, says it's a 4D mouse
								// in the description, and rotation of the mouse is detected, but the scroll wheel is not.
								// Since there is no Intuos2 lens cursor listed in the Linux driver, they must use 0x096
								// for both Intuos1 and Intuos2.
								// The same thing seems to hold for 2D mice.
								usb_tool_id = tool_id;
								break;

							case 0x812: // Intuos2 ink pen XP-110-00A
							case 0x012: // Inking pen
							case 0x822: // Intuos1 Pen GP-300E-01H
							case 0x852: // Intuos2 Grip Pen XP-501E-00A
							case 0x842: // Designer pen
							case 0x022: // Intuos1 pen
							case 0x832: // Intuos2 stroke pen XP-120-00A
							case 0x032: // Stroke pen

							default: // Unknown tool
								penEvent.is_mouse = 0;
								usb_tool_id = 0x852;
								break;

							case 0x112: // Airbrush (generic?)
							case 0xd12: // Intuos1 airbrush
							case 0x912: // Intuos2 airbrush							
								penEvent.is_mouse = 0;
								usb_tool_id = 0x912;
								break;

							case 0x82a: // Intuos1 eraser
							case 0x91a: // Intuos1 eraser
							case 0xd1a: // Intuos1 eraser
							case 0x0fa: // Intuos1 eraser
							case 0x85a: // Intuos2 eraser
								eraser_mode = 1;
								usb_tool_id = 0x85A;
								break;
						}

						penEvent.proximity = 1;
						penEvent.eraser = eraser_mode;
						penEvent.tool_id = usb_tool_id;
						penEvent.tool_serial_num = tool_serial_num;
						Pen::input_pen_event(penEvent);
					}
					else if ((buffer[0] & 0xFE) == 0x80)
					{
						// out of proximity
						resetToolState();

						penEvent.proximity = 0;
						Pen::input_pen_event(penEvent);
					}
					else {
						// Pen and mouse packets all contain the X/Y data in the same bits:
						penEvent.x =	(((uint16_t)buffer[1] & 0x7f) <<  9) |
								(((uint16_t)buffer[2] & 0x7f) <<  2) |
								(((uint16_t)buffer[3] & 0x60) >>  5);

						penEvent.y =	(((uint16_t)buffer[3] & 0x1f) << 11) |
								(((uint16_t)buffer[4] & 0x7f) <<  4) |
								(((uint16_t)buffer[5] & 0x78) >>  3);

						if (((buffer[0] & 0xB8) == 0xA0) || ((buffer[0] & 0xBE) == 0xB4))
						{
							// pen packet

							penEvent.eraser = eraser_mode;

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
							else // 2nd airbrush packet
							{
								penEvent.abswheel = ((((int)buffer[5] & 0x07) << 7) |
										((int)buffer[6] & 0x7f));
							}
	
							penEvent.proximity = ((uint16_t)buffer[0] & PROXIMITY_BIT);
							penEvent.touch = (buffer[3] & 0x08) ? 1:0; // shouldn't we look at the pressure instead?

							//if(penEvent.proximity) {
								Pen::input_pen_event(penEvent);
							//}
						}

						/* 4D mouse 1st packet or Lens cursor packet or 2D mouse packet*/
						else if (((buffer[0] & 0xbe) == 0xa8) || ((buffer[0] & 0xbe) == 0xb0)) {
							//penEvent.proximity = 1;
							int have_data = 0;
							penEvent.is_mouse = 1;
							
							penEvent.tilt_x = 0;
							penEvent.tilt_y = 0;

							if(MOUSE_4D(penEvent)) {     /* 4D mouse */
								penEvent.throttle = ((((uint16_t)buffer[5] & 0x07) << 7) | ((uint16_t)buffer[6] & 0x7f));
								if ((uint16_t)buffer[8] & 0x08) penEvent.throttle = -penEvent.throttle;
								penEvent.buttons = ((((uint16_t)buffer[8] & 0x70) >> 1) | ((uint16_t)buffer[8] & 0x07));
								have_data = !penEvent.discard_first;
							} else if (LENS_CURSOR(penEvent)) {   /* Lens cursor */
								penEvent.buttons = (uint16_t)buffer[8];
								have_data = 1;
							} else if (MOUSE_2D(penEvent)) {      /* 2D mouse */
								penEvent.buttons = ((uint16_t)buffer[8] & 0x1C) >> 2;
								penEvent.relwheel = -((uint16_t)buffer[8] & 1) + (((uint16_t)buffer[8] & 2) >> 1);
								have_data = 1; /* must send since relwheel is reset */
							}

							// On 4D mouse, button bits are:
							// 0: left, 1: middle, 2: right, 3: lower right, 4: lower left.
							//console::print("Buttons: ");
							//console::printHex(penEvent.buttons, 8);
							//console::println();


							penEvent.proximity = ((uint16_t)buffer[0] & PROXIMITY_BIT);
							if(have_data) { // && penEvent.proximity) {
								Pen::input_pen_event(penEvent);
							}
						}
						/* 4D mouse 2nd packet */
						else if (((uint16_t)buffer[0] & 0xbe) == 0xaa) {
							int have_data = 0;
							penEvent.is_mouse = 1;

							penEvent.rotation_z = ((((uint16_t)buffer[6] & 0x0f) << 7) | ((uint16_t)buffer[7] & 0x7f));
							//if (penEvent.rotation_z < 900) penEvent.rotation_z = -penEvent.rotation_z;
							//else penEvent.rotation_z = 1799 - penEvent.rotation_z;
							penEvent.proximity = ((uint16_t)buffer[0] & PROXIMITY_BIT);
							have_data = 1;
							penEvent.discard_first = 0;
							if(have_data) { // && penEvent.proximity) {
								Pen::input_pen_event(penEvent);
							}
						}
					}

					// TODO: understand the extra logic from the linux driver (wacserial.c)

					/* check on previous proximity */
					/* we might have been fooled by tip and second
					 * sideswitch when it came into prox */

					//console::println();
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
