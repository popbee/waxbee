/*
 * strings.c
 *
 *  Created on: 2010-11-06
 *      Author: Bernard
 */

#include "featureinclusion.h"

#include <stdint.h>
#include <avr/pgmspace.h>

#define STRINGS_C_  // "concretize" the strings

#include "strings.h"

// the string 0 is not a string but a 16 bit list of supported locale codes
const uint8_t PROGMEM usbdescriptor_string0[4] = { 4, 3, 0x04,0x09 };

