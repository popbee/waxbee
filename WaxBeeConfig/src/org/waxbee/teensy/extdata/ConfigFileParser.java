/**
 * 
 */
package org.waxbee.teensy.extdata;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.LineNumberReader;
import java.io.Reader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.waxbee.teensy.extdata.ConfigItem.BinaryComment;

/**
 * @author Bernard
 *
 */
public class ConfigFileParser
{
	WaxbeeConfig itsWaxbeeConfig;
	
	HashMap<String, ConfigItem> itsName2ConfigItemMap;
	
	final static private Pattern LINEPATTERN = Pattern.compile(
			"^\\s*([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*(\"[^\"]*\"|[A-Za-z0-9\\()_]+|@\\[.*)\\s*(#\\s?(.*)\\s*)?$");

	public WaxbeeConfig parse(Reader file) throws Exception
	{
		init();
		
		LineNumberReader reader = new LineNumberReader(file);

		String line = reader.readLine();
		
		while(line != null)
		{
			Matcher matcher = LINEPATTERN.matcher(line);
			
			if(matcher.matches())
			{
				String identifier = matcher.group(1).toUpperCase();
				String data = matcher.group(2);
				String comment = matcher.group(4);

				if(identifier.equals("CONFIG_VERSION"))
				{
					itsWaxbeeConfig.setConfigVersion(extractInt(data, identifier)); 
				}
				else if(identifier.equals("NAME"))
				{
					itsWaxbeeConfig.setName(extractString(data, identifier));
				}
				else if(identifier.equals("DESCRIPTION"))
				{
					itsWaxbeeConfig.setDescription(extractString(data, identifier));
				}
				else if(identifier.equals("SLAVE_TABLET_X_MAX"))
				{
					itsWaxbeeConfig.setSlaveTabletXMax(extractInt(data, identifier)); 
				}
				else if(identifier.equals("SLAVE_TABLET_Y_MAX"))
				{
					itsWaxbeeConfig.setSlaveTabletYMax(extractInt(data, identifier)); 
				}
				else if(identifier.equals("USB_TABLET_X_MAX"))
				{
					itsWaxbeeConfig.setUSBTabletXMax(extractInt(data, identifier)); 
				}
				else if(identifier.equals("USB_TABLET_Y_MAX"))
				{
					itsWaxbeeConfig.setUSBTabletYMax(extractInt(data, identifier)); 
				}
				else
				{
					ConfigItem configItem = itsName2ConfigItemMap.get(identifier);

					if(configItem == null)
						throw new Exception("Configuration reader: Unrecognized identifier '" + identifier + "' at line " + reader.getLineNumber());
					
					if(configItem instanceof ConfigItem.USBStringItem)
					{
						ConfigItem.USBStringItem stringItem = (ConfigItem.USBStringItem)configItem;
						stringItem.setValue(extractString(data, identifier));
						stringItem.setValueComment(comment);
					}
					else if(configItem instanceof ConfigItem.IntegerItem)
					{
						ConfigItem.IntegerItem integerItem = (ConfigItem.IntegerItem)configItem;

						int value;
						
						try
						{
							value = Integer.parseInt(data);
						}
						catch(NumberFormatException e)
						{
							throw new Exception("Value '"+data+"' must be a integer number for "+identifier);
						}
						
						integerItem.validateValue(value);
						integerItem.setValue(value);
						
						integerItem.setValueComment(comment);
					}
					else if(configItem instanceof ConfigItem.BinaryItem)
					{
						parseBinary(data, comment, reader, (ConfigItem.BinaryItem)configItem);
					}
				}
			}
			
			line = reader.readLine();
		}
		
		return itsWaxbeeConfig;
	}
	
	public void parseBinaryTextField(ConfigItem.BinaryItem configItem, String text) throws Exception
	{
		LineNumberReader reader = new LineNumberReader(new StringReader(text+"\n]"));
		
		parseBinary("",null,reader,configItem);
	}
	
	final static private Pattern BINARYTOKEN = Pattern.compile(
			"(@\\[|\\]|,|LE16\\(\\s*(0x[0-9A-Fa-f]+|[0-9]+)\\s*\\)|(?<!LE[^,]{0,15})(?:0x[0-9A-Fa-f]+|[0-9]+))");

	final static private Pattern BINARYLE16 = Pattern.compile(
		"LE16\\(\\s*(0x[0-9A-Fa-f]+|[0-9]+)\\s*\\)");

	final static private Pattern BINARYLINE = Pattern.compile(
		"^\\s*([^#]*?)\\s*(?:#\\s?(.*?))?\\s*$");
	
	private void parseBinary(String firstline_data, String firstline_comment, LineNumberReader reader, 
			ConfigItem.BinaryItem configItem) throws Exception
	{
		ArrayList<Byte> bytes = new ArrayList<Byte>(100);
		ArrayList<BinaryComment> comments = new ArrayList<BinaryComment>();

		int lastIndexOnLine = -1;
		
		String data = firstline_data;
		String comment = firstline_comment;
		
		while(true)
		{		
			Matcher matcher = BINARYTOKEN.matcher(data);

			while(matcher.find())
			{
				String token = matcher.group(1);
				
				if(token.startsWith("0x"))
				{
					int byteValue = Integer.parseInt(token.substring(2),16);
					
					if(byteValue < 0 || byteValue > 255)
						throw new Exception("Value "+token+" out of range. Must be between 0 and 0xFF (255). (line " + 
								reader.getLineNumber()+")");
	
					bytes.add((byte)byteValue);
					lastIndexOnLine = bytes.size()-1;
				}
				else if(token.equals(","))
				{
					 // do nothing (check?)
				}
				else if(token.startsWith("LE16("))
				{
					// 2 bytes -- little endian encoding
					Matcher regx = BINARYLE16.matcher(token);
					regx.matches();
					int shortValue;
					if(regx.group(1).startsWith("0x"))
						shortValue = Integer.parseInt(regx.group(1).substring(2),16);
					else
						shortValue = Integer.parseInt(regx.group(1));
					
					if(shortValue < 0 || shortValue > 65535)
						throw new Exception("Value "+token+" out of range. Must be between 0 and 0xFFFF (65535). (line " + 
								reader.getLineNumber()+")");
	
					bytes.add((byte)(shortValue&0xff));
					bytes.add((byte)((shortValue>>8)&0xff));
					
					lastIndexOnLine = bytes.size()-1;
				}
				else if(token.equals("@["))
				{
					 // do nothing (check?)
				}
				else if(token.equals("]"))
				{
					// finished
					byte[] byteArray = new byte[bytes.size()];
					
					int ndx = 0;
					for(byte b : bytes)
						byteArray[ndx++] = b;
					
					configItem.setValue(byteArray);		
					configItem.setValueComments(comments);
					return;
				}
				else // numeric digits case
				{
					int byteValue = Integer.parseInt(token);
					
					if(byteValue < 0 || byteValue > 255)
						throw new Exception("Value "+token+" out of range. Must be between 0 and 255 (0xFF). (line " + 
									reader.getLineNumber()+")");
	
					bytes.add((byte)byteValue);
					lastIndexOnLine = bytes.size()-1;
				}
			}
			
			// line finished
			
			// attach comment to last byte on the line
			if(lastIndexOnLine > -1 && comment != null)
				comments.add(new ConfigItem.BinaryComment(lastIndexOnLine, comment));
			
			lastIndexOnLine = -1;

			Matcher lineMatcher;

			do
			{
				String line = reader.readLine();
				if(line == null)
					throw new Exception("End Of file unexpected while parsing identifier "+configItem.getName());
						
				lineMatcher = BINARYLINE.matcher(line);
			}
			while(!lineMatcher.matches());
			
			data = lineMatcher.group(1);
			comment = lineMatcher.group(2); // may be null
		}
	}

	private int extractInt(String data, String identifier) throws Exception
	{
		try	
		{ 
			return Integer.parseInt(data); 
		}
		catch(NumberFormatException e) 
		{ 
			throw new Exception("Value '"+data+"' must be a integer number for "+identifier); 
		}
	}
	
	private String extractString(String data, String identifier) throws Exception
	{
		if(data.length() < 2 || data.charAt(0) != '"' || data.charAt(data.length()-1) != '"')
			throw new Exception("Value expected to be a string that begins and ends with double quotes for "+
					identifier+". Value: "+data);
		
		return data.substring(1, data.length()-1);
	}

	private void init()
	{
		itsWaxbeeConfig = ConfigTemplate.create();
		itsName2ConfigItemMap = new HashMap<String, ConfigItem>(itsWaxbeeConfig.itsConfigItems.size());
		
		for(ConfigItem configItem : itsWaxbeeConfig.getConfigItems())
			itsName2ConfigItemMap.put(configItem.getConfigId(), configItem);
	}
	
	public static WaxbeeConfig loadFile(String filename) throws FileNotFoundException, Exception
	{
		return new ConfigFileParser().parse(new FileReader(filename));
	}
	
	public static void main(String[] args)
	{
	
		WaxbeeConfig config;
		try
		{
			config = loadFile(args[0]);

			System.out.println(config.getName() + "\n" + config.getDescription());
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
