// USB layer

//
// This file was heavily inspired by the work of PJRC.COM, LLC, copyright notice below:
//
// Copyright (c) Bernard Poulin (bernard-at-acm-dot-org) WaxBee.org

/* 
 * Copyright (c) 2009 PJRC.COM, LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "usb.h"
#include "usb_priv.h"
#include "led.h"
#include "strings.h"
#include "extdata.h"
#include "usb_debug_channel.h"

#define RAWHID_INTERFACE	0
#define RAWHID_RX_SIZE		8	// receive packet size

#if defined(__AVR_AT90USB162__)
#define RAWHID_TX_BUFFER	EP_SINGLE_BUFFER
#define RAWHID_RX_BUFFER	EP_SINGLE_BUFFER
#else
#define RAWHID_TX_BUFFER	EP_DOUBLE_BUFFER
#define RAWHID_RX_BUFFER	EP_DOUBLE_BUFFER
#endif


// zero when we are not configured, non-zero when enumerated
volatile uint8_t usb_configuration = 0;

// these are a more reliable timeout than polling the
// frame counter (UDFNUML)
static volatile uint8_t rx_timeout_count = 0;
static volatile uint8_t tx_timeout_count = 0;

// Misc functions to wait for ready and send/receive packets
static inline void usb_wait_in_ready(void)
{
	while (!(UEINTX & (1 << TXINI)))
		;
}
/** return false if aborted */
static inline bool usb_wait_in_ready_or_abort()
{
	uint8_t i;

	// wait for host ready for IN packet
	do
	{
		i = UEINTX;
	} while (!(i & ((1 << TXINI) | (1 << RXOUTI))));

	if (i & (1 << RXOUTI))
		return false;

	return true;
}

static inline void usb_send_in(void)
{
	UEINTX = ~(1 << TXINI);
}
static inline void usb_wait_receive_out(void)
{
	while (!(UEINTX & (1 << RXOUTI)))
		;
}
static inline void usb_ack_out(void)
{
	UEINTX = ~(1 << RXOUTI);
}

static uint8_t endpointSize[5];

// initialize USB
void usb_init(void)
{
	HW_CONFIG();
	USB_FREEZE(); // enable USB
	PLL_CONFIG(); // config PLL
	while (!(PLLCSR & (1 << PLOCK)))
		; // wait for PLL lock
	USB_CONFIG(); // start USB clock
	//	UDCON = (1<<LSM);			// enable attach resistor - Low Speed USB
	UDCON = 0; // enable attach resistor - Full Speed USB
	usb_configuration = 0;

	endpointSize[0] = extdata_getValue8(EXTDATA_USB_ENDPOINT0SIZE);
	endpointSize[1] = extdata_getValue8(EXTDATA_USB_ENDPOINT1SIZE);
	endpointSize[2] = extdata_getValue8(EXTDATA_USB_ENDPOINT2SIZE);
	endpointSize[3] = extdata_getValue8(EXTDATA_USB_ENDPOINT3SIZE);

	// enable End-Of-Reset and Start-Of-Frame interrupts
	UDIEN = (1<<EORSTE)|(1<<SOFE);
}

// return 0 if the USB is not configured, or the configuration
// number selected by the HOST
bool usb_configured(void)
{
	return usb_configuration;
}

#if (GCC_VERSION >= 40300) && (GCC_VERSION < 40302)
#error "Buggy GCC 4.3.0 compiler, please upgrade!"
#endif

/** send a USB packet, with timeout
 *
 * Note: size cannot be larger than the endpoint configured size (data will be truncated if so).
 */
int8_t usb_send_packet(const uint8_t *buffer, uint8_t size, uint8_t endpoint, uint8_t timeout)
{
	uint8_t intr_state;

	if(size > endpointSize[endpoint])
	{
		// truncate data if too long
		size = endpointSize[endpoint];
	}

	// if we're not online (enumerated and configured), error
	if (!usb_configuration)
		return -1;
	intr_state = SREG;
	cli();
	tx_timeout_count = timeout;
	UENUM = endpoint;
	// wait for the FIFO to be ready to accept data
	while (1)
	{
		if (UEINTX & (1 << RWAL))
			break;
		SREG = intr_state;
		if (tx_timeout_count == 0)
			return 0;
		if (!usb_configuration)
			return -1;
		intr_state = SREG;
		cli();
		UENUM = endpoint;
	}

	uint8_t i = size;
	while(i--)
		UEDATX = *buffer++;

	// transmit it now
//	cbi(UEINTX, TXINI);
//	cbi(UEINTX, FIFOCON);
	UEINTX = 0x3A;
	SREG = intr_state;
	return size;
}

// USB General/Device Interrupt - handle all device-level events
// the transmit buffer flushing is triggered by the start of frame
//
ISR(USB_GEN_vect)
{
	//===================================
	// USB Device Interrupts
	//===================================

	uint8_t intbits = UDINT;

	// clear all USB Device interrupt flags
	UDINT = 0;

	if (intbits & (1 << EORSTI))
	{
		//---------------------------
		// End-Of-Reset Interrupt
		//---------------------------

		//---------------------------
		// Enable Endpoint 0
		//---------------------------
		UENUM = 0;
		UECONX = 1;
		UECFG0X = EP_TYPE_CONTROL;
		UECFG1X = EP_SIZE(endpointSize[0]) | EP_SINGLE_BUFFER;
		UEIENX = (1 << RXSTPE);

		usb_configuration = 0;
	}

	if ((intbits & (1 << SOFI)) && usb_configuration)
	{
		//---------------------------
		// Start-Of-Frame Interrupt
		//---------------------------

		uint8_t t = rx_timeout_count;
		if (t)
			rx_timeout_count = --t;
		t = tx_timeout_count;
		if (t)
			tx_timeout_count = --t;

		usb_debug::start_of_frame_interrupt();
	}
}


struct SetupPacket
{
	uint8_t bmRequestType;
	uint8_t bRequest;
	union
	{
		uint16_t wValue;
		struct  // little endian
		{
			uint8_t value_low;
			uint8_t value_high;
		};
	};
	union
	{
		uint16_t wIndex;
		struct // little endian
		{
			uint8_t index_low;
			uint8_t index_high;
		};
	};
	union
	{
		uint16_t wLength;
		struct // little endian
		{
			uint8_t length_low;
			uint8_t length_high;
		};
	};
};

static SetupPacket req;

#define USBREQ_STALL	false
#define USBREQ_SUCCESS	true
#define USBREQ_ABORT	true

// note: called within an interrupt
static inline bool handleStandardEndpoint0()
{
	switch (req.bRequest)
	{
		case GET_DESCRIPTOR:
		{
			uint16_t desc_addr;
			uint8_t desc_length;

			switch(req.value_high)
			{
				case 0x01:  // ----- DEVICE DESCRIPTOR -----
					if(req.value_low != 0) return USBREQ_STALL;

					desc_addr = extdata_getAddress(EXTDATA_USB_DEVICEDESC);
					desc_length = extdata_getLength8(EXTDATA_USB_DEVICEDESC);
					break;
				case 0x02:  // ----- CONFIG DESCRIPTOR -----
					if(req.value_low != 0) return USBREQ_STALL;

					desc_addr = extdata_getAddress(EXTDATA_USB_CONFIGDESC);
					desc_length = extdata_getLength8(EXTDATA_USB_CONFIGDESC);
					break;
				case 0x21:  // ----- HID INTERFACE DESCRIPTOR -----
					//if(req.value_low != 0) return USBREQ_STALL;
					// no support of this for now (never got a query for it)
					// (already returned part of the config descriptor)
					return USBREQ_STALL;

				case 0x22:  // ----- HID REPORT DESCRIPTOR -----
					if(req.value_low != 0) return USBREQ_STALL;

					if(req.index_low == 0)
					{
						desc_addr = extdata_getAddress(EXTDATA_USB_HIDREPORTDESC);
						desc_length = extdata_getLength8(EXTDATA_USB_HIDREPORTDESC);
					}
					else if(req.index_low == 1)
					{
						desc_addr = extdata_getAddress(EXTDATA_USB_HIDREPORTDESC2);
						desc_length = extdata_getLength8(EXTDATA_USB_HIDREPORTDESC2);
					}
					else
					{
						return USBREQ_STALL;
					}

					break;
				case 0x03:  // ----- STRING DESCRIPTOR -----
					switch(req.value_low)
					{
						case 0:
							desc_addr = (uint16_t)usbdescriptor_string0;
							desc_length = sizeof(usbdescriptor_string0);
							break;
						case 1:
							desc_addr = extdata_getAddress(EXTDATA_USB_STRING1);
							desc_length = extdata_getLength8(EXTDATA_USB_STRING1);
							break;
						case 2:
							desc_addr = extdata_getAddress(EXTDATA_USB_STRING2);
							desc_length = extdata_getLength8(EXTDATA_USB_STRING2);
							break;
						case 3:
							desc_addr = extdata_getAddress(EXTDATA_USB_STRING3);
							desc_length = extdata_getLength8(EXTDATA_USB_STRING3);
							break;
						case 4:
							desc_addr = extdata_getAddress(EXTDATA_USB_STRING4);
							desc_length = extdata_getLength8(EXTDATA_USB_STRING4);
							break;
						case 5:
							desc_addr = extdata_getAddress(EXTDATA_USB_STRING5);
							desc_length = extdata_getLength8(EXTDATA_USB_STRING5);
							break;
						default:
							return USBREQ_STALL;
					}
					break;
				default:
					return USBREQ_STALL;
			}

			if(desc_length == 0)
				return USBREQ_STALL;

			uint8_t len = (req.wLength < 256) ? req.wLength : 255;
			if (len > desc_length)
				len = desc_length;

			uint8_t n,i;

			do
			{
				// wait for host ready for IN packet
				do
				{
					i = UEINTX;
				} while (!(i & ((1 << TXINI) | (1 << RXOUTI))));

				if (i & (1 << RXOUTI))
					return USBREQ_ABORT;

				// send IN packet
				if(len < endpointSize[0])
					n = len;
				else
					n = endpointSize[0];

				for (i = n; i; i--)
				{
					UEDATX = pgm_read_byte(desc_addr++);
				}

				len -= n;
				usb_send_in();
			} while (len || n == endpointSize[0]);

			return USBREQ_SUCCESS;
		}
		case SET_ADDRESS:
		{
			usb_send_in();
			usb_wait_in_ready();
			UDADDR = req.wValue | (1 << ADDEN);
			return USBREQ_SUCCESS;
		}
		case SET_CONFIGURATION:
		{
			if (req.bmRequestType != 0)
				return USBREQ_STALL;

			usb_configuration = req.wValue;
			usb_send_in();

			// Time to activate all the "other" endpoints ( > 0 ) in the USB controller

			if(endpointSize[1] > 0)
			{
				//--------------------
				// Endpoint 1
				//--------------------
				UENUM = 1;
				UECONX = 1;
				UECFG0X = EP_TYPE_INTERRUPT_IN;
				UECFG1X = EP_SIZE(endpointSize[1]) | RAWHID_TX_BUFFER;
				//--------------------
			}

			if(endpointSize[2] > 0)
			{
				//--------------------
				// Endpoint 2
				//--------------------
				UENUM = 2;
				UECONX = 1;
				UECFG0X = EP_TYPE_INTERRUPT_IN;
				UECFG1X = EP_SIZE(endpointSize[2]) | RAWHID_TX_BUFFER;
				//--------------------
			}

			if(endpointSize[3] > 0)
			{
				//--------------------
				// Endpoint 3
				//--------------------
				UENUM = 3;
				UECONX = 1;
				UECFG0X = EP_TYPE_INTERRUPT_IN;
				UECFG1X = EP_SIZE(endpointSize[3]) | DEBUG_TX_BUFFER;
				//--------------------
			}

			UERST = 0x1E;
			UERST = 0;
			return USBREQ_SUCCESS;
		}
		case GET_CONFIGURATION:
		{
			if (req.bmRequestType != 0x80)
				return USBREQ_STALL;

			usb_wait_in_ready();
			UEDATX = usb_configuration;
			usb_send_in();
			return USBREQ_SUCCESS;
		}
		case GET_STATUS:
		{
			usb_wait_in_ready();
			uint8_t i = 0;

			if (req.bmRequestType == 0x82)
			{
				UENUM = req.wIndex;
				if (UECONX & (1 << STALLRQ))
					i = 1;
				UENUM = 0;
			}

			UEDATX = i;
			UEDATX = 0;
			usb_send_in();
			return USBREQ_SUCCESS;
		}
		case CLEAR_FEATURE:
		case SET_FEATURE:
		{
			if (req.bmRequestType != 0x02 || req.wValue != 0)
				return USBREQ_STALL;

			uint8_t i = req.wIndex & 0x7F;
			if (i >= 1 && i <= MAX_ENDPOINT)
			{
				usb_send_in();
				UENUM = i;

				if (req.bRequest == SET_FEATURE)
					return USBREQ_STALL;

				UECONX = (1 << STALLRQC) | (1 << RSTDT)	| (1 << EPEN);
				UERST = (1 << i);
				UERST = 0;

				return USBREQ_SUCCESS;
			}
		}
	} //switch

	return USBREQ_STALL;
}

// note: called within an interrupt
static inline bool handleHidEndpoint0()
{
	if (req.wIndex != RAWHID_INTERFACE)
		return USBREQ_STALL;

	switch (req.bRequest)
	{
		case HID_GET_REPORT:
		{
			// wait for host ready for IN packet
			if(!usb_wait_in_ready_or_abort())
				return USBREQ_ABORT;

			switch(req.value_low) // Report ID
			{
				case 3: // GetReport(3)
				{
					// Not sure what is this about.
					// Graphire3, Intuos2 and BambooFun returns 0x03 00
					UEDATX = 0x03;
					UEDATX = 0x00;

					usb_send_in();
					break;
				}
				case 5: // GetReport(5)
				{
					// Not sure what is this about.
					// Intuos2 appears to return 0x05 80 00 00 00 00 00 00 00
					UEDATX = 0x05;
					UEDATX = 0x80;
					UEDATX = 0x00;
					UEDATX = 0x00;
					UEDATX = 0x00;
					UEDATX = 0x00;
					UEDATX = 0x00;
					UEDATX = 0x00;

					// send after 8 first bytes (endpoint0 buffer size)
					usb_send_in();

					// wait for host ready for (next) IN packet
					if(!usb_wait_in_ready_or_abort())
						return USBREQ_ABORT;

					UEDATX = 0x00;

					usb_send_in();
					break;
				}
				case 6: // GetReport(6)
				{
					// Not sure what is this about.
					// Intuos2 appears to return 0x06 81 00 00 00 00 00 00 00
					UEDATX = 0x06;
					UEDATX = 0x81;
					UEDATX = 0x00;
					UEDATX = 0x00;
					UEDATX = 0x00;
					UEDATX = 0x00;
					UEDATX = 0x00;
					UEDATX = 0x00;

					// send after 8 first bytes (endpoint0 buffer size)
					usb_send_in();

					// wait for host ready for (next) IN packet
					if(!usb_wait_in_ready_or_abort())
						return USBREQ_ABORT;

					UEDATX = 0x00;

					usb_send_in();
					break;
				}
			}

			return USBREQ_SUCCESS;
		}
		case HID_SET_REPORT:
		{
			uint8_t len = RAWHID_RX_SIZE;
			do
			{
				uint8_t n = len < endpointSize[0] ? len
						: endpointSize[0];
				usb_wait_receive_out();

				switch(req.value_low)
				{
					case 2:
					{
						// The Graphire3, Intuos2 and Bamboo Wacom driver sends 0x02 0x02 here.

						// I think this to "switch" to the digitizer protocol (Report ID 0x02)
						// (by default, the Graphire3 sends standard HID Mouse packets)
						break;
					}
					case 4:
					{
						// Intuos2 driver sends 0x04 0x01 when initializing, dunno what this is about.
						// Some feature I suppose.
						break;
					}
					case 9:
					{
						// Intuos2 driver sends 0x09 0x00 when initializing, dunno what this is about.
						// Some feature I suppose.
						break;
					}
					case 21:
					{
						// Bamboo driver sends 0x21 0x01 when initializing, dunno what this is about.
						// Some feature I suppose.
						break;
					}
	
				}
				// ignore incoming bytes

				usb_ack_out();
				len -= n;
			} while (len);

			usb_wait_in_ready();
			usb_send_in();
			return USBREQ_SUCCESS;
		}
	} //switch

	return USBREQ_STALL;
}

// handle endpoint 0
//
// return USBREQ_STALL on error to force a STALL
//
// return USBREQ_SUCCESS on success
// return USBREQ_ABORT for "no stall aborts errors"
//
// note: called within an interrupt
static inline bool handleEnpoint0()
{
	// gather the setup packet data
	req.bmRequestType = UEDATX;

	req.bRequest = UEDATX;

	req.value_low = UEDATX;
	req.value_high = UEDATX;

	req.index_low = UEDATX;
	req.index_high = UEDATX;

	req.length_low = UEDATX;
	req.length_high = UEDATX;

	UEINTX = ~((1 << RXSTPI) | (1 << RXOUTI) | (1 << TXINI));

	switch (req.bmRequestType)
	{
		default:
			return handleStandardEndpoint0();
		case USB_REQUESTTYPE_HID_H2D:
		case USB_REQUESTTYPE_HID_D2H:
			return handleHidEndpoint0();
	}
}

// USB Endpoint Interrupt - endpoint 0 is handled here.  The
// other endpoints are manipulated by the user-callable
// functions, and the start-of-frame interrupt.
//
ISR(USB_COM_vect)
{
//	sbi(PORTC,6);

	UENUM = 0;
	uint8_t intbits = UEINTX;

	if (intbits & (1 << RXSTPI))
	{
		if (handleEnpoint0())
			return;
	}

	// Stall USB line
	UECONX = (1 << STALLRQ) | (1 << EPEN);

//	cbi(PORTC,6);
}

