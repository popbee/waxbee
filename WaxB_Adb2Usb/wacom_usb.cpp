/*
 *
 * Wacom-related stuff
 *
 *
 */

#include "usb.h"
#include "extdata.h"
#include "console.h"
#include "wacom_usb.h"
#include "pen_events.h"

namespace WacomUsb
{
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


	static Pen::PenEvent penEvent;

	void send_protocol4_packet(Pen::PenEvent& f_penEvent)
	{
		// copy event for direct access (TODO: does that make a difference?)
		penEvent = f_penEvent;

		protocol4_packet.hid_identifier = 0x02;

		protocol4_packet.is_mouse = penEvent.is_mouse;
		protocol4_packet.proximity = penEvent.proximity;
		protocol4_packet.inrange = penEvent.proximity;
		protocol4_packet.reserved = 0;

		if(protocol4_packet.inrange)
		{
			protocol4_packet.x = Pen::compute_x_position(penEvent.x);
			protocol4_packet.y = Pen::compute_y_position(penEvent.y);
			protocol4_packet.pressure = Pen::compute_pressure(penEvent.pressure);
			protocol4_packet.touch = penEvent.touch;
			protocol4_packet.eraser = penEvent.eraser;
			protocol4_packet.button0 = penEvent.button0;
			protocol4_packet.button1 = penEvent.button1;
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
			usb_rawhid_send(protocol4_packet.buffer, 50);
	}

	void send_protocol5_packet(Pen::PenEvent& f_penEvent)
	{
	}
}

