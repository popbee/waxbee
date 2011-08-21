#ifndef USB_UTIL_H_
#define USB_UTIL_H_

/*
 * usb_util.h
 *
 *      Author: Bernard
 *      Date: 2011/08/10
 */

#include <stdint.h>

#define USB_MAX_PACKET_SIZE 40

namespace UsbUtil
{
	void init();	// initialize

	/** 5ms interrupt hook */
	void timer_5ms_intr();

	/** main loop hook */
	void main_loop();

	/** sends a USB packet and will repeat that packet if nothing is being sent
	 * for a period of time */
	int8_t send_packet_fill_idle_time(const uint8_t *buffer, uint8_t size,
						uint8_t endpoint, uint8_t timeout);

	/** send a USB packet. Use this call to properly manage the idle time filler. */
	int8_t send_packet(const uint8_t *buffer, uint8_t size,
				uint8_t endpoint, uint8_t timeout);
}

#endif /* USB_UTIL_H_ */
