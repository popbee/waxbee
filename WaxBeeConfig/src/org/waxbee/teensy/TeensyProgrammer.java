/**
 * 
 */
package org.waxbee.teensy;

import java.io.FileReader;
import java.util.Arrays;

import org.waxbee.Teensy;

/**
 * @author Bernard
 *
 */
public class TeensyProgrammer 
{
	// processor list:
	// atmega32u4:  code_size = 32256   block_size = 128
	// at90usb162: 	code_size = 15872  	block_size = 128
	// at90usb646:  code_size = 64512   block_size = 256
	// at90usb1286: code_size = 130048  block_size = 256

	// Too bad halfkay does not return the MCU and/or some sort of board type.
	// We could probably autodetect it by "trying" to upload stuff to the device.
	//
	// NOTE: The Teensy Loader software does some sort of CPU check when a .elf 
	// file is present. Try to discover how that works.
	
	// Our Teensy is always an atmega32u4 (at least for now)
	//
	int itsCodeSize = 32256;
	int itsBlockSize = 128;

	public static void main(String[] args) throws Exception
	{
		FileReader file = new FileReader(args[0]);
		FirmwareImage firmwareImage = new FirmwareImage();
		firmwareImage.loadIntelHex(file);
		file.close();

		System.out.println("Hex File Located. ");
		
		int count = 0;
		if(!checkIfTeensyPresent())
		{
			System.out.println("Waiting for Teensy bootloader (halfkay) - press the button on the Teensy");
			
			while(!checkIfTeensyPresent())
			{
				Thread.sleep(100);
				count++;
				if(count%10 == 0)
					System.out.print(".");
			}
		}
		
		System.out.println("\nTeensy Bootloader (Halfkay) found.");
		
		TeensyProgrammer programmer = new TeensyProgrammer();
		
		programmer.program(firmwareImage, new IProgressMonitor()
		{
			@Override
			public void setProgress(int blockCount)
			{
				System.out.print(".");
			}
			
			@Override
			public void setMaximum(int maxBlockCount)
			{
			}
			
			@Override
			public void setFinished()
			{
				System.out.print("!");
			}
		});		
	}
	
	public static boolean checkIfTeensyPresent()
	{
		Teensy teensy = new Teensy();

		if(!teensy.open())
			return false;
		
		teensy.close();
		
		return true;
	}
	
	/** NOTE: this method is a "blocking" call -- probably running this in a separate 
	 * thread might be a good idea for "windowed" application.
	 */
	public void program(FirmwareImage image, IProgressMonitor progressMonitor) throws Exception
	{		
		Teensy teensy = new Teensy();

		if(!teensy.open())
		{
			throw new Exception("Teensy board not found");
		}

		try
		{
			//--------------------------------------------
			// first pass to compute the number of blocks
			//--------------------------------------------
			
			int numberOfBlocks = 0;
	
			for (int address = 0; address < itsCodeSize; address += itsBlockSize)
			{
				if (address > 0 && !image.hasData(address, itsBlockSize)) 
				{
					// don't waste time on blocks that are unused,
					// but always do the first one to erase the chip
					continue;
				}
				
				numberOfBlocks++;
			}
			
			progressMonitor.setMaximum(numberOfBlocks+1);
			
			byte[] packet = new byte[2+itsBlockSize];
	
			int blockCount = 0;
			
			for (int address = 0; address < itsCodeSize; address += itsBlockSize)
			{
				if(Thread.currentThread().isInterrupted())	// allow aborting
					return;
				
				if (address > 0 && !image.hasData(address, itsBlockSize)) 
				{
					// don't waste time on blocks that are unused,
					// but always do the first one to erase the chip
					continue;
				}
	
				progressMonitor.setProgress(blockCount);
				blockCount++;
	
				if (itsCodeSize < 0x10000) 
				{
					packet[0] = (byte)(address & 0xff);
					packet[1] = (byte)((address >> 8) & 0xff);
				} 
				else 
				{
					packet[0] = (byte)((address >> 8) & 0xff);
					packet[1] = (byte)((address >> 16) & 0xff);
				}
	
				image.getData(address, itsBlockSize, packet, 2);
	
				if(!teensy.write(packet, itsBlockSize+2, address == 0 ? 3.0 : 0.25))
					throw new Exception("Error flashing device - while writing block #"+
								blockCount+" at 0x"+Integer.toString(address,16)); 
			}
			
			// almost finished
			progressMonitor.setProgress(numberOfBlocks);
			
			// give the teensy a chance to properly digest before rebooting it
			Thread.sleep(350);
			
			// reboot Teensy
			Arrays.fill(packet, (byte)0x00);
			packet[0] = (byte)0xFF;
			packet[1] = (byte)0xFF;
			
			if(!teensy.write(packet, itsBlockSize+2, 0.50))
				throw new Exception("Error rebooting device"); 			

			// finished!
			progressMonitor.setProgress(numberOfBlocks+1);
			progressMonitor.setFinished();
		}
		finally
		{
			teensy.close();
		}
	}
}
