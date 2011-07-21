//================================================================================
// CONSTANTS - note: this file has been generated do not modify
//================================================================================

#define EXTDATA_USB_STRING_COUNT 5

#define EXTDATA_COUNT 42

#define EXTDATA_VERSION 4

#define EXTDATA_MAIN_TABLE_ADDRESS_PLACEHOLDER 291

#define EXTDATA_MAINTABLE	0	// special - MainTable location
#define EXTDATA_USB_PORT	1	// enum - USB Port Usage

// EXTDATA_USB_PORT should contain one of:
#define EXTDATA_USB_PORT_DIGITIZER	1	// Digitizer Tablet
#define EXTDATA_USB_PORT_DISABLED	0	// Disabled
#define EXTDATA_USB_PORT_DEBUG	2	// Debug Console

#define EXTDATA_SERIAL_PORT	2	// enum - Serial Port Usage

// EXTDATA_SERIAL_PORT should contain one of:
#define EXTDATA_SERIAL_PORT_SLAVE_DIGITIZER	1	// Slave Digitizer
#define EXTDATA_SERIAL_PORT_DISABLED	0	// Disabled
#define EXTDATA_SERIAL_PORT_DEBUG	2	// Debugging Output

#define EXTDATA_ADB_PORT	3	// enum - ADB Port Usage

// EXTDATA_ADB_PORT should contain one of:
#define EXTDATA_ADB_PORT_SLAVE_DIGITIZER	1	// Slave Tablet Digitizer
#define EXTDATA_ADB_PORT_DISABLED	0	// Disabled

#define EXTDATA_USB_DEVICEDESC	4	// binary - USB Device Descriptor
#define EXTDATA_USB_CONFIGDESC	5	// binary - USB Configuration Descriptor
#define EXTDATA_USB_HIDREPORTDESC	6	// binary - USB HID Report Descriptor
#define EXTDATA_USB_HIDREPORTDESC2	7	// binary - Second USB HID Report Descriptor
#define EXTDATA_USB_STRING1	8	// usbstring - USB Descriptor String index 01
#define EXTDATA_USB_STRING2	9	// usbstring - USB Descriptor String index 02
#define EXTDATA_USB_STRING3	10	// usbstring - USB Descriptor String index 03
#define EXTDATA_USB_STRING4	11	// usbstring - USB Descriptor String index 04
#define EXTDATA_USB_STRING5	12	// usbstring - USB Descriptor String index 05
#define EXTDATA_USB_PROTOCOL	13	// enum - USB Protocol

// EXTDATA_USB_PROTOCOL should contain one of:
#define EXTDATA_USB_PROTOCOL_WACOM_PROTOCOL5	5	// Wacom Protocol 5
#define EXTDATA_USB_PROTOCOL_WACOM_PROTOCOL4	4	// Wacom Protocol 4
#define EXTDATA_USB_PROTOCOL_WACOM_BAMBOO	6	// Wacom Bamboo

#define EXTDATA_USB_ENDPOINT0SIZE	14	// USB Endpoint 0 Buffer size
#define EXTDATA_USB_ENDPOINT1SIZE	15	// USB Endpoint 1 Buffer size
#define EXTDATA_USB_ENDPOINT2SIZE	16	// USB Endpoint 2 Buffer size
#define EXTDATA_USB_ENDPOINT3SIZE	17	// USB Endpoint 3 Buffer size
#define EXTDATA_USB_X_MIN	18	// uint16 - USB Minimum X position (left)
#define EXTDATA_USB_Y_MIN	19	// uint16 - USB Minimum Y position (top)
#define EXTDATA_USB_X_MAX	20	// uint16 - USB Maximum X position (right)
#define EXTDATA_USB_Y_MAX	21	// uint16 - USB Maximum Y position (bottom)
#define EXTDATA_USB_X_ANCHOR_MIN	22	// uint16 - USB Minimum X anchor position (left) (0 to disable)
#define EXTDATA_USB_Y_ANCHOR_MIN	23	// uint16 - USB Minimum Y anchor position (top) (0 to disable)
#define EXTDATA_USB_X_ANCHOR_MAX	24	// uint16 - USB Maximum X anchor position (right) (0 to disable)
#define EXTDATA_USB_Y_ANCHOR_MAX	25	// uint16 - USB Maximum Y anchor position (bottom) (0 to disable)
#define EXTDATA_USB_PRESSURE_MAX	26	// uint16 - USB Stylus Pressure maximum
#define EXTDATA_SLAVE_PROTOCOL	27	// enum - Slave (ADB/Serial side) Protocol

// EXTDATA_SLAVE_PROTOCOL should contain one of:
#define EXTDATA_SLAVE_PROTOCOL_WACOM_PROTOCOL5	3	// Wacom Protocol 5
#define EXTDATA_SLAVE_PROTOCOL_WACOM_PROTOCOL4	4	// Wacom Protocol 4
#define EXTDATA_SLAVE_PROTOCOL_WACOM_ISDV4	2	// Wacom ISDV4 (Penenabled)
#define EXTDATA_SLAVE_PROTOCOL_WACOM_ADB	1	// Wacom ADB

#define EXTDATA_SLAVE_ORIENTATION	28	// enum - Slave Tablet Orientation

// EXTDATA_SLAVE_ORIENTATION should contain one of:
#define EXTDATA_SLAVE_ORIENTATION_ROTATED_LEFT	3	// Rotated Left
#define EXTDATA_SLAVE_ORIENTATION_UPSIDE_DOWN	2	// Upside Down
#define EXTDATA_SLAVE_ORIENTATION_ROTATED_RIGHT	1	// Rotated Right
#define EXTDATA_SLAVE_ORIENTATION_NORMAL	0	// Standard Orientation

#define EXTDATA_SLAVE_X_MIN	29	// uint16 - Slave Minimum X position (left)
#define EXTDATA_SLAVE_Y_MIN	30	// uint16 - Slave Minimum Y position (top)
#define EXTDATA_SLAVE_X_MAX	31	// uint16 - Slave Maximum X position (right)
#define EXTDATA_SLAVE_Y_MAX	32	// uint16 - Slave Maximum Y position (bottom)
#define EXTDATA_SLAVE_X_ANCHOR_MIN	33	// uint16 - Slave Minimum X anchor position (left) (0 to disable)
#define EXTDATA_SLAVE_Y_ANCHOR_MIN	34	// uint16 - Slave Minimum Y anchor position (top) (0 to disable)
#define EXTDATA_SLAVE_X_ANCHOR_MAX	35	// uint16 - Slave Maximum X anchor position (right) (0 to disable)
#define EXTDATA_SLAVE_Y_ANCHOR_MAX	36	// uint16 - Slave Maximum Y anchor position (bottom) (0 to disable)
#define EXTDATA_SLAVE_PRESSURE_MAX	37	// uint16 - Slave Stylus Pressure maximum
#define EXTDATA_CPU_CORE_CLOCK	38	// enum - CPU core clock

// EXTDATA_CPU_CORE_CLOCK should contain one of:
#define EXTDATA_CPU_CORE_CLOCK_F_8MHZ	1	// 8 Mhz - half speed
#define EXTDATA_CPU_CORE_CLOCK_F_16MHZ	0	// 16 Mhz - full speed

#define EXTDATA_INITIAL_SERIAL_PORT_SPEED	39	// enum - Initial Serial Port Speed

// EXTDATA_INITIAL_SERIAL_PORT_SPEED should contain one of:
#define EXTDATA_INITIAL_SERIAL_PORT_SPEED_BAUD_9600	0	// 9600 Baud, N-8-1
#define EXTDATA_INITIAL_SERIAL_PORT_SPEED_BAUD_38400	2	// 38400 Baud, N-8-1
#define EXTDATA_INITIAL_SERIAL_PORT_SPEED_BAUD_19200	1	// 19200 Baud, N-8-1

#define EXTDATA_SERIAL_PORT_SPEED	40	// enum - Serial Port Speed

// EXTDATA_SERIAL_PORT_SPEED should contain one of:
#define EXTDATA_SERIAL_PORT_SPEED_BAUD_9600	0	// 9600 Baud, N-8-1
#define EXTDATA_SERIAL_PORT_SPEED_BAUD_38400	2	// 38400 Baud, N-8-1
#define EXTDATA_SERIAL_PORT_SPEED_BAUD_19200	1	// 19200 Baud, N-8-1

#define EXTDATA_GPIO_INIT	41	// utf8string - GPIO Init string
