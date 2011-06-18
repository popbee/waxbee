package org.waxbee;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import javax.swing.JOptionPane;

public class Teensy
{
	static 
	{
		try
		{
			File tempfile = File.createTempFile("teensy", ".jnilib");
			tempfile.deleteOnExit();
	
			StringBuilder libnamesMsg = new StringBuilder();

			String[] liblist;

			if(System.getProperty("teensylib") != null)
			{
				liblist = new String[]{ System.getProperty( "teensylib") };
			}
			else
			{
				liblist = new String[]{ "teensy64.dll", "teensy32.dll" };
			}

			for(String libname : liblist)
			{
				if(libnamesMsg.length() > 0)
					libnamesMsg.append(", ");
				libnamesMsg.append(libname);
				
				try
				{
					if(loadlib(tempfile, libname))
						break;
				}
				catch(Exception ex)
				{
					JOptionPane.showMessageDialog(null, "Cannot load teensy native library '" + libname + 
							"'. (tried " + libnamesMsg + ") - " + ex.getMessage());
				}
			}
		}
		catch(Exception ex)
		{
			JOptionPane.showMessageDialog(null, "Cannot load teensy native libraries - " + ex.getMessage());
		}
	}

	protected static boolean loadlib(File tempfile, String libname) throws IOException, FileNotFoundException
	{
		FileOutputStream os = new FileOutputStream(tempfile);
		InputStream is = Teensy.class.getClassLoader().getResourceAsStream(libname);
		
		if(is == null)
			throw new FileNotFoundException("Library " + libname + " not found.");
		
		System.out.print("Loading Library " + libname + "...");

		byte[] buffer = new byte[32*1024];
		
		int len;
		while((len = is.read(buffer)) >= 0)
			os.write(buffer,0,len);
		
		is.close();
		os.close();
		
		try
		{
			System.load(tempfile.getAbsolutePath());
		}
		catch(UnsatisfiedLinkError x)
		{
			return false;
		}
		
		return true;
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
