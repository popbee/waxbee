package org.waxbee;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

import javax.swing.JOptionPane;

import net.miginfocom.layout.PlatformDefaults;

public class Teensy
{
	static 
	{
		String libname;
		
		libname = "teensy64.dll";
		
		try
		{
			File tempfile = File.createTempFile("teensy", ".jnilib");
			FileOutputStream os = new FileOutputStream(tempfile);
			InputStream is = Teensy.class.getClassLoader().getResourceAsStream(libname);
			byte[] buffer = new byte[32*1024];
			
			int len;
			while((len = is.read(buffer)) >= 0)
				os.write(buffer,0,len);
			
			is.close();
			os.close();
			System.load(tempfile.getAbsolutePath());
			tempfile.deleteOnExit();
		}
		catch(Exception ex)
		{
			JOptionPane.showMessageDialog(null, "Cannot load teensy native library: " + libname + " - " + ex.getMessage());
		}
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
