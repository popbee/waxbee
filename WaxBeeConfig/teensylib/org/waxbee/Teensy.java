package org.waxbee;

public class Teensy
{
	static 
	{
//		System.loadLibrary("teensy64");
		System.load("c:\\dev\\workspace\\waxbeeconfig\\teensylib\\teensy64.dll");
	}

	public native int version();
	public native boolean open();
	public native void close();
	public native boolean write(byte[] bytes, int len, double timeout);
	public native boolean hard_reboot();
	

	public static void main(String[] args) throws Exception
	{
		System.out.println("Teensy test");
		
		Teensy teensy = new Teensy();
		
		// version check is also a test to see if the JNI library bindings appears OK
		if(teensy.version() < 10)
		{
			System.out.print("Fatal: Unexpected Teensy library");
			return;
		}
		
		System.out.print("Looking for Teensy");
		
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
	}
}
