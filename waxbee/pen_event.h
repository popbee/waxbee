/*
 * pen_event.h
 *
 * This is the "universal" pen event structure and api.
 *
 *  Created on: 2010-12-26
 *      Author: Bernard
 */

#ifndef PEN_EVENT_H_
#define PEN_EVENT_H_

#define DEBUG_SERIAL_DATA
#define DEBUG_USB_DATA

namespace Pen
{
	class PenEvent
	{
	public:
		// serial_packet_first_byte 0xE0 is the value used for a standard stylus position update.
		// It's important to use this value for non-Intuos tablets that may not explicitly
		// set serial_packet_first_byte or their penEvents could have problems like being
		// misinterpreted as Intuos Airbrush "second packets".
		PenEvent() : tool_id(0), tool_serial_num(0), serial_packet_first_byte(0xE0) { };

		bool proximity:1;
		bool eraser:1;
		bool is_mouse:1; // 0=pen, 1=mouse
		bool button0:1;
		bool button1:1;

		bool touch:1; // pen is touching the surface

		uint16_t x;	// "slave" tablet coordinates
		uint16_t y;	// "slave" tablet coordinates
		uint16_t pressure; // "slave" tablet pressure value

		int16_t tilt_x; // this is signed
		int16_t tilt_y; // this is signed

		int16_t rotation_z; // this is signed

		uint32_t tool_id;
		uint32_t tool_serial_num;

		// For mouse support:
		bool discard_first;
		uint8_t serial_packet_first_byte;
		int16_t throttle;
		uint8_t buttons;
		int8_t relwheel;

		// For airbrush support:
		int16_t abswheel;
	};

	void init();

	void input_pen_event(Pen::PenEvent& penEvent);
}


#define HEADER_BIT      0x80
#define ZAXIS_SIGN_BIT  0x40
#define ZAXIS_BIT       0x04
#define ZAXIS_BITS      0x3f
#define POINTER_BIT     0x20
#define PROXIMITY_BIT   0x40
#define BUTTON_FLAG     0x08
#define BUTTONS_BITS    0x78
#define TILT_SIGN_BIT   0x40
#define TILT_BITS       0x3f

/* defines to discriminate second side button and the eraser */
#define ERASER_PROX     4
#define OTHER_PROX      1

/*
#define DEVICE_ID(flags) ((flags) & 0x07)

#define STYLUS_ID               1
#define CURSOR_ID               2
#define ERASER_ID               4
#define ABSOLUTE_FLAG           8
#define KEEP_SHAPE_FLAG         16
#define BAUD_19200_FLAG         32
#define BETA_FLAG               64
#define BUTTONS_ONLY_FLAG       128

#define IsCursor(priv) (DEVICE_ID((priv)->flags) == CURSOR_ID)
#define IsStylus(priv) (DEVICE_ID((priv)->flags) == STYLUS_ID)
#define IsEraser(priv) (DEVICE_ID((priv)->flags) == ERASER_ID)*/

#define PEN(penEvent)         (((penEvent.tool_id) & 0x07ff) == 0x0022 \
                        || ((penEvent.tool_id) & 0x07ff) == 0x0042 \
                        || ((penEvent.tool_id) & 0x07ff) == 0x0052)
#define STROKING_PEN(penEvent) (((penEvent.tool_id) & 0x07ff) == 0x0032)
#define AIRBRUSH(penEvent)    (((penEvent.tool_id) & 0x07ff) == 0x0112)
#define MOUSE_4D(penEvent)    (((penEvent.tool_id) & 0x07ff) == 0x0094)
#define MOUSE_2D(penEvent)    (((penEvent.tool_id) & 0x07ff) == 0x0007)
#define LENS_CURSOR(penEvent) (((penEvent.tool_id) & 0x07ff) == 0x0096)
#define INKING_PEN(penEvent)  (((penEvent.tool_id) & 0x07ff) == 0x0012)
#define STYLUS_TOOL(penEvent) (PEN(penEvent) || STROKING_PEN(penEvent) || INKING_PEN(penEvent) || \
                        AIRBRUSH(penEvent))
#define CURSOR_TOOL(penEvent) (MOUSE_4D(penEvent) || LENS_CURSOR(penEvent) || MOUSE_2D(penEvent))

#endif /* PEN_EVENT_H_ */
