/*
 * touch_event.cpp
 *
 *  Created on: 2011-08-10
 *      Author: Bernard
 */

#include <stdint.h>

#include "console.h"
#include "extdata.h"
#include "touch_event.h"
#include "wacom_usb.h"

/** Finger Touch-related stuff
 *
 * NOTE: Touch::init() needs to be called at startup
 */
namespace Touch
{
	static uint8_t	usbProtocol;

	void init()
	{
		usbProtocol = extdata_getValue8(EXTDATA_USB_PROTOCOL);
	}

	bool touchEnabled()
	{
		return usbProtocol == EXTDATA_USB_PROTOCOL_WACOM_BAMBOO_TOUCH;
	}

	/** redirects to USB emulation specific logic */
	void output_touch_event(Touch::TouchEvent& touchEvent)
	{
		switch(usbProtocol)
		{
			case EXTDATA_USB_PROTOCOL_WACOM_BAMBOO_TOUCH:
				WacomUsb::send_bamboo_touch_packet(touchEvent);
				break;
			default:
				break;
		}
	}
}
