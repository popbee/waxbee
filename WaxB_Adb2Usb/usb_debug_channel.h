/*
 * usb_debug_channel.h
 *
 *  Created on: 2010-11-14
 *      Author: Bernard
 */

#ifndef USB_DEBUG_CHANNEL_H_
#define USB_DEBUG_CHANNEL_H_

#ifdef __cplusplus
namespace usb_debug
{
	int8_t putchar(uint8_t c);
	void flush_output(void);
	void start_of_frame_interrupt();
}
#endif

#define DEBUG_INTERFACE		1
#define DEBUG_TX_ENDPOINT	3
#define DEBUG_TX_SIZE		32
#define DEBUG_TX_BUFFER		EP_DOUBLE_BUFFER

#endif /* USB_DEBUG_CHANNEL_H_ */
