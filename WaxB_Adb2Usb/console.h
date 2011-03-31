/*
 * console.h
 *
 *  Created on: 2010-11-11
 *      Author: Bernard
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <stdint.h>

namespace console
{
	extern bool console_enabled;

	void init();

	void print(const char* str);
	void println();
	void println(const char* str);
	void print(const char c);
	void printbit(bool bit);
	void printNumber(long number);
	void printBinaryByte(uint8_t number);
	void printHex(uint16_t number, uint8_t hexdigits);
	void flush();
}

#endif /* CONSOLE_H_ */
