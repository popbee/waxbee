package org.waxbee.teensy;

import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.LineNumberReader;
import java.io.Reader;
import java.util.Arrays;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * FirmwareImage 
 * 
 * Represents a firmware image up to 64 Kilobytes.  Can parse and generate Intel Hex files.
 * 
 * @author Bernard
 */
public class FirmwareImage
{
	// Note: This is far from being efficient but the logic is kept relatively simple -- 
	// which is good enough for processing 64kbytes.

	/** test */
	public static void main(String[] args) throws Exception
	{
		FileReader file = new FileReader(args[0]);
		
		FirmwareImage firmwareImage = new FirmwareImage();

		firmwareImage.loadIntelHex(file);
		file.close();
		
		System.out.println("MaxLocation = 0x" + Integer.toString(firmwareImage.getMaxLocation(),16));
		
		FileWriter out = new FileWriter(args[0]+".hex");
		firmwareImage.generateIntelHex(out);
		out.close();
	}

	private static final int MAX_MEMORY_SIZE = 0x10000; // 64k

	private byte[] itsFirmwareImage;
	private boolean[] itsFirmwareMask;
	
	private int itsAddressBase;
	/** max location is inclusive */
	private int itsMaxLocation;
	private int itsMinLocation;
	
	public FirmwareImage duplicate()
	{
		FirmwareImage newImage = new FirmwareImage();
		
		newImage.itsFirmwareImage = itsFirmwareImage.clone();
		newImage.itsFirmwareMask = itsFirmwareMask.clone();
		
		newImage.itsAddressBase = itsAddressBase;
		newImage.itsMaxLocation = itsMaxLocation;
		newImage.itsMinLocation = itsMinLocation;

		return newImage;
	}

	/** The highest byte address that contains data in the firmware image
	 * 
	 * @return Max Location is _inclusive_ 
	 */
	public int getMaxLocation()
	{
		return itsMaxLocation;
	}

	/** The lowest byte address that contains data in the firmware image
	 * 
	 */
	public int getMinLocation()
	{
		return itsMinLocation;
	}	
	
	public int getSpanSize()
	{
		return itsMaxLocation - itsMinLocation;
	}
	
	private void resetImage() 
	{
		itsAddressBase = 0;
		
		itsFirmwareImage = new byte[MAX_MEMORY_SIZE];	
		Arrays.fill(itsFirmwareImage, (byte)0xFF);
		
		itsFirmwareMask = new boolean[MAX_MEMORY_SIZE];
		Arrays.fill(itsFirmwareMask, false);

		itsMaxLocation = 0;
		itsMinLocation = Integer.MAX_VALUE;		
	}

	public boolean hasData(int address, int len)
	{
		int end = address+len-1;
		
		if(address > itsMaxLocation || end < itsMinLocation)
			return false;
		
		for (int i=address; i<=end; i++) 
		{
			if (itsFirmwareMask[i]) 
				return true;
		}
		
		return false;
	}

	public int getUInt16(int address, boolean littleEndian)
	{
		if(littleEndian)
		{
			return	(itsFirmwareImage[address] & 0xff) | 
					((itsFirmwareImage[address+1] & 0xff) << 8);
		}
		else
		{
			return	((itsFirmwareImage[address] & 0xff) << 8) | 
					(itsFirmwareImage[address+1] & 0xff);
			
		}
	}
	
	public int getUInt8(int address)
	{
		return (itsFirmwareImage[address] & 0xff);
	}
	
	public byte getByte(int address)
	{
		return itsFirmwareImage[address];
	}	

	public void putByte(int address, byte value)
	{
		itsFirmwareImage[address] = value;
		itsFirmwareMask[address] = true;
		
		itsMaxLocation = Math.max(itsMaxLocation, address);
		itsMinLocation = Math.min(itsMinLocation, address);
	}
	
	public void putUInt16(int address, int value, boolean littleEndian)
	{
		if(littleEndian)
		{
			itsFirmwareImage[address]   = (byte)(value & 0xff);
			itsFirmwareImage[address+1] = (byte)((value >> 8) & 0xff);
		}
		else
		{
			itsFirmwareImage[address]   = (byte)((value >> 8) & 0xff);
			itsFirmwareImage[address+1] = (byte)(value & 0xff);
		}

		itsFirmwareMask[address] = true;
		itsFirmwareMask[address+1] = true;
		
		itsMaxLocation = Math.max(itsMaxLocation, address+1);
		itsMinLocation = Math.min(itsMinLocation, address);
	}

	public void putUInt8(int address, int value)
	{
		putByte(address, (byte)(value & 0xff));
	}
	
	public byte[] getData(int address, int len)
	{
		return Arrays.copyOfRange(itsFirmwareImage, address, address+len);
	}	

	public void getData(int address, int len, byte[] buffer, int buffer_start)
	{
		System.arraycopy(itsFirmwareImage, address, buffer, buffer_start, len);
	}
	
	/** 
	 * Put data into the firmware image at specified address.
	 */
	public void putData(int address, byte[] data)
	{
		System.arraycopy(data, 0, itsFirmwareImage, address, data.length);
		Arrays.fill(itsFirmwareMask, address, address + data.length, true);
		itsMaxLocation = Math.max(itsMaxLocation, address + data.length - 1);
		itsMinLocation = Math.min(itsMinLocation, address);
	}

	/** Fetches firmware data from a stream of characters parsed as an Intel Hex file.
	 * 
	 * @throws Exception 
	 */
	public void loadIntelHex(Reader hexfile) throws Exception
	{
		LineNumberReader reader = new LineNumberReader(hexfile);

		resetImage();

		String line;
		boolean endMarkerFound = false;
		
		while((line = reader.readLine()) != null)
		{
			if(line.charAt(0) != ':')
				continue;

			HexLine hexline;
			
			try
			{
				hexline = parseHexLine(line);
			}
			catch(Exception e)
			{
				throw new Exception("Error parsing hex file (line "+reader.getLineNumber()+"):" + e.getMessage());
			}
			
			switch(hexline.itsType)
			{
			case 0:
				if(endMarkerFound)
					throw new Exception("Data beyond End-Of-File marker");
				
				putData(itsAddressBase + hexline.itsAddress, hexline.itsData);
				break;
			case 1:
				endMarkerFound = true;
				break;
			case 2:
				if(endMarkerFound)
					throw new Exception("Records beyond End-Of-File marker");

				// Extended Segment Address Record:
				// From Wikipedia:
				// Used when 16 bits are not enough, identical to 80x86 real mode addressing. 
				// The address specified by the 02 record is multiplied by 16 (shifted 4 bits left) and 
				// added to the subsequent 00 record addresses. This allows addressing of up to a megabyte 
				// of address space. The address field of this record has to be 0000, the byte count is 
				// 02 (the segment is 16-bit). The least significant hex digit of the segment address is always 0
				
				if(hexline.itsAddress != 0 || hexline.itsData.length != 2)
					throw new Exception("Error parsing hex file (line " + reader.getLineNumber() +
							"): Invalid hex line type 2 (Extended Segment Address Record).");
				
				itsAddressBase = (hexline.itsData[0] << 12) | (hexline.itsData[1] << 4);
				break;
			case 3:
			case 4:
			case 5:
				throw new Exception("unsupported hex line type "+hexline.itsType);
			default:
				throw new Exception("invalid hex line type "+hexline.itsType);
			}
		}
		
		if(!endMarkerFound)
			throw new Exception("Missing End-Of-File marker");
	}

	private int hexValue(char digit)
	{
		if(digit >= '0' && digit <= '9')
			return digit - '0';
			
		if(digit >= 'A' && digit <= 'F')
			return 10+digit - 'A';

		if(digit >= 'a' && digit <= 'f')
			return 10+digit - 'a';

		return 0;
	}
	
	private int parseHexByte(CharSequence twodigits)
	{
		return (hexValue(twodigits.charAt(0)) << 4 ) | hexValue(twodigits.charAt(1)); 
	}

	private static final Pattern HEXLINE_PATTERN = Pattern.compile(
		"^:([0-9a-fA-F]{2})([0-9a-fA-F]{4})([0-9a-fA-F]{2})((?:[0-9a-fA-F]{2})*)([0-9a-fA-F]{2})$"); 

	private HexLine parseHexLine(String line) throws Exception
	{
		HexLine hexline = new HexLine();
		
		Matcher matcher = HEXLINE_PATTERN.matcher(line);
		
		if(!matcher.matches())
			throw new Exception("unexpected format");

		// extract data from the matched line
		
		int record_byteCount = parseHexByte(matcher.group(1));
		hexline.itsAddress = Integer.parseInt(matcher.group(2), 16);
		hexline.itsType = parseHexByte(matcher.group(3));
		String record_dataStr = matcher.group(4);
		int record_checksum = parseHexByte(matcher.group(5));
		
		if(record_dataStr.length() != record_byteCount*2)
			throw new Exception("byte count not matching data length");
		
		hexline.itsData = new byte[record_byteCount];
		
		for(int i=0; i<record_byteCount; i++)
		{
			hexline.itsData[i] = (byte)parseHexByte(record_dataStr.subSequence(i*2, (i*2)+2));
		}

		//--------------------
		// compute checksum
		//--------------------
		
		int sum = record_byteCount + 
			(hexline.itsAddress >> 8) + (hexline.itsAddress & 0xFF) + 
			hexline.itsType;
		
		for(int i=0; i<hexline.itsData.length; i++)
			sum += hexline.itsData[i];
		
		sum += record_checksum;
		
		sum &= 0xFF;
		sum ^= 0xFF;
		sum++;
		sum &= 0xFF;

		if(sum != 0)
			throw new Exception("invalid checksum");

		return hexline;
	}

	private static class HexLine
	{
		int itsType;
		int itsAddress;
		byte[] itsData;
	}

	public void generateIntelHex(Appendable out) throws IOException
	{
		final int MAX_BYTES_PER_LINE = 16;
		
		int nextLoc = 0;

		HexLine hexline = new HexLine();

		while(true)
		{
			// search next chunk beginning
			while(itsFirmwareMask[nextLoc] == false && nextLoc <= itsMaxLocation)
			{
				nextLoc++;
			}
			
			int chunkStart = nextLoc;
			
			// search how big can that chunk be
			int i = 0;
			while(itsFirmwareMask[nextLoc + i] == true && i < MAX_BYTES_PER_LINE && (i+nextLoc) <= itsMaxLocation)
				i++;
			
			int chunkLength = i;
			
			nextLoc += i;

			if(chunkLength == 0)
			{
				break; // finished
			}
			
			hexline.itsType = 0;
			hexline.itsAddress = chunkStart;
			hexline.itsData = this.getData(chunkStart, chunkLength);
			
			writeHexLine(out, hexline);
		}
		
		hexline.itsType = 1;
		hexline.itsAddress = 0;
		hexline.itsData = new byte[0];
		
		writeHexLine(out, hexline);
	}
	
	private void writeHex8(Appendable out, int value) throws IOException
	{
		String hex = Integer.toString(value & 0xff, 16).toUpperCase();
		if(hex.length() == 1)
			out.append('0');
		out.append(hex);
	}
	
	private void writeHex16(Appendable out, int value) throws IOException
	{
		String hex = Integer.toString(value & 0xffff, 16).toUpperCase();
		switch(hex.length())
		{
			case 1: out.append("000"); break;
			case 2: out.append("00"); break;
			case 3: out.append("0"); break;
		}
		
		out.append(hex);
	}

	private void writeHexLine(Appendable out, HexLine hexline) throws IOException
	{
		out.append(":");
		writeHex8(out, hexline.itsData.length);
		writeHex16(out, hexline.itsAddress);
		writeHex8(out, hexline.itsType);

		int sum = hexline.itsData.length + 
			(hexline.itsAddress >> 8) + 
			(hexline.itsAddress & 0xFF) + 
			hexline.itsType;
		
		for(int i=0; i<hexline.itsData.length; i++)
		{
			writeHex8(out, hexline.itsData[i]);
			sum += hexline.itsData[i];
		}
		
		sum &= 0xFF;
		sum ^= 0xFF;
		sum++;
		sum &= 0xFF;

		writeHex8(out, sum);
		out.append("\n");
	}

	public int findByteSequence(byte[] sequence, int searchStart)
	{
		final byte[] firmwareImage = itsFirmwareImage;
		final byte firsttokenbyte = sequence[0];
		final int maxsearch = itsMaxLocation-sequence.length + 1;
		
		outer_loop:
		for(int i = Math.max(searchStart,itsMinLocation); i<=maxsearch; i++)
 		{
			if(firmwareImage[i] == firsttokenbyte)
			{
				for(int j=0; j<sequence.length; j++)
				{
					if(firmwareImage[i+j] != sequence[j] || !itsFirmwareMask[i+j])
						continue outer_loop;
				}
				
				// found it
				return i;
			}
		}
		
		// not found
		return -1;
	}
}
