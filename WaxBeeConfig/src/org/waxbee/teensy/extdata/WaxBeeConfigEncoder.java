/**
 * 
 */
package org.waxbee.teensy.extdata;

import java.io.FileReader;
import java.io.FileWriter;
import java.util.Locale;

import org.waxbee.teensy.FirmwareImage;

/**
 * @author Bernard
 *
 */
public class WaxBeeConfigEncoder
{
	private WaxbeeConfig itsWaxBeeConfig;	
	private FirmwareImage itsFirmwareImage;
	private Appendable itsReport;
	
	int	itsExtDataTokenLoc;
	int	itsExtDataAddressLoc;
	int itsExtDataTableLoc;
	int itsNextChunkLoc;
	boolean itIsLittleEndian;
	
	static final byte[] EXTDAT_TOKEN = new byte[] {'E','X','T','*','D','A','T'};
	
	/*
	 * Encode the WaxBee configuration data into a copy of the firmware 'image'.
	 * 
	 * @return the modified version of the Firmware
	 */
	public FirmwareImage encodeConfig(FirmwareImage image, WaxbeeConfig config, Appendable report) throws Exception
	{
		// clone image
		itsFirmwareImage = image.duplicate();
		itsWaxBeeConfig = config;
		itsReport = report;
		
		locateExtDataToken();
		
		checkExtDataVersion();

		checkOverwriteAndDetectEndianess();
		
		setupExtDataTableLoc();
		
		if(itsWaxBeeConfig.itsConfigItems.size() > ConfigTemplate.EXTDATA_COUNT)
		{
			throw new Exception("Internal issue with configuration: WaxBee Config Item count (" + 
					itsWaxBeeConfig.itsConfigItems.size() +
					") is higher than ConfigTemplate.EXTDATA_COUNT (" + ConfigTemplate.EXTDATA_COUNT + ")");
		}
		
		for(ConfigItem item : itsWaxBeeConfig.itsConfigItems)
			encodeConfigItem(item);
		
		if(itsReport != null)
		{
			itsReport.append("Firmware size augmented of " +
					(itsFirmwareImage.getSpanSize() - image.getSpanSize()) + " bytes:\n");

			itsReport.append("Original size: "+kilobytes(image.getMaxLocation())+" "+image.getMaxLocation()+" (0x" + Integer.toString(image.getMaxLocation(), 16)+")\n");
			itsReport.append("Modified size: "+kilobytes(itsFirmwareImage.getMaxLocation())+" "+itsFirmwareImage.getMaxLocation()+" (0x" + Integer.toString(itsFirmwareImage.getMaxLocation(), 16)+")\n");
		}
		return itsFirmwareImage;
	}

	private String kilobytes(int value)
	{
		double kilos = ((double)value) / 1024.0;
		
		return String.format(Locale.ENGLISH, "%.2fK", kilos);
	}

	private void putUInt16(int address, int value16)
	{
		itsFirmwareImage.putUInt16(address, value16, itIsLittleEndian);
	}
	
	private void encodeConfigItem(ConfigItem item)
	{
		int entryAddress = itsExtDataTableLoc + (item.getExtDataIndex()*4);
		
		if(item instanceof ConfigItem.EnumItem || item instanceof ConfigItem.UInt8Item)
		{
			itsFirmwareImage.putUInt8(entryAddress, ((ConfigItem.IntegerItem)item).getValue());
			itsFirmwareImage.putByte(entryAddress+1, (byte)0);
			putUInt16(entryAddress+2, 0xFF01);
		}
		else if(item instanceof ConfigItem.UInt16Item)
		{
			putUInt16(entryAddress, ((ConfigItem.UInt16Item)item).getValue());
			putUInt16(entryAddress+2, 0xFF02);
		}
		else if(item instanceof ConfigItem.SpecialItem)
		{
			putUInt16(entryAddress, itsExtDataTableLoc);
			putUInt16(entryAddress+2, itsWaxBeeConfig.itsConfigItems.size() * 4);
		}
		else if(item instanceof ConfigItem.USBStringItem)
		{
			ConfigItem.USBStringItem stringItem = (ConfigItem.USBStringItem)item;
			
			if(stringItem.getExtDataSize() == 0)
			{
				putUInt16(entryAddress, 0);
				putUInt16(entryAddress+2, 0xFF00);
			}
			else
			{
				byte[] stringbytes = stringItem.getBytes();

				int chunkLoc = allocChunk(stringbytes.length);

				putUInt16(entryAddress, chunkLoc);
				putUInt16(entryAddress+2, stringbytes.length);

				itsFirmwareImage.putData(chunkLoc, stringbytes);
			}
		}
		else if(item instanceof ConfigItem.Utf8StringItem)
		{
			ConfigItem.Utf8StringItem stringItem = (ConfigItem.Utf8StringItem)item;
			
			if(stringItem.getExtDataSize() == 0)
			{
				putUInt16(entryAddress, 0);
				putUInt16(entryAddress+2, 0xFF00);
			}
			else
			{
				byte[] stringbytes = stringItem.getBytes();

				int chunkLoc = allocChunk(stringbytes.length);

				putUInt16(entryAddress, chunkLoc);
				putUInt16(entryAddress+2, stringbytes.length);

				itsFirmwareImage.putData(chunkLoc, stringbytes);
			}
		}
		else if(item instanceof ConfigItem.BinaryItem)
		{
			ConfigItem.BinaryItem binaryItem = (ConfigItem.BinaryItem)item;
			
			if(binaryItem.getExtDataSize() == 0)
			{
				putUInt16(entryAddress, 0);
				putUInt16(entryAddress+2, 0xFF00);
			}
			else
			{
				int chunkLoc = allocChunk(binaryItem.getExtDataSize());

				putUInt16(entryAddress, chunkLoc);
				putUInt16(entryAddress+2, binaryItem.getExtDataSize());

				itsFirmwareImage.putData(chunkLoc, binaryItem.getValue());
			}
		}
	}

	private int allocChunk(int size)
	{
		int chunkLoc = itsNextChunkLoc;
		
		itsNextChunkLoc += size;
		
		// keep chunks aligned on word boundary
		if((itsNextChunkLoc & 1) == 1)
			itsNextChunkLoc += 1;
		
		return chunkLoc;
	}

	private void locateExtDataToken() throws Exception
	{
		itsExtDataTokenLoc = itsFirmwareImage.findByteSequence(EXTDAT_TOKEN, 0);
		
		if(itsExtDataTokenLoc < 0)
			throw new Exception("'EXT*DAT' token not found in firmware image");
		
		if(itsReport != null)
			itsReport.append("EXT*DAT located at address ").append(Integer.toString(itsExtDataTokenLoc,16)).append(".\n");		
	}
	
	private void checkExtDataVersion() throws Exception
	{
		if(itsFirmwareImage.getUInt8(itsExtDataTokenLoc+EXTDAT_TOKEN.length) != ConfigTemplate.EXTDATA_VERSION)
			throw new Exception("Original Firmware version " +
					itsFirmwareImage.getUInt8(itsExtDataAddressLoc) + 
					" do not match our version (" + ConfigTemplate.EXTDATA_VERSION + ")");

		if(itsReport != null)
			itsReport.append("Original firmware version matches (").append(Integer.toString(ConfigTemplate.EXTDATA_VERSION)).append(").\n");
	}

	private void checkOverwriteAndDetectEndianess() throws Exception
	{
		itsExtDataAddressLoc = itsExtDataTokenLoc + 8;

		if(itsFirmwareImage.getUInt16(itsExtDataAddressLoc, true) == ConfigTemplate.EXTDATA_MAIN_TABLE_ADDRESS_PLACEHOLDER)
		{
			itIsLittleEndian = true;
			return;
		}
		
		if(itsFirmwareImage.getUInt16(itsExtDataAddressLoc, false) == ConfigTemplate.EXTDATA_MAIN_TABLE_ADDRESS_PLACEHOLDER)
		{
			itIsLittleEndian = false;
			return;
		}
		
		throw new Exception("Original firmware image appears to already contain EXTDATA");
	}

	private void setupExtDataTableLoc() throws Exception
	{
		int nextAvailableAddress = itsFirmwareImage.getMaxLocation() + 1;
		
		// adjust to an even address
		
		if((nextAvailableAddress & 1) == 1)
			nextAvailableAddress += 1;
		
		itsExtDataTableLoc = nextAvailableAddress;

		if(itsReport != null)
			itsReport.append("ExtData table at ").append(Integer.toString(itsExtDataTableLoc,16)).append(".\n");		
		
		putUInt16(itsExtDataAddressLoc, itsExtDataTableLoc);
		
		itsNextChunkLoc = itsExtDataTableLoc + ConfigTemplate.EXTDATA_COUNT*4;
	}

	public static void main(String[] args) throws Exception
	{
		FileReader file = new FileReader(args[0]);
		FirmwareImage firmwareImage = new FirmwareImage();
		firmwareImage.loadIntelHex(file);
		file.close();

		WaxbeeConfig config = ConfigFileParser.loadFile(args[1]);
		
		WaxBeeConfigEncoder waxbeeConfigEncoder = new WaxBeeConfigEncoder();

		FirmwareImage outputImage = waxbeeConfigEncoder.encodeConfig(firmwareImage, config, System.out);
		
		FileWriter out = new FileWriter(args[0]+"config.hex");
		outputImage.generateIntelHex(out);
		out.close();
	}
}
