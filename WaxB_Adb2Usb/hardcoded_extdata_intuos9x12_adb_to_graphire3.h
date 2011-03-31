/*
 * hardcoded_extdata_intuos9x12_adb_to_graphire3.h
 *
 *  Created on: 2010-12-03
 *      Author: Bernard
 */

#ifndef HARDCODED_EXTDATA_H_
#define HARDCODED_EXTDATA_H_

#define HARDCODED_EXTDATA

#include "tables.h"
#include "usb_priv.h"

#define NODATA       { 0, 0xFF00 }
#define VALUE8(val)  { val, 0xFF01 }
#define VALUE16(val) { val, 0xFF02 }

#define VENDOR_ID		0x056A  // WACOM Co., Ltd.
#define PRODUCT_ID		0x0014  // Graphire3 6x8

#define ENDPOINT0_SIZE		8
#define RAWHID_TX_SIZE		8	// transmit packet size

uint8_t PROGMEM device_descriptor[18] =
{
	18,					// bLength
	1,					// bDescriptorType
	0x00, 0x02,				// bcdUSB  // the Graphire3 is set at 1.1  0x01,0x01 -- hope that is not a problem
	0,					// bDeviceClass
	0,					// bDeviceSubClass
	0,					// bDeviceProtocol
	ENDPOINT0_SIZE,				// bMaxPacketSize0 for endpoint 0
	LSB(VENDOR_ID), MSB(VENDOR_ID),		// idVendor
	LSB(PRODUCT_ID), MSB(PRODUCT_ID),	// idProduct
	0x14, 0x03,				// bcdDevice // "version number" of the Wacom Graphire3
	1,					// iManufacturer
	5,					// iProduct
	0,					// iSerialNumber
	1					// bNumConfigurations
};


uint8_t PROGMEM hid_report_desc[] =
{
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x01,                    //   REPORT_ID (1)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x01,                    //     INPUT (Cnst,Ary,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xc0,                          //   END_COLLECTION
    0xc0,                          // END_COLLECTION
    0x05, 0x0d,                    // USAGE_PAGE (Digitizers)
    0x09, 0x01,                    // USAGE (Digitizer)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x02,                    //   REPORT_ID (2)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x06, 0x00, 0xff,              //     USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    //     USAGE (Vendor Usage 1)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //     LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x07,                    //     REPORT_COUNT (7)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0x09, 0x01,                    //   USAGE (Vendor Usage 1)
    0x85, 0x02,                    //   REPORT_ID (2)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x01,                    //   USAGE (Vendor Usage 1)
    0x85, 0x03,                    //   REPORT_ID (3)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0xc0                           // END_COLLECTION
};

#define MAXPOWER_MILLIAMPS(mA) (mA/2)

#define RAWHID_TX_INTERVAL	1	// max # of ms between transmit packets

#define CONFIG_DESC_SIZE	(9+9+9+7)

uint8_t PROGMEM config_descriptor[CONFIG_DESC_SIZE] =
{
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9, 					// bLength;
	2,					// bDescriptorType;
	LSB(CONFIG_DESC_SIZE),			// wTotalLength
	MSB(CONFIG_DESC_SIZE),
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
	0x01,					// bInterfaceSubClass (0x01 = Boot)
	0x01,					// bInterfaceProtocol (0x02 = Mouse)
	0,					// iInterface
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,					// bLength
	0x21,					// bDescriptorType
	0x00, 0x01,				// bcdHID  // Graphire3 is using HID version 1.00 (instead of 1.11)
	0,					// bCountryCode
	1,					// bNumDescriptors
	0x22,					// bDescriptorType
	LSB(sizeof(hid_report_desc)),		// wDescriptorLength (Graphire3 is 0x0062)
	MSB(sizeof(hid_report_desc)),		//
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,					// bLength
	5,					// bDescriptorType
	RAWHID_TX_ENDPOINT | 0x80,		// bEndpointAddress
	0x03,					// bmAttributes (0x03=intr)
	RAWHID_TX_SIZE, 0,			// wMaxPacketSize
	RAWHID_TX_INTERVAL			// bInterval
};

#define STR_MANUFACTURER	L"WACOM"
#define STR_PRODUCT		L"CTE-630-UV3.1-4 - WaxBee emulation"

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

const struct usb_string_descriptor_struct PROGMEM string5 =
{
	sizeof(STR_PRODUCT),
	3,
	STR_PRODUCT
};


struct ExtData PROGMEM hardcoded_extdata_table[] =
{
	{ (uint16_t)hardcoded_extdata_table, sizeof(struct ExtData)*EXTDATA_COUNT }, // EXTDATA_MAINTABLE

	VALUE8(EXTDATA_USB_PORT_DIGITIZER),		// EXTDATA_USB_PORT
	VALUE8(EXTDATA_SERIAL_PORT_DISABLED),		// EXTDATA_SERIAL_PORT
	VALUE8(EXTDATA_ADB_PORT_SLAVE_DIGITIZER),	// EXTDATA_ADB_PORT

	{(uint16_t)device_descriptor, sizeof(device_descriptor)},	// EXTDATA_USB_DEVICEDESC
	{(uint16_t)config_descriptor, sizeof(config_descriptor)},	// EXTDATA_USB_CONFIGDESC
	{(uint16_t)hid_report_desc, sizeof(hid_report_desc)},		// EXTDATA_USB_HIDREPORTDESC

	{(uint16_t)&string1, sizeof(STR_MANUFACTURER) },		// EXTDATA_USB_STRING1
	{(uint16_t)&dummystr, 4 },					// EXTDATA_USB_STRING2
	{(uint16_t)&dummystr, 4 },					// EXTDATA_USB_STRING3
	{(uint16_t)&dummystr, 4 },					// EXTDATA_USB_STRING4
	{(uint16_t)&string5, sizeof(STR_PRODUCT) },			// EXTDATA_USB_STRING5

	VALUE8(EXTDATA_USB_PROTOCOL_WACOM_PROTOCOL4),			// EXTDATA_USB_PROTOCOL

	VALUE8(ENDPOINT0_SIZE),						// EXTDATA_USB_ENDPOINT0SIZE
	VALUE8(RAWHID_TX_SIZE),						// EXTDATA_USB_ENDPOINT1SIZE
	VALUE8(0),							// EXTDATA_USB_ENDPOINT3SIZE

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
	VALUE8(EXTDATA_SLAVE_ORIENTATION_NORMAL),	// EXTDATA_SLAVE_ORIENTATION

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
