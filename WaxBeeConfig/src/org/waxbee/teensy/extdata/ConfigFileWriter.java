/**
 *
 */
package org.waxbee.teensy.extdata;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

/**
 * @author Bernard
 *
 */
public class ConfigFileWriter
{
	WaxbeeConfig itsWaxbeeConfig;
	
	public ConfigFileWriter(WaxbeeConfig waxbeeConfig)
	{
		itsWaxbeeConfig = waxbeeConfig;
	}
	
	public void save(File file) throws Exception
	{
		FileWriter writer = new FileWriter(file);
		
		generate(writer);
		
		writer.close();
	}
	
	public void generate(Appendable out) throws Exception
	{
		prolog(out);

		headers(out);

		for(ConfigItem item: itsWaxbeeConfig.getConfigItems())
		{
			if(item.getItemComment() != null)
			{
				out.append("\n# ");
				
				String comments = item.getItemComment().replaceAll("\n", "\n# ");
				
				out.append(comments);
				out.append("#\n");
			}

			if(item instanceof ConfigItem.SpecialItem)
				continue;

			if(item instanceof ConfigItem.EnumItem)
			{
				out.append("\n# Valid choices for ").append(item.getConfigId()).append(":\n");
				
				for(ConfigItem.EnumValue enumValue: ((ConfigItem.EnumItem)item).getEnumValues())
				{
					out.append("# ").append(Integer.toString(enumValue.getValue())).append(": ");
					out.append(enumValue.getConfigId()).append("\n");
				}
				
				out.append("\n");
			}
		
			out.append(item.getConfigId());
			out.append(" = ");
			item.renderData(out);
			out.append("\n");
		}
		
		epilog(out);
	}

	private void headers(Appendable out) throws IOException
	{
		out.append("NAME = \"");
		out.append(safeStr(itsWaxbeeConfig.getName()));
		out.append("\"\n");
		
		out.append("DESCRIPTION = \"");
		out.append(safeStr(itsWaxbeeConfig.getDescription()));
		out.append("\"\n");

		out.append("SLAVE_TABLET_X_MAX = ");
		out.append(Integer.toString(itsWaxbeeConfig.getSlaveTabletXMax()));
		out.append("\n");
		
		out.append("SLAVE_TABLET_Y_MAX = ");
		out.append(Integer.toString(itsWaxbeeConfig.getSlaveTabletYMax()));
		out.append("\n");
		
		out.append("USB_TABLET_X_MAX = ");
		out.append(Integer.toString(itsWaxbeeConfig.getUSBTabletXMax()));
		out.append("\n");
		
		out.append("USB_TABLET_Y_MAX = ");
		out.append(Integer.toString(itsWaxbeeConfig.getUSBTabletYMax()));
		out.append("\n");
	}

	private void prolog(Appendable out) throws IOException
	{
		separator(out);
		out.append("# WaxBee configuration\n");
		separator(out);
	}
	
	private String safeStr(String str)
	{
		if(str == null)
			return "";
		
		return str;
	}

	private void separator(Appendable out) throws IOException 
	{
		out.append("#------------------------------------------------------------------------------\n");
	}

	private void doubleSeparator(Appendable out) throws IOException 
	{
		out.append("#==============================================================================\n");
	}

	private void epilog(Appendable out) throws IOException
	{
		doubleSeparator(out);
		out.append("# A note about this file and its format for the braves that wants to tweak stuff\n");
		separator(out);
		out.append("# Do not change anything here unless you think you know what you are doing. :)\n#\n");
		out.append("# Note that even if it stops working (a thing that can happen quite easily),\n");
		out.append("# there are virtually no chances that you break something 'physically'.\n#\n");
		out.append("# In any case, it cannot 'brick' the WaxBee, there is always a way out. At worse,\n");
		out.append("# it might crash your OS (or the USB driver) or make it install USB drivers you\n");
		out.append("# did not want. Nothing that cannot be fixed.\n#\n");
		separator(out);
		out.append("# Configuration items have the format :\n#\n");
		out.append("#   NAME = 123\n#\n");
		out.append("# Numeric values can be expressed as base-10 (decimal) or base-16 (hexadecimal) \n");
		out.append("# The hexadecimal notation requires 0x to be prepended. Example: 0x5e\n#\n");
		out.append("# Text requires double-quotes as delimiters.\n#\n");
		out.append("# Text comments starts with a # and goes up to the end of the line. Empty lines\n");
		out.append("# are OK.\n#\n");
		out.append("# Each entry has a multi-line comment shown before. These are fixed. They will\n");
		out.append("# will be overridden when the WaxBee Config utility writes-back this file when\n");
		out.append("# 'saving'.\n#\n");
		out.append("# Each value can have a one-line comment attached which will be shown after the\n");
		out.append("# value on the same line. This comment is tied to the value and will be \n");
		out.append("# retained after a read/write of the file (unless the value changed).\n#\n");
		out.append("#   USB_PORT = 2  # DEBUG\n#\n");
		out.append("# About multi-byte values (e.g. @[ ... ] ):\n#\n");
		out.append("# The values are separated by commas and can be expressed as base-10 (decimal)\n");
		out.append("# or base-16 (hexadecimal). The hexadecimal notation requires 0x to be\n");
		out.append("# prepended.\n#\n");
		out.append("# Each value is normally shown one byte (8 bits) at a time, but two bytes (16 \n");
		out.append("# bits) are supported.# Currently, only bytes in 'Little-Endian mode' is\n");
		out.append("# supported. Use the notation: LE16() (i.e. Little Endian, 16-bits). \n#\n");
		out.append("# For example:\n#\n");
		out.append("# 	LE16(0x0c17),\n#\n");
		out.append("# A one-line comment can be 'attached' to a value location, that comment\n");
		out.append("# reflects all the bytes that occurred before the previous comment and should be\n");
		out.append("# shown together (on the same line).  These comments are tied to the 'values'.\n");
		out.append("# These comments will be retained when the WaxBee Config tool reads/writes a \n");
		out.append("# configuration file.\n#\n");
		out.append("# Typically a configuration is always a 'copy' of an existing one which already\n");
		out.append("# contained commented values\n#\n");
		doubleSeparator(out);
		out.append("\n");
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception
	{
		WaxbeeConfig waxbeeConfig = ConfigTemplate.create();
		ConfigFileWriter configFileWriter = new ConfigFileWriter(waxbeeConfig);

		configFileWriter.save(new File(args[0]));
	}

}
