/*
 * Wacom-related stuff for the USB-side interface
 *
 */
#include "featureinclusion.h"

#include "usb_util.h"
#include "extdata.h"
#include "console.h"
#include "wacom_usb.h"
#include "pen_event.h"
#include "pen_data_transform.h"
#include "strings.h"
#include "debugproc.h"
#include "led.h"

#define WACOM_PEN_ENDPOINT	1
#define WACOM_INTUOS5_PEN_ENDPOINT 3

namespace WacomUsb
{
	static uint8_t	usbProtocol;

	void init()
	{
		usbProtocol = extdata_getValue8(EXTDATA_USB_PROTOCOL);
	}

#ifdef USB_PROTOCOL_IV_SUPPORT
	//-----------------------------------------------------------------
	// USB Protocol IV (4)
	//-----------------------------------------------------------------

	struct protocol4_struct
	{
		union
		{
			uint8_t buffer[8];
			struct
			{
				uint8_t hid_identifier; // 0x02
				struct
				{
					unsigned touch:1;   // bit 0 - pen touches the tablet
					unsigned button0:1; // bit 1 - first button on the pen
					unsigned button1:1; // bit 2 - second button on the pen
					unsigned reserved:1;// bit 3 - appears to be always 0
					unsigned inrange:1; // bit 4 - like 'proximity' but for a greater distance.
							    // 		mouse is not moving in Windows when in that range
							    // 		but x/y coordinates are being sent.
					unsigned eraser:1;  // bit 5 - eraser tool
					unsigned is_mouse:1;// bit 6 - 0=pen, 1=mouse

					unsigned proximity:1;//bit 7 - pen in close proximity, moves the mouse
				};
				uint16_t x;
				uint16_t y;
				unsigned pressure:9;
				unsigned zero:7;	// 0
			};
		};
	} protocol4_packet;

	void send_protocol4_packet(Pen::PenEvent& penEvent)
	{
		bool shouldfill = false;
		// copy event for direct access (TODO: does that make a difference?)

		protocol4_packet.hid_identifier = 0x02;

		protocol4_packet.reserved = 0;
		protocol4_packet.is_mouse = penEvent.is_mouse;
		protocol4_packet.proximity = penEvent.proximity;
		protocol4_packet.inrange = penEvent.proximity;

		if(protocol4_packet.inrange)
		{
			PenDataTransform::XformedData xformed;

			PenDataTransform::transform_pen_data(penEvent, xformed,
								false); // no tilt

			protocol4_packet.x = xformed.x;
			protocol4_packet.y = xformed.y;
			protocol4_packet.pressure = xformed.pressure;
			protocol4_packet.touch = penEvent.touch;
			protocol4_packet.eraser = penEvent.eraser;
			protocol4_packet.button0 = penEvent.button0;
			protocol4_packet.button1 = penEvent.button1;

			shouldfill = true;
		}
		else
		{
			protocol4_packet.x = 0;
			protocol4_packet.y = 0;
			protocol4_packet.pressure = 0;
			protocol4_packet.touch = 0;
			protocol4_packet.eraser = 0;
			protocol4_packet.button0 = 0;
			protocol4_packet.button1 = 0;
#ifdef DEBUG_SUPPORT
			DebugProc::proxOutTrigger();
#endif
		}

/*		if(console::console_enabled)
		{
			console::print("[USB Packet - prox:");
			console::printbit(protocol4_packet.proximity);
			console::print(" mou:");
			console::printbit(protocol4_packet.is_mouse);
			console::print(" range:");
			console::printbit(protocol4_packet.inrange);
			console::print(" touc:");
			console::printbit(protocol4_packet.touch);
			console::print(" era:");
			console::printbit(protocol4_packet.eraser);
			console::print(" b0:");
			console::printbit(protocol4_packet.button0);
			console::print(" b1:");
			console::printbit(protocol4_packet.button1);
			console::print(" res:");
			console::printbit(protocol4_packet.reserved);
			console::print(" x=");
			console::printNumber(protocol4_packet.x);
			console::print(", y=");
			console::printNumber(protocol4_packet.y);
			console::print(", pressure=");
			console::printNumber(protocol4_packet.pressure);
			console::println("]");
		}
*/
		if(extdata_getValue8(EXTDATA_USB_PORT) == EXTDATA_USB_PORT_DIGITIZER)
			UsbUtil::send_packet_fill_idle_time(protocol4_packet.buffer, 8, WACOM_PEN_ENDPOINT, 50);
	}
#endif


	//-----------------------------------------------------------------
	// USB Bamboo Pen Packet
	//-----------------------------------------------------------------

	struct bamboo_pen_struct
	{
		union
		{
			uint8_t buffer[9];
			struct
			{
				uint8_t hid_identifier; // 0x02
				struct
				{
					unsigned touch:1;   // bit 0 - pen touches the tablet
					unsigned button0:1; // bit 1 - first button on the pen
					unsigned button1:1; // bit 2 - second button on the pen
					unsigned eraser:1;  // bit 3 - eraser tool
					unsigned inrange:1; // bit 4 - (?)like 'proximity' but for a greater distance.
							    // 		mouse is not moving in Windows when in that range
							    // 		but x/y coordinates are being sent.
					unsigned proximity:1;// bit 5 - proximity(?) pen in close proximity, moves the mouse
					unsigned notsure:1; // bit 6 - ??
					unsigned outofprox:1;// bit 7 - appears to be the only thing set with everything else is zero?
				};
				uint16_t x;
				uint16_t y;
				uint16_t pressure;
				uint8_t distance;
			};
		};
	} bamboo_pen_packet;

	void send_bamboo_pen_packet(Pen::PenEvent& penEvent)
	{
		bool shouldfill = false;
		bamboo_pen_packet.hid_identifier = 0x02;

		bamboo_pen_packet.notsure = 0;

//		bamboo_pen_packet.is_mouse = penEvent.is_mouse; // no mouse with bamboo(?)
		bamboo_pen_packet.inrange = penEvent.proximity;

		if(bamboo_pen_packet.inrange)
		{
			PenDataTransform::XformedData xformed;

			PenDataTransform::transform_pen_data(penEvent, xformed,
								false); // no tilt

			bamboo_pen_packet.x = xformed.x;
			bamboo_pen_packet.y = xformed.y;
			bamboo_pen_packet.pressure = xformed.pressure;
			bamboo_pen_packet.proximity = penEvent.proximity;
			bamboo_pen_packet.touch = penEvent.touch;
			bamboo_pen_packet.eraser = penEvent.eraser;
			bamboo_pen_packet.button0 = penEvent.button0;
			bamboo_pen_packet.button1 = penEvent.button1;
			bamboo_pen_packet.distance = 0x1A; // not sure if that value is used(?)
			bamboo_pen_packet.outofprox = 0;

			shouldfill = true;
		}
		else
		{
			bamboo_pen_packet.x = 0;
			bamboo_pen_packet.y = 0;
			bamboo_pen_packet.proximity = 0;
			bamboo_pen_packet.pressure = 0;
			bamboo_pen_packet.distance = 0;
			bamboo_pen_packet.touch = 0;
			bamboo_pen_packet.eraser = 0;
			bamboo_pen_packet.button0 = 0;
			bamboo_pen_packet.button1 = 0;
			bamboo_pen_packet.outofprox = 1;

#ifdef DEBUG_SUPPORT
			DebugProc::proxOutTrigger();
#endif
		}

		if(console::console_enabled)
		{
			console::print("[USB Packet - prox:");
			console::printbit(bamboo_pen_packet.proximity);
			console::print(" outofprox:");
			console::printbit(bamboo_pen_packet.outofprox);
			console::print(" range:");
			console::printbit(bamboo_pen_packet.inrange);
			console::print(" touc:");
			console::printbit(bamboo_pen_packet.touch);
			console::print(" era:");
			console::printbit(bamboo_pen_packet.eraser);
			console::print(" b0:");
			console::printbit(bamboo_pen_packet.button0);
			console::print(" b1:");
			console::printbit(bamboo_pen_packet.button1);
			console::print(" x=");
			console::printNumber(bamboo_pen_packet.x);
			console::print(", y=");
			console::printNumber(bamboo_pen_packet.y);
			console::print(", pressure=");
			console::printNumber(bamboo_pen_packet.pressure);
			console::print(", distance=");
			console::printNumber(bamboo_pen_packet.distance);
			console::println("]");
		}

		if(extdata_getValue8(EXTDATA_USB_PORT) == EXTDATA_USB_PORT_DIGITIZER)
			UsbUtil::send_packet_fill_idle_time(bamboo_pen_packet.buffer, 9, WACOM_PEN_ENDPOINT, 50);
	}

	struct bamboo_touch_struct
	{
		union
		{
			uint8_t buffer[20];
			struct
			{
				uint8_t hid_identifier; // 0x2
			};
		};
	} bamboo_touch_packet;

	void send_bamboo_touch_packet(Touch::TouchEvent& touchEvent)
	{
		if(!touchEvent.touch)
		{
			for(int i=0;i<20;i++)
				bamboo_touch_packet.buffer[i] = 0;
			bamboo_touch_packet.hid_identifier = 0x2;
		}
		else
		{
			return;
		}

		if(extdata_getValue8(EXTDATA_USB_PORT) == EXTDATA_USB_PORT_DIGITIZER)
			UsbUtil::send_packet(bamboo_touch_packet.buffer, 20, WACOM_PEN_ENDPOINT, 50);
	}

	//-----------------------------------------------------------------
	// USB Protocol V (5)
	//
	// Data gleaned for the Intuos2
	// Button 0 = side switch near the tip (not working with eraser)
	// Button 1 = side switch away from the tip (not working with eraser)
	//
	//	02 C2 85 20 16 00 2C D0 00 00   In Range: NORMAL PEN
	//	02 C2 85 A0 16 00 2C D0 00 00   In Range: ERASER (PEN)
	//
	//	02 A0 91 06 66 9F 00 20 40 B8   Pen Data (see below)
	//	...
	//	...
	//
	//	02 80 00 00 00 00 00 00 00 00   Out of range
	//
	//
	// Pen Data:	02 A0 91 06 66 9F 00 20 40 B8
	//
	// [0]	02  HID report identifier (02)
	// [1]	A0  1 1 p 0  0 b c i   p = prox, b = button1, c = button0, i = tool index
	// [2]	91  x x x x  x x x x   x pos
	// [3]	06  x x x x  x x x x
	// [4]	66  y y y y  y y y y   y pos
	// [5]	9F  y y y y  y y y y
	// [6]	00  p p p p  p p p p   pressure (1024)
	// [7]	20  p p t t  t t t t   t = tiltx - 0 = -60 degrees, 40 = 0, 7F = +60 degrees
	// [8]	40  t u u u  u u u u   u = tilty - 0 = -60 degrees, 40 = 0, 7F = +60 degrees
	// [9]	B8  d d d d  d 0 0 0   d = sensor z distance 0x26..0x0D
	//
	// touch = pressure > 10
	//
	//-----------------------------------------------------------------

	struct protocol5_struct
	{
		union
		{
			uint8_t buffer[10];
			struct
			{
				uint8_t hid_identifier; // 0x02
				struct
				{
					unsigned toolindex:1;// bit 0 - tool index -- set to 0 -- Intuos4/5: pressure low data extra bit
					unsigned button0:1; // bit 1 - first button on the pen
					unsigned button1:1; // bit 2 - second button on the pen
					unsigned bit3:1;    // bit 3 - set to 0 (for pen)
					unsigned bit4:1;    // bit 4 - set to 0 (for pen)
					unsigned proximity:1;//bit 5 - when 0, pen is far from the surface but
							    // mouse is moving in Windows.
					unsigned bit6:1;    // bit 6 - set to 1 (for pen)
					unsigned bit7:1;    // bit 7 - set to 1 (for pen)
				};
				uint8_t x_high;
				uint8_t x_low;
				uint8_t y_high;
				uint8_t y_low;
				uint8_t byte6;			// pressure high
				uint8_t byte7;			// pressure + tiltx
				uint8_t byte8;			// tiltx + tilty
				unsigned y_lowest:1;		// for Intuos3/4/5. (Intuos2 = 0)
				unsigned x_lowest:1;		// for Intuos3/4/5. (Intuos2 = 0)
				unsigned distance_lowest:1;	// for Intuos3/4/5. (Intuos2 = 0)
				unsigned distance:5;		// distance from the sensor -
								// Intuos2: seen numbers from 0x26 (far) to 0x0D (close) (bits 3..7)
			};
		};
	} protocol5_packet;

	static bool currentlyInRange = false;

	static void do_send_protocol5_packet(Pen::PenEvent& penEvent)
	{
		bool shouldfill = false;

		uint8_t* pbuf = protocol5_packet.buffer;

		*pbuf++ = 0x02; // [0] - HID report identifier

		if(penEvent.proximity)
		{
			if(currentlyInRange)
			{
				if(penEvent.is_mouse == false)
				{
					// normal pen packet
					protocol5_packet.toolindex = 0;
					protocol5_packet.button0 = penEvent.button0;
					protocol5_packet.button1 = penEvent.button1;
					protocol5_packet.bit3 = 0;
					protocol5_packet.bit4 = 0;
					protocol5_packet.proximity = penEvent.proximity; // TODO: Does this work correctly?
					protocol5_packet.bit6 = 1;
					protocol5_packet.bit7 = 1;

					// Bizarre discovery: Wondering if bit 6 was actually proximity, I set proximity to 0
					// and set bit6 = penEvent.proximity and when I put an Intuos1 pen on the tablet it 
					// started cycling through a bunch of weird tools, many of which I don't think actually
					// exist such as "10 button stylus" and "Wood pen".

					PenDataTransform::XformedData xformed;

					PenDataTransform::transform_pen_data(penEvent, xformed,
										true); // with tilt

					protocol5_packet.x_high = ((xformed.x >> 8) & 0xff);
					protocol5_packet.x_low = (xformed.x & 0xff);

					protocol5_packet.y_high = ((xformed.y >> 8) & 0xff);
					protocol5_packet.y_low = (xformed.y & 0xff);

					uint8_t tiltx = xformed.tilt_x + 0x40;
					uint8_t tilty = xformed.tilt_y + 0x40;

					if((penEvent.serial_packet_first_byte & 0xB8) != 0xA0)
					{
						// TODO: Test this code with an actual airbrush.
						// This is an airbrush "second packet".  The only difference between the first and second
						// packets is that the absolute wheel position replaces the bits that contain the pressure
						// value in the first packet and all buttons are ignored.
						xformed.pressure = penEvent.abswheel;

						// Also, to identify that this is an airbrush second packet using the USB protocol,
						// (data[1] & 0xbc) == 0xb4 must be true.
						// 0xbc: 1011 1100
						// 0xb4: 1011 0100
						// So basically we need to send a value like this (where * bits can have changing values):
						//       1*11 01**
						// 
						// The three unknown * bits are like a 3 bit number with 8 possible values.  I tried
						// sending each of these values using debug packet playback and found the following:
						// B4 and F4 seem to give correct results.
						// B6 and F6 change the wheel value to negative in the "Diagnose" panel of the Wacom
						// control panel.  Since this bit isn't interpreted by the linux driver, I assume this 
						// is a bug or the start of a feature not implemented.
						// F5, B5, B7, and F7 all have bit 0 set which is supposed to be the tool index bit.
						// When any of these values are passed, no wheel position is detected by the Diagnose
						// panel and even the pen position packets are mostly ignored.
						// So B4 and F4 are the only working values.  I'm choosing B4 since Linux is actually
						// checking for a B4 value.
						// 0xb4 = 1011 0100
						// The standard data[1] we send with a normal pen packet is (B are buttons, P is proximity):
						//        11P0 0BB0
						// So we need to set button1 and bit4 to 1 and button0 and bit6 to 0.  Other bits are already
						// set correctly.
						protocol5_packet.button0 = 0;
						protocol5_packet.button1 = 1;
						protocol5_packet.bit4 = 1;
						protocol5_packet.bit6 = 0;

						// According to the linux driver, pressure, tilt, and the side button state are ignored
						// unless (data[1] & 0xb8) == 0xa0:
						// 0xb8: 1011 1000
						// 0xa0: 1010 0000
						// So we'd need to send 1*10 0*** but that isn't compatible with sending 1*11 01**
						// so pressure, tilt, and the side button state in the second packet would be ignored
						// except that the linux driver interprets just the tilt values when (data[1] & 0xbc) == 0xb4.
						// So we can expect tilt to change based on the second packet, but not pressure (which
						// is replaced by the wheel value) or button state.
					}

					uint16_t pressure = xformed.pressure;

					if(usbProtocol == EXTDATA_USB_PROTOCOL_WACOM_INTUOS5)
					{
						// extra pressure range for INTUOS4/5
						protocol5_packet.toolindex = pressure & 1;
						pressure = pressure >> 1;
					}

					protocol5_packet.byte6 = (pressure >> 2) & 0xFF;
					protocol5_packet.byte7 = ((pressure & 0x3) << 6) | ((tiltx >> 1) & 0x3F);
					protocol5_packet.byte8 = ((tiltx << 7) & 0x80) | ( tilty & 0x7F);

					protocol5_packet.distance = 0x0D; // The serial protocol doesn't include a distance value, but it does include the PROXIMITY_BIT

/*					if(console::console_enabled)
					{
						console::print("[USB Packet - prox:");
						console::printbit(protocol5_packet.proximity);
						console::print(" b0:");
						console::printbit(protocol5_packet.button0);
						console::print(" b1:");
						console::printbit(protocol5_packet.button1);
						console::print(" x:");
						console::printNumber((((uint16_t)protocol5_packet.x_high) << 8) | protocol5_packet.x_low);
						console::print(" y:");
						console::printNumber((((uint16_t)protocol5_packet.y_high) << 8) | protocol5_packet.y_low);
						console::print(" pressure:");
						console::printNumber((((uint16_t)protocol5_packet.byte6) << 2) |
								(((uint16_t)protocol5_packet.byte7) >> 6));
						console::print(" byte6,7,8:");
						console::printHex(protocol5_packet.byte6,2);
						console::printHex(protocol5_packet.byte7,2);
						console::printHex(protocol5_packet.byte8,2);
						console::println("]");
					}
*/				}
				else
				{
					// data[1] must be set so the following if statement is true (from the Linux driver):
					// if ((data[1] & 0xbc) == 0xa8 || (data[1] & 0xbe) == 0xb0 || (data[1] & 0xbc) == 0xac) {
					// 0xbc: 1011 1100
					// 0xbe: 1011 1110
					// 0xa8: 1010 1000
					// 0xb0: 1011 0000
					// 0xac: 1010 1100
					// So the if statement is saying:
					// If bit 7=1, 5=1, 4=0, 3=1, 2=0       // Theory: 4D mouse
					//    ||  7=1, 5=1, 4=1, 3=0, 2=0, 1=0  // Theory: Lens cursor packet
					//    ||  7=1, 5=1, 4=0, 3=1, 2=1       // Theory: 2D mouse and/or maybe marker pen
					// We later see that bit 1 is set only for a rotation packet.
					// bit 4 is cleared only for 4D mouse packets.
					// bit 3 is never referenced, but it's set to 0 only when bit 4 is set, so I'll assume it varies with bit 4.
					// bit 2 is never referenced but when it's set, the mouse's rotation and throttle wheel stop working so I would guess this bit means 2D mouse.
					// bit 5 is the "proximity" bit (set later), according to protocol5_struct, so if that bit isn't set, the linux
					// driver will only pay attention to the position of the mouse and not its buttons, rotation, etc.
					// Through experimentation I found that bit 6 must be set to 1 or the Wacom driver will not detect any button presses from the mouse.  It does still detect mouse position, rotation, and throttle position.
					*pbuf++ = 0xe0; // data[1] with bit 7, 6, and 5 set.

					// These bytes will later be filled with x/y position.
					*pbuf++ = 0; // data[2]
					*pbuf++ = 0; // data[3]
					*pbuf++ = 0; // data[4]
					*pbuf++ = 0; // data[5]
					
					int throttle = 0;
					int rotation = 0;

					if(LENS_CURSOR(penEvent)) {
						// Construct lens cursor packet (tested using a 4D mouse to spoof a lens cursor tool id)
						protocol5_packet.bit4 = 1;

						*pbuf++ = 0; // data[6]
						*pbuf++ = 0; // data[7]
						*pbuf++ = penEvent.buttons & 0x1f; // data[8] bits: 0:left btn, 1:mid btn, 2:right btn, 3:lower right button, 4:lower left button
					}
					else if (MOUSE_2D(penEvent))
					{
						// Construct 2D mouse packet
						// TODO: Test this code with a 2D mouse
						protocol5_packet.bit3 = 1;
						protocol5_packet.button1 = 1;
						
						*pbuf++ = 0; // data[6]
						*pbuf++ = 0; // data[7]

						// A 2d mouse has only left and right buttons plus a scroll wheel that can be clicked
						// to send a middle button.  The scroll wheel apparently works by sending an "up" or
						// "down" bit rather than sending an absolute value or a "throttle" value like the 4D mouse.
						// penEvent.relwheel will be 0 if no scroll wheel movement was received, or -1 or 1 if an
						// up or down wheel turn was received.
						// It looks like the serial protocol interprets bit 0 as -1 and bit 1 as +1, but I think
						// the USB protocol does the opposite.
						// USB is expecting these bits to be set in data[8] (R=right btn, M=middle btn, L=left btn,
						// D=down scroll wheel, U=up scroll wheel, *=doesn't matter, although two of these bits
						// are used by the Intuos3 mouse to represent two side buttons):
						//   ***RMLDU
						*pbuf++ = ((penEvent.buttons & 0x07) << 2) 
							      | (penEvent.relwheel > 0 ? 0x01 : 0x00)
								  | (penEvent.relwheel < 0 ? 0x02 : 0x00);
					}
					else
					{
						// Construct 4D mouse packet
						protocol5_packet.bit3 = 1;

						if((penEvent.serial_packet_first_byte & 0xbe) == 0xaa)
						{
							// This is a 4D mouse "second packet" containing rotational information.
							protocol5_packet.button0 = 1; // Set bit 1 to indicate this is the "second packet".

							// In the serial protocol, rotation is measured by an 11 bit number (11 bits stores 0-2047).
							// By looking at the linux driver and by rotating the mouse, I found that with the mouse held
							// straight up, rotation_z jumps between 0 and 1799.  Rotating clockwise it decreases below 1799,
							// 1350 at 90 degrees, 900 at 180, 450 at 270.
							// In USB, rotation gets 10 bits plus a single +/- bit, so we can represent -1023 to 1023.
							// By rotating the mouse in About > Diagnose I found that a value of -1 represents 359.9 degrees,
							// -3 is around 358, etc.  A value of 125 is about 25 deg, while 1 is about 0 deg.  So they're
							// essentially representing 0 (straight up) to 180 degrees with 0 to 900 rotating clockwise,
							// then it jumps to around -900 and returns to 0 as you continue to rotate clockwise.
							rotation = penEvent.rotation_z;
							if(rotation < 900)
							{
								// Rotation must be shifted left by 1 bit to make room to use bit 0 to represent
								// positive or negative.  Why the heck they made the formatting so confusing is beyond me.
								rotation = (rotation << 1) | 0x1; // Bit 0 is set when rotation is negative.
							}
							else
							{
								rotation = 1799 - rotation;
								rotation = rotation << 1;
							}
							*pbuf++ = (rotation & 0x7f8) >> 3; // data[6] holds the first 8 bits
							*pbuf++ = (rotation & 0x07) << 5; // data[7]: bits 7-5 contain bits 2-0 of rotation_z, bit 5 also indicates whether the rotation is negative or not.
							*pbuf++ = 0; // data[8]
							//*pbuf++ = (penEvent.buttons & 0x07) | ((penEvent.buttons & 0x18) << 1); // data[8] bits: 0:left btn, 1:mid btn, 2:right btn, 3:throttle wheel has negative value when set, 4:lower right button, 5:lower left button
						}
						else
						{
							// The serial data contains throttle values ranging from -1023 to 1023 using 10 bits for the value plus
							// 1 bit for the +/-.  The USB data only supports an 8-bit value plus 1 bit for +/-, so we must scale
							// throttle to range from -255 to +255 using 8 bits for the number and 1 bit for the +/-
							throttle = penEvent.throttle;
							if(throttle < 0) {
								throttle = -throttle;
							}
							throttle >>= 2;

							*pbuf++ = (throttle & 0xfc) >> 2; // data[6] stores bits 7-2 of throttle
							*pbuf++ = (throttle & 0x03) << 6; // data[7] stores bits 1-0 of throttle
							*pbuf++ = (penEvent.buttons & 0x07) | ((penEvent.buttons & 0x18) << 1) | (penEvent.throttle < 0 ? 0x08 : 0); // data[8] bits: 0:left btn, 1:mid btn, 2:right btn, 3:throttle wheel has negative value when set, 4:lower right button, 5:lower left button
						}
					}

					// TODO: If setting X/Y here works, set it in one shared place for pen and mouse events.
					protocol5_packet.proximity = penEvent.proximity;
					//protocol5_packet.bit6 = penEvent.proximity;

					PenDataTransform::XformedData xformed;

					PenDataTransform::transform_pen_data(penEvent, xformed,
										false); // without tilt
				
					// This will overwrite data[2] - [5] and the last 5 bits of [9]:
					protocol5_packet.x_high = ((xformed.x >> 8) & 0xff);
					protocol5_packet.x_low = (xformed.x & 0xff);

					protocol5_packet.y_high = ((xformed.y >> 8) & 0xff);
					protocol5_packet.y_low = (xformed.y & 0xff);

					protocol5_packet.distance = 0x0D;

					/*
					if(protocol5_packet.button0 == 0) {
						console::print("SB: ");
						console::printNumber(penEvent.throttle);
						console::print(" ");
						console::printNumber(throttle);
						console::print(" ");
						console::printHex(protocol5_packet.byte8, 2);
						console::println();
					}
					if(protocol5_packet.button0 == 1) {
						console::print("SB: ");
						console::printNumber(penEvent.rotation_z);
						console::print(" ");
						console::printNumber(rotation);
						console::print(" ");
						console::printHex(protocol5_packet.byte8, 2);
						console::println();
					}*/
					/*
					if(protocol5_packet.button0 == 0) {
						console::print("SB: ");
						console::printHex(protocol5_packet.byte8, 2);
						console::println();
					}*/
				}
			}
			else
			{
				currentlyInRange = true;

				// Tell the Wacom driver a tool has come into proximity.
				//
				// Example Intuos2 pen and eraser proximity packets:
				//   02 C2 85 20 16 00 2C D0 00 00   In Range: NORMAL PEN
				//   02 C2 85 A0 16 00 2C D0 00 00   In Range: ERASER (PEN)
				// Tool id is 85<<4 | 20>>4 | D0&0f<<20 | 00&f0<<12 = 850 | 2 | 0 | 0 = 0x000852
				// Serial number is 20&0f<<28 + 16<<20 + 00<<12 + 2c<<4 + d0>>4 = 0+1600000+0+2c0+d = 0x016002CD
				//
				//   02 C2 85 A0 16 00 2C D0 00 00   In Range: ERASER (PEN)
				// Tool id is 85<<4 | 20>>4 | D0&0f<<20 | 00&f0<<12 = 850 | 2 | 0 | 0 = 0x00085A
				// Serial number (is the same) = 0x016002CD
				//
				// Intuos5 touch S pen proximity packet:
				//   02 C2 80 22 28 08 26 91 00 00   In Range: Intuos5(?) Pen
				//   02 C2 80 A2 28 08 26 91 00 00  eraser
				// Tool id is 80<<4 | 22>>4 | 91&0f<<20 | 00&f0<<12 = 800 | 2 | 100000 | 0 = 0x100802 pen
				// Tool id is 80<<4 | A2>>4 | 91&0f<<20 | 00&f0<<12 = 800 | A | 100000 | 0 = 0x10080A eraser
				// Serial number is = 0x22808269
				//

				//
				/*
				*pbuf++ = 0xC2; // [1]
				*pbuf++ = 0x85; // [2]

				if(penEvent.eraser)
					*pbuf++ = 0xA0; // [3]
				else
					*pbuf++ = 0x20; // [3]

				*pbuf++ = 0x16; // [4]
				*pbuf++ = 0x00; // [5]
				*pbuf++ = 0x2C; // [6]
				*pbuf++ = 0xD0; // [7]
				*pbuf++ = 0x00; // [8]
				*pbuf   = 0x00; // [9]*/


				// From http://git.yoctoproject.org/cgit/cgit.cgi/linux-yocto-2.6.37/plain/drivers/input/tablet/wacom_wac.c
				// data[1] & 0x01 contains the "tool number".  Since this is a single bit, I'm guessing it's used to
				// track up to two tools being used at the same time?  We'll just set it to 0.
				// (data[1] & 0xfc) == 0xc0 must be true to be considered a "tool in proximity" packet.
				// In the captured packets from an Intuos2 pen, this byte contained 0xc2, but the source code above
				// doesn't use the "2" bit for anything.  Still, we may as well replicate the captured data.
				*pbuf++ = 0xC2; // data[1]

				// This is how the tool type is decoded by the USB linux driver:
				//   (data[2] << 4) | (data[3] >> 4) | ((data[7] & 0x0f) << 20) | ((data[8] & 0xf0) << 12);
				// And this is how the serial is decoded:
				//   ((data[3] & 0x0f) << 28) + (data[4] << 20) + (data[5] << 12) + (data[6] << 4) + (data[7] >> 4);
				// We need to reverse these equations to encode the tool id and serial.  If you look at the equations,
				// Tool id (T) is encoded using 20 bits and serial (S) is encoded using 32 bits.  However, notice that
				// tool id decodes to a 24 bit number where only bits 23-16, 11-0 are assigned values stored in the
				// encoded data.  It would be logical to place bits 15-12 as the last 4 bits of data[8] but the
				// decoding equation ignores those bits using & 0xf0.  That could be a bug in the decoding equation.
				// Either way, these bits are all 0 in the data we plan to send so it doesn't matter for our purposes.
				// The tool id bits are spread out in a strange pattern, presumably because tool type started out as 12 bits
				// and was later padded to 24 by placing additional bits in the last half of data[7] and the first
				// half of data[8].
				// data[2]  [3]      [4]      [5]      [6]      [7]      [8]
				// TTTTTTTT TTTTSSSS SSSSSSSS SSSSSSSS SSSSSSSS SSSSTTTT TTTT????
				// Bit numbers (a 3 above a 1 means bit 31):
				// 11987654 32103322 22222222 11111111 11987654 32102222 1111
				// 10           1098 76543210 98765432 10           3210 9876

				// Some places that call do_send_protocol5_packet don't set the toolid or serial, so in
				// that case, assign generic values.
				uint32_t tool_id;

				if(penEvent.tool_id == 0)
				{
					if(usbProtocol == EXTDATA_USB_PROTOCOL_WACOM_INTUOS5)
					{
						if(penEvent.eraser)
						{
							LED_TOGGLE;
							tool_id = 0x10080A; // Eraser tool id (intuos 5)
						}
						else
							tool_id = 0x100802; // Pen tool id  (intuos 5)
					}
					else
					{
						if(penEvent.eraser)
							tool_id = 0x85A; // Eraser tool id  (intuos 2)
						else
							tool_id = 0x852; // Pen tool id  (intuos 2)
					}
				}
				else
				{
					tool_id = penEvent.tool_id;
				}

				uint32_t tool_serial_num;

				if(penEvent.tool_serial_num == 0)
				{
					if(usbProtocol == EXTDATA_USB_PROTOCOL_WACOM_INTUOS5)
						tool_serial_num = 0x22808269U; // Intuos 5 pen serial number (same for both the pen and eraser side)
					else
						tool_serial_num = 0x016002CDU; // Intuos 2 pen serial number (same for both the pen and eraser side)
				}
				else
					tool_serial_num = penEvent.tool_serial_num;
				
				*pbuf++ = tool_id >> 4; // data[2]
				*pbuf++ = ((tool_id & 0x0f) << 4) | ((tool_serial_num & 0xf0000000) >> 28); // data[3]
				*pbuf++ = (tool_serial_num & 0xff00000) >> 20; // data[4]
				*pbuf++ = (tool_serial_num & 0xff000) >> 12; // data[5]
				*pbuf++ = (tool_serial_num & 0xff0) >> 4; // data[6]
				*pbuf++ = ((tool_serial_num & 0xf) << 4) | ((tool_id & 0xf00000) >> 20); // data[7]
				*pbuf++ = (tool_id & 0xf0000) >> 12; // data[8]

				// I don't think data[9] means anything, but the captured data contained 0, so set to 0.
				*pbuf = 0; // data[9]

/*				if(console::console_enabled)
				{
					console::printP(STR_USB_PACKET_IN_RANGE_PACKET_ERASER);
					console::printbit(penEvent.eraser);
					console::println("]");
				}
*/			}

			shouldfill = true;
		}
		else if(currentlyInRange)
		{
			currentlyInRange = false;

			// out of range  -- 02 80 00 00 00 00 00 00 00 00

			*pbuf++ = 0x80; // [1]
			*pbuf++ = 0x00; // [2]
			*pbuf++ = 0x00; // [3]
			*pbuf++ = 0x00; // [4]
			*pbuf++ = 0x00; // [5]
			*pbuf++ = 0x00; // [6]
			*pbuf++ = 0x00; // [7]
			*pbuf++ = 0x00; // [8]
			*pbuf   = 0x00; // [9]

/*			if(console::console_enabled)
			{
				console::printlnP(STR_USB_PACKET_OUT_OF_RANGE_PACKET_ALL_ZEROS);
			}
*/
#ifdef DEBUG_SUPPORT
			DebugProc::proxOutTrigger();
#endif
		}
		else
		{
			// invalid state -- do not send anything
			return;
		}

		#ifdef DEBUG_USB_DATA
/*			console::print("SU: "); // Sending USB data.  Keep these debug statements short or it seems to overflow the USB buffer
			for(int i = 0; i < 10; i++) {
				console::printHex(protocol5_packet.buffer[i], 2);
				console::print(" ");
			}
			console::println();
*/		#endif
		if(extdata_getValue8(EXTDATA_USB_PORT) == EXTDATA_USB_PORT_DIGITIZER)
		{
			uint8_t endPoint;

			if(usbProtocol == EXTDATA_USB_PROTOCOL_WACOM_INTUOS5)
				endPoint = WACOM_INTUOS5_PEN_ENDPOINT;  // Pen data on Endpoint 3 (0x83) with Intuos5
			else
				endPoint = WACOM_PEN_ENDPOINT; // Pen data on Endpoint 1 (0x81) with Intuos2

			UsbUtil::send_packet_fill_idle_time(protocol5_packet.buffer, 10, endPoint, 50);
		}
	}

	bool in_special_operation = false;
	bool special_op_inrange_state = false;

	void send_protocol5_packet(Pen::PenEvent& penEvent)
	{
		if(in_special_operation)
			return; // already in a special operation!

		do_send_protocol5_packet(penEvent);
	}

	/**
	 * get the device out of proximity momentarily
	 */
	void begin_special_operation_protocol5()
	{
		if(in_special_operation)
			return;	// already in a special operation!

		in_special_operation = true;
		special_op_inrange_state = currentlyInRange;

		if(currentlyInRange)
		{
			Pen::PenEvent penEvent;

			penEvent.proximity = false;

			do_send_protocol5_packet(penEvent);
		}
	}

	/**
	 * return to the state before the device was out of prox momentarily
	 */
	void end_special_operation_protocol5()
	{
		if(!in_special_operation)
			return;	// was not in a special operation

		in_special_operation = false;

		if(special_op_inrange_state)
		{
			// do nothing. Let the next pen movement re-initialize the status
		}

		special_op_inrange_state = false;
	}
}

