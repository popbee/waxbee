/*
 * hardcoded_extdata_hid_debug.h
 *
 *  Created on: 2010-12-06
 *      Author: Bernard
 */

#ifndef HARDCODED_EXTDATA_H_
#define HARDCODED_EXTDATA_H_

#define HARDCODED_EXTDATA

#include "tables.h"
#include "usb_debug_channel.h"
#include "usb_priv.h"

#define NODATA       { 0, 0xFF00 }
#define VALUE8(val)  { val, 0xFF01 }
#define VALUE16(val) { val, 0xFF02 }


#define VENDOR_ID		0x16C0  //
#define PRODUCT_ID		0x0479  //

#define ENDPOINT0_SIZE		32

uint8_t PROGMEM device_descriptor[18] =
{
	18,					// bLength
	1,					// bDescriptorType
	LE16(0x200),				// bcdUSB  // the Graphire3 is set at 1.1  0x01,0x01 -- hope that is not a problem
	0,					// bDeviceClass
	0,					// bDeviceSubClass
	0,					// bDeviceProtocol
	ENDPOINT0_SIZE,				// bMaxPacketSize0 for endpoint 0
	LE16(VENDOR_ID),			// idVendor
	LE16(PRODUCT_ID),			// idProduct
	LE16(0x100),				// bcdDevice
	1,					// iManufacturer
	2,					// iProduct
	0,					// iSerialNumber
	1					// bNumConfigurations
};

uint8_t PROGMEM hid_report_desc[] =
{
	0x06, LE16(0xFF31),			// Usage Page 0xFF31 (vendor defined)
	0x09, 0x74,				// Usage 0x74
	0xA1, 0x53,				// Collection 0x53
	0x75, 0x08,				// report size = 8 bits
	0x15, 0x00,				// logical minimum = 0
	0x26, 0xFF, 0x00,			// logical maximum = 255
	0x95, DEBUG_TX_SIZE,			// report count
	0x09, 0x75,				// usage
	0x81, 0x02,				// Input (array)
	0xC0					// end collection
};

#define MAXPOWER_MILLIAMPS(mA) (mA/2)

#define DEBUG_TX_INTERVAL	1	// max # of ms between transmit packets

#define CONFIG_DESC_SIZE	(9+9+9+7)

uint8_t PROGMEM config_descriptor[CONFIG_DESC_SIZE] =
{
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9, 					// bLength;
	2,					// bDescriptorType;
	LE16(CONFIG_DESC_SIZE),			// wTotalLength
	1,					// bNumInterfaces
	1,					// bConfigurationValue
	0,					// iConfiguration
	0xC0,					// bmAttributes
	MAXPOWER_MILLIAMPS(100),		// bMaxPower (Graphire3 = 40mA, Teensy = ??? : Total: 100mA)
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,					// bLength
	4,					// bDescriptorType
	RAWHID_INTERFACE,			// bInterfaceNumber
	0,					// bAlternateSetting
	1,					// bNumEndpoints
	0x03,					// bInterfaceClass (0x03 = HID)
	0x00,					// bInterfaceSubClass (0x01 = Boot)
	0x00,					// bInterfaceProtocol (0x02 = Mouse)
	0,					// iInterface
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,					// bLength
	0x21,					// bDescriptorType
	LE16(0x111),				// bcdHID  // Graphire3 is using HID version 1.00 (instead of 1.11)
	0,					// bCountryCode
	1,					// bNumDescriptors
	0x22,					// bDescriptorType
	LE16(sizeof(hid_report_desc)),		// wDescriptorLength (Graphire3 is 0x0062)
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,					// bLength
	5,					// bDescriptorType
	DEBUG_TX_ENDPOINT | 0x80,		// bEndpointAddress
	0x03,					// bmAttributes (0x03=intr)
	LE16(DEBUG_TX_SIZE),			// wMaxPacketSize
	DEBUG_TX_INTERVAL			// bInterval
};

#define STR_MANUFACTURER	L"WaxBee"
#define STR_PRODUCT		L"WaxBee Debugger"

struct usb_string_descriptor_struct
{
	const uint8_t bLength;
	const uint8_t bDescriptorType;
	const int16_t wString[];
};

const struct usb_string_descriptor_struct PROGMEM string1 =
{
	sizeof(STR_MANUFACTURER),
	3,
	STR_MANUFACTURER
};

const struct usb_string_descriptor_struct PROGMEM dummystr =
{
	4,
	3,
	L"*"
};

const struct usb_string_descriptor_struct PROGMEM string2 =
{
	sizeof(STR_PRODUCT),
	3,
	STR_PRODUCT
};


struct ExtData PROGMEM hardcoded_extdata_table[] =
{
	{ (uint16_t)hardcoded_extdata_table, sizeof(struct ExtData)*EXTDATA_COUNT }, // EXTDATA_MAINTABLE

	VALUE8(EXTDATA_USB_PORT_DEBUG),			// EXTDATA_USB_PORT
	VALUE8(EXTDATA_SERIAL_PORT_DISABLED),		// EXTDATA_SERIAL_PORT
	VALUE8(EXTDATA_ADB_PORT_SLAVE_DIGITIZER),	// EXTDATA_ADB_PORT

	{(uint16_t)device_descriptor, sizeof(device_descriptor)},	// EXTDATA_USB_DEVICEDESC
	{(uint16_t)config_descriptor, sizeof(config_descriptor)},	// EXTDATA_USB_CONFIGDESC
	{(uint16_t)hid_report_desc, sizeof(hid_report_desc)},		// EXTDATA_USB_HIDREPORTDESC

	{(uint16_t)&string1, sizeof(STR_MANUFACTURER) },		// EXTDATA_USB_STRING1
	{(uint16_t)&string2, sizeof(STR_PRODUCT) },			// EXTDATA_USB_STRING2
	{(uint16_t)&dummystr, 4 },					// EXTDATA_USB_STRING3
	{(uint16_t)&dummystr, 4 },					// EXTDATA_USB_STRING4
	{(uint16_t)&dummystr, 4 },					// EXTDATA_USB_STRING5

	VALUE8(0),							// EXTDATA_USB_PROTOCOL

	VALUE8(ENDPOINT0_SIZE),						// EXTDATA_USB_ENDPOINT0SIZE
	VALUE8(0),							// EXTDATA_USB_ENDPOINT1SIZE
	VALUE8(DEBUG_TX_SIZE),						// EXTDATA_USB_ENDPOINT3SIZE

	VALUE16(0),		// EXTDATA_USB_X_MIN
	VALUE16(0),		// EXTDATA_USB_Y_MIN
	VALUE16(16704),		// EXTDATA_USB_X_MAX
	VALUE16(12064),		// EXTDATA_USB_Y_MAX
	VALUE16(0),		// EXTDATA_USB_X_ANCHOR_MIN
	VALUE16(0),		// EXTDATA_USB_Y_ANCHOR_MIN
	VALUE16(0),		// EXTDATA_USB_X_ANCHOR_MAX
	VALUE16(0),		// EXTDATA_USB_Y_ANCHOR_MAX
	VALUE16(511),		// EXTDATA_USB_PRESSURE_MAX

	VALUE8(EXTDATA_SLAVE_PROTOCOL_WACOM_ADB),	// EXTDATA_SLAVE_PROTOCOL
	VALUE8(EXTDATA_SLAVE_ORIENTATION_NORMAL),		// EXTDATA_SLAVE_ORIENTATION

	VALUE16(0),		// EXTDATA_SLAVE_X_MIN
	VALUE16(0),		// EXTDATA_SLAVE_Y_MIN
	VALUE16(20480),		// EXTDATA_SLAVE_X_MAX
	VALUE16(15360),		// EXTDATA_SLAVE_Y_MAX
	VALUE16(0),		// EXTDATA_SLAVE_ANCHOR_X_MIN
	VALUE16(0),		// EXTDATA_SLAVE_ANCHOR_Y_MIN
	VALUE16(0),		// EXTDATA_SLAVE_ANCHOR_X_MAX
	VALUE16(0),		// EXTDATA_SLAVE_ANCHOR_Y_MAX
	VALUE16(255),		// EXTDATA_SLAVE_PRESSURE_MAX

	VALUE8(EXTDATA_CPU_CORE_CLOCK_F_16MHZ)		// EXTDATA_CPU_CORE_CLOCK
};

#endif /* HARDCODED_EXTDATA_H_ */
