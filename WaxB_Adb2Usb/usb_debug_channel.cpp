/*
 * usb_debug_channel.cpp
 *
 *  Created on: 2010-11-14
 *      Author: Bernard
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usb.h"
#include "led.h"
#include "usb_debug_channel.h"

// the time remaining before we transmit any partially full
// packet, or send a zero length packet.
static volatile uint8_t debug_flush_timer = 0;

namespace usb_debug
{
	// transmit a character.  0 returned on success, -1 on error
	int8_t putchar(uint8_t c)
	{
		static uint8_t previous_timeout = 0;
		uint8_t timeout, intr_state;

		// if we're not online (enumerated and configured), error
		if (!usb_configured())
			return -1;

		// interrupts are disabled so these functions can be
		// used from the main program or interrupt context,
		// even both in the same program!
		intr_state = SREG;
		cli();
		UENUM = DEBUG_TX_ENDPOINT;

		// if we gave up due to timeout before, don't wait again
		if (previous_timeout)
		{
			if (!(UEINTX & (1 << RWAL)))
			{
				SREG = intr_state;
				return -1;
			}

			previous_timeout = 0;
		}

		// wait for the FIFO to be ready to accept data
		timeout = UDFNUML + 4;
		while (1)
		{
			// are we ready to transmit?
			if (UEINTX & (1 << RWAL))
				break;
			SREG = intr_state;
			// have we waited too long?
			if (UDFNUML == timeout)
			{
				previous_timeout = 1;
				return -1;
			}

			// has the USB gone offline?
			if (!usb_configured())
				return -1;

			// get ready to try checking again
			intr_state = SREG;
			cli();
			UENUM = DEBUG_TX_ENDPOINT;
		}

		//------------------------------
		// write the byte into the FIFO
		//------------------------------
		UEDATX = c;

		// if this completed a packet, transmit it now!
		if (!(UEINTX & (1 << RWAL)))
		{
			UEINTX = 0x3A;
			debug_flush_timer = 0;
		}
		else
		{
			debug_flush_timer = 2;
		}

		SREG = intr_state;
		return 0;
	}

	static inline void flush()
	{
		UENUM = DEBUG_TX_ENDPOINT;

		while ((UEINTX & (1 << RWAL)))
		{
			UEDATX = 0;
		}

		UEINTX = 0x3A;
	}

	// immediately transmit any buffered output.
	void flush_output(void)
	{
		uint8_t intr_state;

		intr_state = SREG;
		cli();
		if (debug_flush_timer)
		{
			flush();
			debug_flush_timer = 0;
		}
		SREG = intr_state;
	}

	// to be called within the USB_GEN interrupt (if configured, etc)
	void start_of_frame_interrupt()
	{
		uint8_t t = debug_flush_timer;
		if (t)
		{
			debug_flush_timer = --t;
			if (!t)
			{
				flush();
			}
		}
	}
}
