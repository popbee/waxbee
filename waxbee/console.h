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

#ifdef DEBUG_SUPPORT
#define CONSOLE_PRINTP(x)	console::printP(x)
#define CONSOLE_PRINTLNP(x)	console::printlnP(x)
#else
#define CONSOLE_PRINTP(x)
#define CONSOLE_PRINTLNP(x)
#endif

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
	void printNumberFixedLength(long number, uint8_t len);
	void printBinaryByte(uint8_t number);
	void printHex(uint32_t number, uint8_t hexdigits);
	void flush();
	void printBinary(uint8_t d, int8_t startbit, int8_t endbit);
}

#endif /* CONSOLE_H_ */
