package org.waxbee;

import java.awt.Component;
import java.awt.Font;
import java.awt.Rectangle;

import javax.swing.JTextField;

public class MiscUtil
{
	private static Font itsMonospacedfont;

	public static Font getMonospacedFont()
	{
		if(itsMonospacedfont == null)
		{
			JTextField textfield = new JTextField();
			
			Font font = textfield.getFont();
			
			itsMonospacedfont = new Font("Monospaced",Font.PLAIN,font.getSize());
		}
		
		return itsMonospacedfont;
	}

	public static void centerWithinParent(Component dlg, Component parent)
	{
		Rectangle rect = dlg.getBounds();
		dlg.setSize(500, 550);
		rect = dlg.getBounds();
		
		Rectangle parentrect = parent.getBounds();
		
		int middlex = parentrect.x + (parentrect.width/2);
		int middley = parentrect.y + (parentrect.height/2);
		
		int newx = middlex - (rect.width/2);
		int newy = middley - (rect.height/2);
		
		if(newx < 5)
			newx = 5;
		if(newy < 5)
			newy = 5;
		
		dlg.setLocation(newx,newy);
	}
	
	public static void centerWithinParent(Component dlg)
	{
		centerWithinParent(dlg, dlg.getParent());
	}
}
