/*
 * console.h
 *
 *  Created on: 2010-11-11
 *      Author: Bernard
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "featureinclusion.h"

#include <stdint.h>
#include <avr/pgmspace.h>

#define CONSOLE_PRINTP(x)	console::printP(x)
#define CONSOLE_PRINTLNP(x)	console::printlnP(x)

namespace console
{
	extern bool console_enabled;

	void init();

	void print(const char* str);

	/** @param progmem_str address of string stored in 'program memory'. */
	void printP(const prog_char* progmem_str);
	/** @param progmem_str address of string stored in 'program memory'. */
	void printlnP(const prog_char* progmem_str);

	void println();
	void println(const char* str);
	void print(const char c);
	void printbit(bool bit);
	void printNumber(long number);
	void printBinaryByte(uint8_t number);
	void printHex(uint32_t number, uint8_t hexdigits);
	void flush();
}

#endif /* CONSOLE_H_ */
