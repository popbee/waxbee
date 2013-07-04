/*
 * usb_util.cpp
 *
 *  Created on: 2011-08-10
 *      Author: Bernard
 */

#include <stdint.h>

#include "extdata.h"
#include "usb_util.h"
#include "usb.h"
#include "console.h"
#include "strings.h"
#include <string.h>

/** Special USB port logic
 *
 * NOTE: UsbUtil::init() needs to be called at startup
 */
namespace UsbUtil
{
	static uint8_t filler_countdown_5ms;
	static uint8_t dummy_intuos5_touch_filler_countdown_5ms;
	static uint16_t dummy_report192_intuos5_countdown_5ms;

	static uint8_t repeat_buffer[USB_MAX_PACKET_SIZE];
	static uint8_t repeat_endpoint;
	static uint8_t repeat_timeout;
	static uint8_t repeat_size;

	void init()
	{
		if(extdata_getValue8(EXTDATA_USB_PROTOCOL) == EXTDATA_USB_PROTOCOL_WACOM_INTUOS5)
		{
			dummy_intuos5_touch_filler_countdown_5ms = 250;
			dummy_report192_intuos5_countdown_5ms = 1000;
		}

		filler_countdown_5ms = 0;
	}

	void timer_5ms_intr()
	{
		if(dummy_intuos5_touch_filler_countdown_5ms > 1)
			dummy_intuos5_touch_filler_countdown_5ms--;

		if(dummy_report192_intuos5_countdown_5ms > 1)
			dummy_report192_intuos5_countdown_5ms--;

		if(filler_countdown_5ms > 1)
			filler_countdown_5ms--;
	}

	/** sends a USB packet and will repeat that packet if nothing is being sent
	 * for a period of time */
	int8_t send_packet_fill_idle_time(const uint8_t *buffer, uint8_t size,
						uint8_t endpoint, uint8_t timeout)
	{
		filler_countdown_5ms = extdata_getValue16(EXTDATA_IDLE_TIME_LIMIT_MS) / 5;

		// crop to USB_MAX_PACKET_SIZE
		if(size > USB_MAX_PACKET_SIZE)
		{
			size = USB_MAX_PACKET_SIZE;
			console::printlnP(STR_USBUTIL_PACKET_SIZE_TOO_LARGE);
		}

		memcpy(repeat_buffer, buffer, size);
		repeat_endpoint = endpoint;
		repeat_timeout = timeout;

		return usb_send_packet(buffer, size, endpoint, timeout);
	}

	/** send a USB packet. Use this call to properly manage the idle time filler. */
	int8_t send_packet(const uint8_t *buffer, uint8_t size,
				uint8_t endpoint, uint8_t timeout)
	{
		filler_countdown_5ms = 0;

		return usb_send_packet(buffer, size, endpoint, timeout);
	}

	static uint8_t dummybuf[64];

	void usb_send_dummy_touch()
	{
		for(int i=3;i<64;i++)
			dummybuf[i] = 0;

		dummybuf[0] = 0x02;
		dummybuf[1] = 0x01;
		dummybuf[2] = 0x81;

		send_packet(dummybuf, 64, 2, 50);
	}

	void usb_send_dummy_report192_intuos5()
	{
		for(int i=1;i<9;i++)
			dummybuf[i] = 0;

		dummybuf[0] = 0xC0;
		dummybuf[9] = 0x01; // this is sometimes 0x05

		send_packet(dummybuf, 10, 3, 50);
	}

	void main_loop()
	{
		if(filler_countdown_5ms == 1)
		{
			filler_countdown_5ms = 0;

			usb_send_packet(repeat_buffer, repeat_size, repeat_endpoint, repeat_timeout);
		}

		if(dummy_report192_intuos5_countdown_5ms < 5)
		{
			dummy_report192_intuos5_countdown_5ms = 0;

			usb_send_dummy_report192_intuos5();

			dummy_report192_intuos5_countdown_5ms = 1000;
		}

		if(dummy_intuos5_touch_filler_countdown_5ms < 5)
		{
			dummy_intuos5_touch_filler_countdown_5ms = 0;

			usb_send_dummy_touch();

			dummy_intuos5_touch_filler_countdown_5ms = 200;
		}
	}
}
