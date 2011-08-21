/*
 * extdata.h
 *
 *  Created on: 2010-12-02
 *      Author: Bernard
 */

#ifndef EXTDATA_H_
#define EXTDATA_H_

#include <avr/pgmspace.h>
#include <stdint.h>
#include "avr_util.h"

#ifdef __cplusplus
extern "C"
{
#endif


//=================================================================================
// public methods

/* return false on error */
bool extdata_init();

uint8_t	 extdata_getValue8(uint8_t index);
uint8_t  extdata_getLength8(uint8_t index);
uint16_t extdata_getValue16(uint8_t index);
uint16_t extdata_getLength16(uint8_t index);

/** returns a 16 bit address in program memory */
uint16_t extdata_getAddress(uint8_t index);

#define PSTR2(s) (__extension__({static prog_char __c[] = (s); &__c[0];}))

void printP(prog_char str[]);

// following file is generated
#include "extdata_gen.h"

#if EXTDATA_COUNT > 63
  #error "Too many EXTDATA entries"
#endif

struct ExtData
{
	uint16_t address;
	uint16_t length;
};

#ifdef __cplusplus
}
#endif

#endif /* EXTDATA_H_ */
