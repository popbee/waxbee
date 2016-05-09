/*
 * featureinclusion.h
 *
 *  Created on: 2013-05-04
 *      Author: Bernard
 */

#ifndef FEATUREINCLUSION_H_
#define FEATUREINCLUSION_H_

// define "CUSTOM_SUPPORT" when the compiling to control the feature inclusion
#ifndef CUSTOM_SUPPORT

#define DEBUG_SUPPORT
#define INITGPIO_SUPPORT
#define ADB_SUPPORT
#define SERIAL_TABLET_SUPPORT
#define TOUCH_SUPPORT
#define BUTTON_SUPPORT
#define USB_PROTOCOL_IV_SUPPORT
#define USB_PROTOCOL_V_SUPPORT

// finer-grain serial selection
#ifdef SERIAL_TABLET_SUPPORT
#define SERIAL_ISDV4_SUPPORT
#define SERIAL_PROTOCOL_IV_SUPPORT
#define SERIAL_PROTOCOL_V_SUPPORT
#define SERIAL_TOPAZ_SUPPORT
#endif

#endif

#endif /* FEATUREINCLUSION_H_ */
