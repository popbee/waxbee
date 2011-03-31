/*
 * tables.c
 *
 *  Created on: 2010-11-06
 *      Author: Bernard
 */

#include "tables.h"
#include "usb_priv.h"
#include "usb_debug_channel.h"

// the string 0 is not a string but a 16 bit list of supported locale codes
uint8_t PROGMEM string0[4] = { 4, 3, 0x04,0x09 };

