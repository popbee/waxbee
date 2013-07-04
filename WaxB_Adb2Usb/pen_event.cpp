/*
 * pen_events.cpp
 *
 *  Created on: 2010-12-26
 *      Author: Bernard
 */
#include "featureinclusion.h"

#include <stdint.h>

#include "console.h"
#include "extdata.h"
#include "pen_event.h"
#include "wacom_usb.h"
#include "debugproc.h"
#include "pen_button_handler.h"

/** Input tablet common logic and 'PenEvent' structure
 *
 * NOTE: Pen::init() needs to be called at startup
 */
namespace Pen
{
	static uint8_t	usbProtocol;
	static uint16_t usbPressureMax;
	static uint8_t	buttonEncoding;

	void init()
	{
		usbProtocol = extdata_getValue8(EXTDATA_USB_PROTOCOL);
		usbPressureMax = extdata_getValue16(EXTDATA_USB_PRESSURE_MAX);
		buttonEncoding = extdata_getValue8(EXTDATA_USB_BUTTON_ENCODING);
	}

	/** redirects to specific USB emulation logic */
	void output_pen_event(Pen::PenEvent& penEvent)
	{
		switch(usbProtocol)
		{
#ifdef USB_PROTOCOL_IV_SUPPORT
			case EXTDATA_USB_PROTOCOL_WACOM_PROTOCOL4:
				WacomUsb::send_protocol4_packet(penEvent);
				break;
#endif
#ifdef TOUCH_SUPPORT
			case EXTDATA_USB_PROTOCOL_WACOM_BAMBOO_TOUCH:
				WacomUsb::send_bamboo_pen_packet(penEvent);
				break;
#endif
#ifdef USB_PROTOCOL_V_SUPPORT
			case EXTDATA_USB_PROTOCOL_WACOM_PROTOCOL5:
			case EXTDATA_USB_PROTOCOL_WACOM_INTUOS5:
				WacomUsb::send_protocol5_packet(penEvent);
				break;
#endif
			default:
				break;
		}
	}

	void input_pen_event(Pen::PenEvent& penEvent)
	{
		// active area selection
		PenButtonHandler::ActiveArea activeArea;

		if(PenButtonHandler::findPenButton(penEvent, activeArea))
		{
			// button area
			PenButtonHandler::handle_pen_button_event(penEvent, activeArea);
		}
		else
		{
			// normal pen area
			output_pen_event(penEvent);
		}
	}
}
