package org.waxbee;

import java.text.SimpleDateFormat;
import java.util.Date;

public class RawHid
{
	static 
	{
//		System.loadLibrary("rawhid64");
		System.load("c:\\dev\\workspace\\waxbeeconfig\\rawhidlib\\rawhid64.dll");
	}

	public static native int version();
	public static native int open(int max, int vid, int pid, int usage_page, int usage);
	public static native int recv(int num, byte[] buf, int timeout);
	public static native int send(int num, byte[] buf, int timeout);
	public static native void close(int num);
	public static native void closeAll(int num);

	public static SimpleDateFormat itsTimeFormat = new SimpleDateFormat("HH:mm:ss.SSS");

	public static void printtime() throws Exception
	{
		System.out.print(itsTimeFormat.format(new Date()));
	}
	
	public static void main(String[] args) throws Exception
	{
		System.out.println("Raw Hid test");
		
		// version check is also a test to see if the JNI library bindings appears OK
		if(RawHid.version() < 10)
		{
			System.out.print("Fatal: Unexpected RawHid library");
			return;
		}
/*		
		System.out.print("Looking for Hid device");
		
		while (true) 
		{
			if (teensy.open()) 
				break;
			
			System.out.print(".");
			Thread.sleep(500);
		}
		System.out.println("\nFound it!");
		
		System.out.println("Closing");
		teensy.close();
*/
		
		
		int numDevs = RawHid.open(1, 0x056A, -1, -1, -1);

		if (numDevs <= 0) 
		{
			System.out.print("no rawhid device found with VendorId = 0x056A\n");
			return;
		}
		
		System.out.print("found "+numDevs+" rawhid device(s) with VendorId = 0x056A\n");

		byte[] buf = new byte[64];
		
		while (true) 
		{
			// check if any Raw HID packet has arrived
			int numBytes = RawHid.recv(0, buf, 220);
			if (numBytes < 0) 
			{
				System.out.print("\nerror reading, device went offline\n");
				RawHid.close(0);
				return;
			}
			
			if (numBytes > 0) 
			{
				printtime();
				System.out.print(" - recv "+numBytes+" bytes: ");
				for (int i=0; i<numBytes; i++) 
				{
					String hex = Integer.toString(((int)buf[i]) & 255, 16);
					if(hex.length() == 1)
						System.out.print("0");
					System.out.print(hex);
					System.out.print(" ");
					if (i % 16 == 15 && i < numBytes-1) 
						System.out.print("\n");
				}

				if(buf[0] == 0x02 && numBytes == 8)
				{
					int c = (int)buf[1];
					
					int prox = (c >> 7) & 1;
					int styl = (c >> 6) & 1;
					int era  = (c >> 5) & 1;
					int res0 = (c >> 4) & 1;
					int res1 = (c >> 3) & 1;
					int b1   = (c >> 2) & 1;
					int b0   = (c >> 1) & 1;
					int touc = (c >> 0) & 1;
				
					System.out.print(" prox:"+ prox +
									" styl:" + styl +
									" era:"  + era +
									" res0:" + res0 +
									" res1:" + res1 +
									" b1:"   + b1 +
									" b0:"   + b0 +
									" touc:" + touc);
				}

				System.out.print("\n");
			}
		}
	}
}
