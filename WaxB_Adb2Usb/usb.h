#ifndef USB_H_
#define USB_H_

/*
 * usb.h
 *
 *      Author: Bernard
 */

#include <stdint.h>

void usb_init(void);		// initialize everything
bool usb_configured(void);	// is the USB port configured

int8_t usb_rawhid_send(const uint8_t *buffer, uint8_t size, uint8_t endpoint, uint8_t timeout);

#endif /* USB_H_ */
