/* Simple example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2008 PJRC.COM, LLC
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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <util/delay.h>
#include "usb_serial.h"
#include "serial.h"
#include "led.h"

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))

void send_str(const char *s);
uint8_t recv_str(char *buf, uint8_t size);
void parse_and_execute_command(const char *buf, uint8_t num);

#if 0
// Very simple character echo test
int main(void)
{
	CPU_PRESCALE(0);
	usb_init();
	while (1) {
		int n = usb_serial_getchar();
		if (n >= 0) usb_serial_putchar(n);
	}
}

#else

uint16_t pulseCount = 0;

void startLedPulse()
{
	LED_ON;
	pulseCount = 1000;
}

void onSerialByteReceived(uint8_t data)
{
	startLedPulse();
	usb_serial_putchar(data);
}

// Basic command interpreter for controlling port pins
int main(void)
{
	// set for 16 MHz clock, and turn on the LED
	CPU_PRESCALE(0);
	LED_CONFIG;
	LED_ON;

	// initialize the USB, and then wait for the host
	// to set configuration.  If the Teensy is powered
	// without a PC connected to the USB port, this 
	// will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;
	_delay_ms(1000);
	LED_OFF;

	serial::init(onSerialByteReceived);

	uint32_t lastbaud = 0;

	while (1)
	{
		if(pulseCount > 0)
		{
			pulseCount--;

			if(pulseCount == 0)
				LED_OFF;
		}

		// wait for the user to run their terminal emulator program
		// which sets DTR to indicate it is ready to receive.
//		while (!(usb_serial_get_control() & USB_SERIAL_DTR))
//		{
//			LED_TOGGLE;
//		}
//		LED_OFF;

		uint32_t baud = usb_serial_get_baud();
		if(baud != lastbaud)
		{
			serial::setBaudRate(baud);
			if(baud == 9600)
			{
				usb_serial_putchar('9');
				usb_serial_putchar('6');
				usb_serial_putchar('0');
				usb_serial_putchar('0');
				usb_serial_putchar('\n');
			}
			else if(baud == 19200)
			{
				usb_serial_putchar('1');
				usb_serial_putchar('9');
				usb_serial_putchar('2');
				usb_serial_putchar('0');
				usb_serial_putchar('0');
				usb_serial_putchar('\n');
			}
			else if(baud == 38400)
			{
				usb_serial_putchar('3');
				usb_serial_putchar('8');
				usb_serial_putchar('4');
				usb_serial_putchar('0');
				usb_serial_putchar('0');
				usb_serial_putchar('\n');
			}
			else
			{
				usb_serial_putchar('B');
				usb_serial_putchar('a');
				usb_serial_putchar('u');
				usb_serial_putchar('d');
				usb_serial_putchar('?');
				usb_serial_putchar('\n');
			}

			lastbaud = baud;
		}

     		serial::serialPortProcessing();

       		int16_t r = usb_serial_getchar();

       		if(r != -1)
       		{
       			startLedPulse();
       			serial::sendByte((uint8_t)(r & 0xFF));
       		}
	}
}
#endif


// Receive a string from the USB serial port.  The string is stored
// in the buffer and this function will not exceed the buffer size.
// A carriage return or newline completes the string, and is not
// stored into the buffer.
// The return value is the number of characters received, or 255 if
// the virtual serial connection was closed while waiting.
//
uint8_t recv_str(char *buf, uint8_t size)
{
	int16_t r;
	uint8_t count=0;

	while (count < size) {
		r = usb_serial_getchar();
		if (r != -1) {
			if (r == '\r' || r == '\n') return count;
			if (r >= ' ' && r <= '~') {
				*buf++ = r;
				usb_serial_putchar(r);
				count++;
			}
		} else {
			if (!usb_configured() ||
			  !(usb_serial_get_control() & USB_SERIAL_DTR)) {
				// user no longer connected
				return 255;
			}
			// just a normal timeout, keep waiting
		}
	}
	return count;
}



