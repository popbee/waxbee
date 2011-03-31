/*
 * console.cpp
 *
 * Output to a debugging console
 *
 *  Created on: 2010-11-11
 *      Author: Bernard
 */

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

	void print(const char* str)
	{
		if(!console_enabled)
			return;

		char c = *str;
		while (c != 0)
		{
			print(c);
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

		if(usb_output)
			usb_debug::putchar(c);
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
				print('1');
			else
				print('0');

			number <<= 1;
		}
	}

	void printNumber(long number)
	{
		if(!console_enabled)
			return;

		if (number == 0)
		{
			print('0');
			return;
		}

		if(number < 0)
		{
			print('-');
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
			print((char)('0' + buf[i++]));
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
			print(hexDigit((number>>12) & 0xf));

		if(hexdigits >= 3)
			print(hexDigit((number>>8) & 0xf));

		if(hexdigits >= 2)
			print(hexDigit((number>>4) & 0xf));

		print(hexDigit(number & 0xf));
	}

	void flush()
	{
		if(!console_enabled)
			return;

		if(usb_output)
			usb_debug::flush_output();
	}
}
