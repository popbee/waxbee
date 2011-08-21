/*
 * console.cpp
 *
 * Output to a debugging console
 *
 *  Created on: 2010-11-11
 *      Author: Bernard
 */

#include <avr/pgmspace.h>

#include "extdata.h"
#include "console.h"
#include "usb_debug_channel.h"

namespace console
{
	bool console_enabled = false;
	bool usb_output;
	bool serial_output;

	void init()
	{
		usb_output = (extdata_getValue8(EXTDATA_USB_PORT) == EXTDATA_USB_PORT_DEBUG);
		serial_output = (extdata_getValue8(EXTDATA_SERIAL_PORT) == EXTDATA_SERIAL_PORT_DEBUG);

		if(usb_output || serial_output)
			console_enabled = true;
	}

	static void do_print(char c)
	{
		if(usb_output)
			usb_debug::putchar(c);
	}

	/** @param progmem_str address of string stored in 'program memory'. */
	void printP(prog_char* progmem_str)
	{
		if(!console_enabled)
			return;

		uint8_t c = pgm_read_byte(progmem_str);

		while(c!=0)
		{
			do_print(c);

			progmem_str++;
			c = pgm_read_byte(progmem_str);
		}
	}

	/** @param progmem_str address of string stored in 'program memory'. */
	void printlnP(prog_char* progmem_str)
	{
		printP(progmem_str);
		println();
	}

	void print(const char* str)
	{
		if(!console_enabled)
			return;

		char c = *str;
		while (c != 0)
		{
			do_print(c);
			str++;
			c = *str;
		}
	}

	void println(const char* str)
	{
		print(str);
		println();
	}

	void print(char c)
	{
		if(!console_enabled)
			return;

		do_print(c);
	}

	void println()
	{
		print('\n');
	}

	void printbit(bool bit)
	{
		print(bit?'1':'0');
	}

	void printBinaryByte(uint8_t number)
	{
		if(!console_enabled)
			return;

		for(int i=0;i<8;i++)
		{
			if(number & 0x80)
				do_print('1');
			else
				do_print('0');

			number <<= 1;
		}
	}

	void printNumber(long number)
	{
		if(!console_enabled)
			return;

		if (number == 0)
		{
			do_print('0');
			return;
		}

		if(number < 0)
		{
			do_print('-');
			number = -number;
		}

		unsigned char buf[10];
		unsigned long i = sizeof(buf);

		while (number > 0)
		{
			buf[--i] = number % 10;
			number /= 10;
		}

		while (i < sizeof(buf))
		{
			do_print((char)('0' + buf[i++]));
		}
	}

	static char hexDigit(uint8_t value)
	{
		if(value >= 10)
			return ('A'-10)+value;

		return '0'+value;
	}

	void printHex(uint16_t number, uint8_t hexdigits)
	{
		if(!console_enabled)
			return;

		if(hexdigits >= 4)
			do_print(hexDigit((number>>12) & 0xf));

		if(hexdigits >= 3)
			do_print(hexDigit((number>>8) & 0xf));

		if(hexdigits >= 2)
			do_print(hexDigit((number>>4) & 0xf));

		do_print(hexDigit(number & 0xf));
	}

	void flush()
	{
		if(!console_enabled)
			return;

		if(usb_output)
			usb_debug::flush_output();
	}
}
