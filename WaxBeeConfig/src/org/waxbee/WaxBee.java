package org.waxbee;

import javax.swing.UIManager;



public class WaxBee 
{
	public static void main(String[] args) 
	{
		String nativeLF = UIManager.getSystemLookAndFeelClassName();

		// Install the look and feel
		try 
		{
			UIManager.setLookAndFeel(nativeLF);
		} 
		catch (Exception e) 
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		MainApplication mainapp = new MainApplication();
		
		mainapp.setVisible(true);
	}

}
