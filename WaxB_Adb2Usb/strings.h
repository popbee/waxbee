/*
 * strings.h
 *
 * Strings stored in program memory.
 *
 * NOTE: This file exists because in GNU CPP (G++) -- unlike in GNU C -- it seems impossible to store
 * initialized objects in program memory. (using techniques like the PSTR() macro).
 *
 *  Created on: 2010-08-16
 *      Author: Bernard
 */

#ifndef STRINGS_H_
#define STRINGS_H_

#include <stdint.h>
#include <avr/pgmspace.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern uint8_t PROGMEM usbdescriptor_string0[4];

// This header file will morph into its associated ".c" file if "STRINGS_C_" is #defined.
// In other words, it will "create" the strings only in strings.c
// strings.c is the only place that STRINGS_C_ is defined.

#ifdef STRINGS_C_
#define PSTRING(symbol, text) prog_char symbol[] = (text)
#else
#define PSTRING(symbol, text) extern prog_char symbol[]
#endif

//----------------------------------------------------------------
// waxbee_main.cpp
//----------------------------------------------------------------

PSTRING(STR_WAXBEE_WELCOME, "WaxBee 0.15a -- bzz! bzz!\nSerial/ADB tablet to USB converter.");

//----------------------------------------------------------------
// adb_controller.cpp
//----------------------------------------------------------------

PSTRING(STR_ENTERING_PROXIMITY_MODE, "Entering proximity. Mode=");
PSTRING(STR_EXITING_PROXIMITY, "Exiting proximity.");
PSTRING(STR_ERROR_CODE, "ADB Error code=");
PSTRING(STR_POWERON, "-- poweron");
PSTRING(STR_IDENTIFY, "-- identify");
PSTRING(STR_ADB_TABLET_INFO_MAX_X, "ADB Tablet Info: max_x=");
PSTRING(STR_MAX_Y, ", max_y=");
PSTRING(STR_4L3, "-- _4L3");
PSTRING(STR_4L3_RESPONSE, "-- _4L3_response");
PSTRING(STR_4T3, "-- _4T3");
PSTRING(STR_4T3_RESPONSE, "-- _4T3_response");
PSTRING(STR_4L1, "-- _4L1");
PSTRING(STR_4L1_RESPONSE, "-- _4L1_response");
PSTRING(STR_4T1_CHECK, "-- _4T1_check");
PSTRING(STR_4T1_CHECK_RESPONSE, "-- _4T1_check_response");
PSTRING(STR_IDENTIFY_RESPONSE, "-- identify_response");

//----------------------------------------------------------------
// adb_codec.cpp
//----------------------------------------------------------------

PSTRING(STR_STOP2START_TIME_TOO_SMALL, "stop2start time too small: ");

//----------------------------------------------------------------
// button_event.cpp
//----------------------------------------------------------------

PSTRING(STR_BUTTON_NO, "Button #");
PSTRING(STR_IN, "in");
PSTRING(STR_HOVERING, "hovering");
PSTRING(STR_DOWN, "down");
PSTRING(STR_HELD_DOWN, "held_down");
PSTRING(STR_UP, "up");
PSTRING(STR_OUT, "out");

//----------------------------------------------------------------
// debugproc.cpp
//----------------------------------------------------------------

PSTRING(STR_TRIGGER, "trigger()");
PSTRING(STR_PROXOUTTRIGGER, "proxOutTrigger()");
PSTRING(STR_DEBUG_PACKET_SIZE_TOO_LARGE_MAX, "debug: packet size too large, max=");
PSTRING(STR_DEBUG_ACTIVITY_ENABLED_SECONDS, "** Debug Activity Enabled ** seconds=");
PSTRING(STR_DELAY_BEFORE_START_SECONDS, "delay_before_start: seconds=");
PSTRING(STR_ACTIVITY_DONE, "Activity done: ");
PSTRING(STR_REPEATING, "repeating...");
PSTRING(STR_STOP, "stop");

//----------------------------------------------------------------
// gpioinit.cpp
//---------------------------------------------------------------

PSTRING(STR_INVALID_PORT_COMMAND, "Invalid Port command");
PSTRING(STR_GPIO_INIT, "GPIO init: ");
PSTRING(STR_INVALID_PAUSE_COMMAND, "Invalid Pause command");
PSTRING(STR_ERROR_PARSING_INIT_STRING, "Error parsing init string, ");
PSTRING(STR_BAD_CHAR, "bad character");
PSTRING(STR_CMD_TOO_LONG_POS, "command too long (pos = ");

//----------------------------------------------------------------
// isdv4_serial.cpp
//----------------------------------------------------------------

PSTRING(STR_PRESSURE_MAX, "\nPressure Max=");
PSTRING(STR_HAS_TILT_TILT_X_MAX, ", Has Tilt: Tilt X Max=");
PSTRING(STR_TILT_Y_MAX, ", Tilt Y Max=");
PSTRING(STR_NO_TILT, ", No Tilt");
PSTRING(STR_VERSION, ", Version=");
PSTRING(STR_ISDV4_SERIAL_INIT, "isdv4_serial::init()");

//----------------------------------------------------------------
// protocol4_serial.cpp
//----------------------------------------------------------------

PSTRING(STR_SERIAL_TABLET_, "Serial Tablet - ");
PSTRING(STR_UNSUPPORTED, "Unsupported");
PSTRING(STR_ULTRAPAD, "UltraPad");
PSTRING(STR_ARTPADII, "ArtPadII");
PSTRING(STR_PENPARTNER, "PenPartner");
PSTRING(STR_ROM_VERSION, " - ROM Version ");
PSTRING(STR_ENTER_PROX, " - Enter prox =");
PSTRING(STR_ERASER, "Eraser");
PSTRING(STR_PEN, "Pen");
PSTRING(STR_EXIT_PROX, " - Exit prox");
PSTRING(STR_PROTOCOL4_SERIAL_INIT, "protocol4_serial::init()\n");

//----------------------------------------------------------------
// protocol5_serial.cpp
//----------------------------------------------------------------

PSTRING(STR_SERIAL_TABLET, "Serial Tablet - ");
PSTRING(STR_INTUOS, "Intuos");
PSTRING(STR_INTUOS2, "Intuos2");
PSTRING(STR_PROTOCOL5_SERIAL_INIT, "protocol5_serial::init()");

//----------------------------------------------------------------
// pen_button_handler.cpp
//----------------------------------------------------------------

PSTRING(STR_UNRECOGNIZED_ACTIVE_AREA_RECORD_TYPE, "Unrecognized Activate Area record type");

//----------------------------------------------------------------
// topaz_serial.cpp
//----------------------------------------------------------------

PSTRING(STR_TOPAZ_SERIAL_INIT, "topaz_serial::init()");

//----------------------------------------------------------------
// usb_util.cpp
//----------------------------------------------------------------

PSTRING(STR_USBUTIL_PACKET_SIZE_TOO_LARGE, "UsbUtil: Packet size too large");

//----------------------------------------------------------------
// wacom_usb.cpp
//----------------------------------------------------------------

PSTRING(STR_USB_PACKET_OUT_OF_RANGE_PACKET_ALL_ZEROS, "[USB Packet - Out of range packet (all zeros)]");
PSTRING(STR_USB_PACKET_IN_RANGE_PACKET_ERASER, "[USB Packet - In Range packet, eraser=");

//----------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* STRINGS_H_ */
