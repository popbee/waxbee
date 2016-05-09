//================================================================================
// CONSTANTS - note: this file has been generated do not modify
//================================================================================

package org.waxbee.teensy.extdata;

public class ExtDataConstant
{
	public static final int EXTDATA_USB_STRING_COUNT = 5;

	public static final int EXTDATA_COUNT = 46;

	public static final int EXTDATA_VERSION = 4;

	public static final int EXTDATA_MAIN_TABLE_ADDRESS_PLACEHOLDER = 291;

	public static final int EXTDATA_MAINTABLE =	0;	// special - MainTable location
	public static final int EXTDATA_USB_PORT =	1;	// enum - USB Port Usage

// EXTDATA_USB_PORT should contain one of:
	public static final int EXTDATA_USB_PORT_DIGITIZER =	1;	// Digitizer Tablet
	public static final int EXTDATA_USB_PORT_DISABLED =	0;	// Disabled
	public static final int EXTDATA_USB_PORT_DEBUG =	2;	// Debug Console

	public static final int EXTDATA_SERIAL_PORT =	2;	// enum - Serial Port Usage

// EXTDATA_SERIAL_PORT should contain one of:
	public static final int EXTDATA_SERIAL_PORT_SLAVE_DIGITIZER =	1;	// Slave Digitizer
	public static final int EXTDATA_SERIAL_PORT_DISABLED =	0;	// Disabled
	public static final int EXTDATA_SERIAL_PORT_DEBUG =	2;	// Debugging Output

	public static final int EXTDATA_ADB_PORT =	3;	// enum - ADB Port Usage

// EXTDATA_ADB_PORT should contain one of:
	public static final int EXTDATA_ADB_PORT_SLAVE_DIGITIZER =	1;	// Slave Tablet Digitizer
	public static final int EXTDATA_ADB_PORT_DISABLED =	0;	// Disabled

	public static final int EXTDATA_USB_DEVICEDESC =	4;	// binary - USB Device Descriptor
	public static final int EXTDATA_USB_CONFIGDESC =	5;	// binary - USB Configuration Descriptor
	public static final int EXTDATA_USB_HIDREPORTDESC =	6;	// binary - USB HID Report Descriptor
	public static final int EXTDATA_USB_HIDREPORTDESC2 =	7;	// binary - Second USB HID Report Descriptor
	public static final int EXTDATA_USB_STRING1 =	8;	// usbstring - USB Descriptor String index 01
	public static final int EXTDATA_USB_STRING2 =	9;	// usbstring - USB Descriptor String index 02
	public static final int EXTDATA_USB_STRING3 =	10;	// usbstring - USB Descriptor String index 03
	public static final int EXTDATA_USB_STRING4 =	11;	// usbstring - USB Descriptor String index 04
	public static final int EXTDATA_USB_STRING5 =	12;	// usbstring - USB Descriptor String index 05
	public static final int EXTDATA_USB_PROTOCOL =	13;	// enum - USB Protocol

// EXTDATA_USB_PROTOCOL should contain one of:
	public static final int EXTDATA_USB_PROTOCOL_WACOM_PROTOCOL5 =	5;	// Wacom Protocol 5
	public static final int EXTDATA_USB_PROTOCOL_WACOM_PROTOCOL4 =	4;	// Wacom Protocol 4
	public static final int EXTDATA_USB_PROTOCOL_WACOM_BAMBOO_TOUCH =	6;	// Wacom Bamboo Touch
	public static final int EXTDATA_USB_PROTOCOL_WACOM_INTUOS5 =	7;	// Wacom Intuos 5

	public static final int EXTDATA_USB_ENDPOINT0SIZE =	14;	// USB Endpoint 0 Buffer size
	public static final int EXTDATA_USB_ENDPOINT1SIZE =	15;	// USB Endpoint 1 Buffer size
	public static final int EXTDATA_USB_ENDPOINT2SIZE =	16;	// USB Endpoint 2 Buffer size
	public static final int EXTDATA_USB_ENDPOINT3SIZE =	17;	// USB Endpoint 3 Buffer size
	public static final int EXTDATA_USB_X_MIN =	18;	// uint16 - USB Minimum X position (left)
	public static final int EXTDATA_USB_Y_MIN =	19;	// uint16 - USB Minimum Y position (top)
	public static final int EXTDATA_USB_X_MAX =	20;	// uint16 - USB Maximum X position (right)
	public static final int EXTDATA_USB_Y_MAX =	21;	// uint16 - USB Maximum Y position (bottom)
	public static final int EXTDATA_USB_X_ANCHOR_MIN =	22;	// uint16 - USB Minimum X anchor position (left) (0 to disable)
	public static final int EXTDATA_USB_Y_ANCHOR_MIN =	23;	// uint16 - USB Minimum Y anchor position (top) (0 to disable)
	public static final int EXTDATA_USB_X_ANCHOR_MAX =	24;	// uint16 - USB Maximum X anchor position (right) (0 to disable)
	public static final int EXTDATA_USB_Y_ANCHOR_MAX =	25;	// uint16 - USB Maximum Y anchor position (bottom) (0 to disable)
	public static final int EXTDATA_USB_PRESSURE_MAX =	26;	// uint16 - USB Stylus Pressure maximum
	public static final int EXTDATA_USB_BUTTON_ENCODING =	27;	// enum - USB Buttons Encoding

// EXTDATA_USB_BUTTON_ENCODING should contain one of:
	public static final int EXTDATA_USB_BUTTON_ENCODING_WACOM_INTUOS2_1218 =	1;	// Intuos2 12x18 menu strip (1..12)
	public static final int EXTDATA_USB_BUTTON_ENCODING_NONE =	0;	// No button support

	public static final int EXTDATA_SLAVE_PROTOCOL =	28;	// enum - Slave (ADB/Serial side) Protocol

// EXTDATA_SLAVE_PROTOCOL should contain one of:
	public static final int EXTDATA_SLAVE_PROTOCOL_WACOM_PROTOCOL5 =	3;	// Wacom Protocol 5
	public static final int EXTDATA_SLAVE_PROTOCOL_WACOM_PROTOCOL4 =	4;	// Wacom Protocol 4
	public static final int EXTDATA_SLAVE_PROTOCOL_WACOM_ISDV4_TOUCH =	5;	// Wacom ISDV4 + Touch (Penenabled)
	public static final int EXTDATA_SLAVE_PROTOCOL_WACOM_ISDV4 =	2;	// Wacom ISDV4 (Penenabled)
	public static final int EXTDATA_SLAVE_PROTOCOL_TOPAZ =	6;	// Topaz tablet
	public static final int EXTDATA_SLAVE_PROTOCOL_WACOM_ADB =	1;	// Wacom ADB

	public static final int EXTDATA_SLAVE_ORIENTATION =	29;	// enum - Slave Tablet Orientation

// EXTDATA_SLAVE_ORIENTATION should contain one of:
	public static final int EXTDATA_SLAVE_ORIENTATION_ROTATED_LEFT =	3;	// Rotated Left
	public static final int EXTDATA_SLAVE_ORIENTATION_UPSIDE_DOWN =	2;	// Upside Down
	public static final int EXTDATA_SLAVE_ORIENTATION_ROTATED_RIGHT =	1;	// Rotated Right
	public static final int EXTDATA_SLAVE_ORIENTATION_NORMAL =	0;	// Standard Orientation

	public static final int EXTDATA_SLAVE_X_MIN =	30;	// uint16 - Slave Minimum X position (left)
	public static final int EXTDATA_SLAVE_Y_MIN =	31;	// uint16 - Slave Minimum Y position (top)
	public static final int EXTDATA_SLAVE_X_MAX =	32;	// uint16 - Slave Maximum X position (right)
	public static final int EXTDATA_SLAVE_Y_MAX =	33;	// uint16 - Slave Maximum Y position (bottom)
	public static final int EXTDATA_SLAVE_X_ANCHOR_MIN =	34;	// uint16 - Slave Minimum X anchor position (left) (0 to disable)
	public static final int EXTDATA_SLAVE_Y_ANCHOR_MIN =	35;	// uint16 - Slave Minimum Y anchor position (top) (0 to disable)
	public static final int EXTDATA_SLAVE_X_ANCHOR_MAX =	36;	// uint16 - Slave Maximum X anchor position (right) (0 to disable)
	public static final int EXTDATA_SLAVE_Y_ANCHOR_MAX =	37;	// uint16 - Slave Maximum Y anchor position (bottom) (0 to disable)
	public static final int EXTDATA_SLAVE_PRESSURE_MAX =	38;	// uint16 - Slave Stylus Pressure maximum
	public static final int EXTDATA_SLAVE_ACTIVE_AREAS =	39;	// binary - Slave Active Area
	public static final int EXTDATA_CPU_CORE_CLOCK =	40;	// enum - CPU core clock

// EXTDATA_CPU_CORE_CLOCK should contain one of:
	public static final int EXTDATA_CPU_CORE_CLOCK_F_8MHZ =	1;	// 8 Mhz - half speed
	public static final int EXTDATA_CPU_CORE_CLOCK_F_16MHZ =	0;	// 16 Mhz - full speed

	public static final int EXTDATA_INITIAL_SERIAL_PORT_SPEED =	41;	// enum - Initial Serial Port Speed

// EXTDATA_INITIAL_SERIAL_PORT_SPEED should contain one of:
	public static final int EXTDATA_INITIAL_SERIAL_PORT_SPEED_BAUD_9600 =	0;	// 9600 Baud, N-8-1
	public static final int EXTDATA_INITIAL_SERIAL_PORT_SPEED_BAUD_38400 =	2;	// 38400 Baud, N-8-1
	public static final int EXTDATA_INITIAL_SERIAL_PORT_SPEED_BAUD_19200 =	1;	// 19200 Baud, N-8-1

	public static final int EXTDATA_SERIAL_PORT_SPEED =	42;	// enum - Serial Port Speed

// EXTDATA_SERIAL_PORT_SPEED should contain one of:
	public static final int EXTDATA_SERIAL_PORT_SPEED_BAUD_9600 =	0;	// 9600 Baud, N-8-1
	public static final int EXTDATA_SERIAL_PORT_SPEED_BAUD_38400 =	2;	// 38400 Baud, N-8-1
	public static final int EXTDATA_SERIAL_PORT_SPEED_BAUD_19200 =	1;	// 19200 Baud, N-8-1

	public static final int EXTDATA_IDLE_TIME_LIMIT_MS =	43;	// uint16 - Idle time limit (ms)
	public static final int EXTDATA_GPIO_INIT =	44;	// utf8string - GPIO Init string
	public static final int EXTDATA_DEBUG_DATA =	45;	// binary - Debug Data

}
