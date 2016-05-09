// NOTE: FILE CONTENT GENERATED - DO NOT MODIFY

package org.waxbee.teensy.extdata;

import java.util.ArrayList;
import java.util.Arrays;

public class ConfigTemplate
{
//================================================================================
// CONSTANTS
//================================================================================

	public static final int EXTDATA_USB_STRING_COUNT = 5;

	public static final int EXTDATA_COUNT = 46;

	public static final int EXTDATA_VERSION = 4;

	public static final int EXTDATA_MAIN_TABLE_ADDRESS_PLACEHOLDER = 291;

	public static WaxbeeConfig create()
	{
		WaxbeeConfig cfg = new WaxbeeConfig();

		ArrayList<ConfigItem> items = new ArrayList<ConfigItem>(50);
		cfg.itsConfigItems = items;

		items.add(new ConfigItem.SpecialItem(0, "MAINTABLE", "MainTable location", null));
		items.add(new ConfigItem.EnumItem(1, "USB_PORT", "USB Port Usage", null, 0, null,
			Arrays.asList(
				new ConfigItem.EnumValue(0, "DISABLED", "Disabled", "USB Port is turned off", null),
				new ConfigItem.EnumValue(1, "DIGITIZER", "Digitizer Tablet", "USB Port set as a Digitizer Tablet", null),
				new ConfigItem.EnumValue(2, "DEBUG", "Debug Console", "USB Port set as a Debug Console (hid_listen utility)", new String[] {"DEBUG_SUPPORT"})
			)));
		items.add(new ConfigItem.EnumItem(2, "SERIAL_PORT", "Serial Port Usage", null, 0, null,
			Arrays.asList(
				new ConfigItem.EnumValue(0, "DISABLED", "Disabled", "Serial Port is turned off", null),
				new ConfigItem.EnumValue(1, "SLAVE_DIGITIZER", "Slave Digitizer", "Slave tablet connected through the serial port", null),
				new ConfigItem.EnumValue(2, "DEBUG", "Debugging Output", "Serial Port is used as the console port output for debuging purposes", new String[] {"DEBUG_SUPPORT"})
			)));
		items.add(new ConfigItem.EnumItem(3, "ADB_PORT", "ADB Port Usage", null, 0, null,
			Arrays.asList(
				new ConfigItem.EnumValue(0, "DISABLED", "Disabled", "ADB Port is turned off", null),
				new ConfigItem.EnumValue(1, "SLAVE_DIGITIZER", "Slave Tablet Digitizer", "ADB Port is used to talk to an ADB Tablet", new String[] {"ADB_SUPPORT"})
			)));
		items.add(new ConfigItem.BinaryItem(4, "USB_DEVICEDESC", "USB Device Descriptor", null, null, null));
		items.add(new ConfigItem.BinaryItem(5, "USB_CONFIGDESC", "USB Configuration Descriptor", null, null, null));
		items.add(new ConfigItem.BinaryItem(6, "USB_HIDREPORTDESC", "USB HID Report Descriptor", null, null, null));
		items.add(new ConfigItem.BinaryItem(7, "USB_HIDREPORTDESC2", "Second USB HID Report Descriptor", null, null, null));
		items.add(new ConfigItem.USBStringItem(8, "USB_STRING1", "USB Descriptor String index 01", null, null, null));
		items.add(new ConfigItem.USBStringItem(9, "USB_STRING2", "USB Descriptor String index 02", null, null, null));
		items.add(new ConfigItem.USBStringItem(10, "USB_STRING3", "USB Descriptor String index 03", null, null, null));
		items.add(new ConfigItem.USBStringItem(11, "USB_STRING4", "USB Descriptor String index 04", null, null, null));
		items.add(new ConfigItem.USBStringItem(12, "USB_STRING5", "USB Descriptor String index 05", null, null, null));
		items.add(new ConfigItem.EnumItem(13, "USB_PROTOCOL", "USB Protocol", null, 4, null,
			Arrays.asList(
				new ConfigItem.EnumValue(4, "WACOM_PROTOCOL4", "Wacom Protocol 4", null, new String[] {"USB_PROTOCOL_IV_SUPPORT"}),
				new ConfigItem.EnumValue(5, "WACOM_PROTOCOL5", "Wacom Protocol 5", null, new String[] {"USB_PROTOCOL_V_SUPPORT"}),
				new ConfigItem.EnumValue(6, "WACOM_BAMBOO_TOUCH", "Wacom Bamboo Touch", null, new String[] {"USB_PROTOCOL_IV_SUPPORT"}),
				new ConfigItem.EnumValue(7, "WACOM_INTUOS5", "Wacom Intuos 5", null, new String[] {"USB_PROTOCOL_V_SUPPORT"})
			)));
		items.add(new ConfigItem.UInt8Item(14, "USB_ENDPOINT0SIZE", "USB Endpoint 0 Buffer size", null, 0, null));
		items.add(new ConfigItem.UInt8Item(15, "USB_ENDPOINT1SIZE", "USB Endpoint 1 Buffer size", null, 0, null));
		items.add(new ConfigItem.UInt8Item(16, "USB_ENDPOINT2SIZE", "USB Endpoint 2 Buffer size", null, 0, null));
		items.add(new ConfigItem.UInt8Item(17, "USB_ENDPOINT3SIZE", "USB Endpoint 3 Buffer size", null, 0, null));
		items.add(new ConfigItem.UInt16Item(18, "USB_X_MIN", "USB Minimum X position (left)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(19, "USB_Y_MIN", "USB Minimum Y position (top)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(20, "USB_X_MAX", "USB Maximum X position (right)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(21, "USB_Y_MAX", "USB Maximum Y position (bottom)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(22, "USB_X_ANCHOR_MIN", "USB Minimum X anchor position (left) (0 to disable)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(23, "USB_Y_ANCHOR_MIN", "USB Minimum Y anchor position (top) (0 to disable)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(24, "USB_X_ANCHOR_MAX", "USB Maximum X anchor position (right) (0 to disable)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(25, "USB_Y_ANCHOR_MAX", "USB Maximum Y anchor position (bottom) (0 to disable)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(26, "USB_PRESSURE_MAX", "USB Stylus Pressure maximum", null, 0, null));
		items.add(new ConfigItem.EnumItem(27, "USB_BUTTON_ENCODING", "USB Buttons Encoding", null, 0, null,
			Arrays.asList(
				new ConfigItem.EnumValue(0, "NONE", "No button support", "No support for buttons encoding.", null),
				new ConfigItem.EnumValue(1, "WACOM_INTUOS2_1218", "Intuos2 12x18 menu strip (1..12)", "Encode buttons as Intuos2-style \"top menu strip\". Button numbers range from 1 to 12.", null)
			)));
		items.add(new ConfigItem.EnumItem(28, "SLAVE_PROTOCOL", "Slave (ADB/Serial side) Protocol", null, 1, null,
			Arrays.asList(
				new ConfigItem.EnumValue(1, "WACOM_ADB", "Wacom ADB", null, null),
				new ConfigItem.EnumValue(2, "WACOM_ISDV4", "Wacom ISDV4 (Penenabled)", null, new String[] {"SERIAL_ISDV4_SUPPORT"}),
				new ConfigItem.EnumValue(3, "WACOM_PROTOCOL5", "Wacom Protocol 5", null, new String[] {"SERIAL_PROTOCOL_V_SUPPORT"}),
				new ConfigItem.EnumValue(4, "WACOM_PROTOCOL4", "Wacom Protocol 4", null, new String[] {"SERIAL_PROTOCOL_IV_SUPPORT"}),
				new ConfigItem.EnumValue(5, "WACOM_ISDV4_TOUCH", "Wacom ISDV4 + Touch (Penenabled)", null, new String[] {"SERIAL_ISDV4_SUPPORT","TOUCH_SUPPORT"}),
				new ConfigItem.EnumValue(6, "TOPAZ", "Topaz tablet", null, new String[] {"SERIAL_TOPAZ_SUPPORT"})
			)));
		items.add(new ConfigItem.EnumItem(29, "SLAVE_ORIENTATION", "Slave Tablet Orientation", "(not implemented yet) Orientation affect X,Y coordinates but should also\naffect \"tilt\" and \"rotation\" values accordingly.\nNOTE: Data is rotated before MIN/MAX/ANCHOR transformation", 0, null,
			Arrays.asList(
				new ConfigItem.EnumValue(0, "NORMAL", "Standard Orientation", "Tablet original orientation", null),
				new ConfigItem.EnumValue(1, "ROTATED_RIGHT", "Rotated Right", "Tablet is rotated 90 degrees clockwise (to the right)", null),
				new ConfigItem.EnumValue(2, "UPSIDE_DOWN", "Upside Down", "Tablet is rotated 180 degrees", null),
				new ConfigItem.EnumValue(3, "ROTATED_LEFT", "Rotated Left", "Tablet is rotated 90 degrees anti-clockwise (to the left)", null)
			)));
		items.add(new ConfigItem.UInt16Item(30, "SLAVE_X_MIN", "Slave Minimum X position (left)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(31, "SLAVE_Y_MIN", "Slave Minimum Y position (top)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(32, "SLAVE_X_MAX", "Slave Maximum X position (right)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(33, "SLAVE_Y_MAX", "Slave Maximum Y position (bottom)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(34, "SLAVE_X_ANCHOR_MIN", "Slave Minimum X anchor position (left) (0 to disable)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(35, "SLAVE_Y_ANCHOR_MIN", "Slave Minimum Y anchor position (top) (0 to disable)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(36, "SLAVE_X_ANCHOR_MAX", "Slave Maximum X anchor position (right) (0 to disable)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(37, "SLAVE_Y_ANCHOR_MAX", "Slave Maximum Y anchor position (bottom) (0 to disable)", null, 0, null));
		items.add(new ConfigItem.UInt16Item(38, "SLAVE_PRESSURE_MAX", "Slave Stylus Pressure maximum", null, 0, null));
		items.add(new ConfigItem.BinaryItem(39, "SLAVE_ACTIVE_AREAS", "Slave Active Area", "Configuration of special \"Active\" areas on the tablet (like a button).", null, null));
		items.add(new ConfigItem.EnumItem(40, "CPU_CORE_CLOCK", "CPU core clock", "16Mhz is not recommended with 3.3v supply.", 0, null,
			Arrays.asList(
				new ConfigItem.EnumValue(0, "F_16MHZ", "16 Mhz - full speed", null, null),
				new ConfigItem.EnumValue(1, "F_8MHZ", "8 Mhz - half speed", null, null)
			)));
		items.add(new ConfigItem.EnumItem(41, "INITIAL_SERIAL_PORT_SPEED", "Initial Serial Port Speed", "Serial port speed at initialization time.", 0, null,
			Arrays.asList(
				new ConfigItem.EnumValue(0, "BAUD_9600", "9600 Baud, N-8-1", null, null),
				new ConfigItem.EnumValue(1, "BAUD_19200", "19200 Baud, N-8-1", null, null),
				new ConfigItem.EnumValue(2, "BAUD_38400", "38400 Baud, N-8-1", null, null)
			)));
		items.add(new ConfigItem.EnumItem(42, "SERIAL_PORT_SPEED", "Serial Port Speed", "Serial port speed during normal operation.", 0, null,
			Arrays.asList(
				new ConfigItem.EnumValue(0, "BAUD_9600", "9600 Baud, N-8-1", null, null),
				new ConfigItem.EnumValue(1, "BAUD_19200", "19200 Baud, N-8-1", null, null),
				new ConfigItem.EnumValue(2, "BAUD_38400", "38400 Baud, N-8-1", null, null)
			)));
		items.add(new ConfigItem.UInt16Item(43, "IDLE_TIME_LIMIT_MS", "Idle time limit (ms)", "Maximum time before packet gets repeated.\n0 to disable; max: 1200 ms", 0, null));
		items.add(new ConfigItem.Utf8StringItem(44, "GPIO_INIT", "GPIO Init string", "Power up setup of GPIO pins\nSyntax: comma separated list of commands. Example:\nC7^,D4-,P250,D4+,P500,D4-\nD4 : Pin D4, + drive high (VCC), - drive low (GND), ^ pull-up, ~ floating, Pn Pause for \"n\" ms", null, null));
		items.add(new ConfigItem.BinaryItem(45, "DEBUG_DATA", "Debug Data", "Multi-purpose internal debug data. To be left empty for normal operation.", null, null));

		return cfg;
	}

	public static ArrayList<BuildVariant> createBuildVariantList()
	{
		ArrayList<BuildVariant> variants = new ArrayList<BuildVariant>(10);

		variants.add(new BuildVariant("waxbee_serial", "Serial Tablet (not ISDV4)",
				new String[] {"INITGPIO_SUPPORT","TOUCH_SUPPORT","BUTTON_SUPPORT","USB_PROTOCOL_IV_SUPPORT","USB_PROTOCOL_V_SUPPORT","SERIAL_PROTOCOL_IV_SUPPORT","SERIAL_PROTOCOL_V_SUPPORT"}));
		variants.add(new BuildVariant("waxbee_debug", "Full Debugging (console on USB)",
				new String[] {"DEBUG_SUPPORT","INITGPIO_SUPPORT","TOUCH_SUPPORT","BUTTON_SUPPORT","ADB_SUPPORT","SERIAL_ISDV4_SUPPORT","SERIAL_PROTOCOL_IV_SUPPORT","SERIAL_PROTOCOL_V_SUPPORT","SERIAL_TOPAZ_SUPPORT"}));
		variants.add(new BuildVariant("waxbee_isdv4", "ISDV4 Serial Tablet",
				new String[] {"INITGPIO_SUPPORT","TOUCH_SUPPORT","BUTTON_SUPPORT","USB_PROTOCOL_IV_SUPPORT","USB_PROTOCOL_V_SUPPORT","SERIAL_ISDV4_SUPPORT"}));
		variants.add(new BuildVariant("waxbee_isdv4_debug", "Debugging of ISDV4 Serial Tablet (console on USB)",
				new String[] {"DEBUG_SUPPORT","INITGPIO_SUPPORT","TOUCH_SUPPORT","BUTTON_SUPPORT","SERIAL_ISDV4_SUPPORT"}));
		variants.add(new BuildVariant("waxbee_serial_debug", "Debugging of Serial Tablet (no ISDV4)",
				new String[] {"DEBUG_SUPPORT","INITGPIO_SUPPORT","TOUCH_SUPPORT","BUTTON_SUPPORT","SERIAL_PROTOCOL_IV_SUPPORT","SERIAL_PROTOCOL_V_SUPPORT"}));
		variants.add(new BuildVariant("waxbee_adb", "ADB Tablet",
				new String[] {"INITGPIO_SUPPORT","ADB_SUPPORT","USB_PROTOCOL_IV_SUPPORT","USB_PROTOCOL_V_SUPPORT"}));
		variants.add(new BuildVariant("waxbee_full", "Everything included",
				new String[] {"DEBUG_SUPPORT","INITGPIO_SUPPORT","TOUCH_SUPPORT","BUTTON_SUPPORT","ADB_SUPPORT","USB_PROTOCOL_IV_SUPPORT","USB_PROTOCOL_V_SUPPORT","SERIAL_ISDV4_SUPPORT","SERIAL_PROTOCOL_IV_SUPPORT","SERIAL_PROTOCOL_V_SUPPORT","SERIAL_TOPAZ_SUPPORT"}));
		variants.add(new BuildVariant("waxbee_adb_debug", "Debugging of ADB Tablet (console on USB)",
				new String[] {"DEBUG_SUPPORT","INITGPIO_SUPPORT","BUTTON_SUPPORT","ADB_SUPPORT"}));

		return variants;
	}
}
