package org.waxbee;

import java.awt.BasicStroke;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Cursor;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.image.BufferedImage;
import java.lang.reflect.Method;

import javax.swing.JPanel;
import javax.swing.JWindow;

import org.waxbee.teensy.extdata.WaxbeeConfig;

@SuppressWarnings("serial")
public class CoordinatesEditor extends JWindow
{
	WaxbeeConfig itsWaxbeeConfig;
	Monitor itsMonitor;

	public CoordinatesEditor(MainApplication mainApplication, WaxbeeConfig waxbeeConfig, Monitor monitor)
	{
		super(mainApplication);
		
		itsWaxbeeConfig = waxbeeConfig;
		itsMonitor = monitor;
		add(new CoordinatesEditorPanel(), BorderLayout.CENTER);

		setSemiTransparent();
		setBounds(itsMonitor.itsBounds);	
	}

	void setSemiTransparent()
	{
		try 
		{
			Class<?> awtUtilitiesClass = Class.forName("com.sun.awt.AWTUtilities");
			Method mSetWindowOpacity = awtUtilitiesClass.getMethod("setWindowOpacity", Window.class, float.class);
			mSetWindowOpacity.invoke(null, this, 0.50f);
		}
		catch(Exception ex)
		{
		}
	}

	class CoordinatesEditorPanel extends JPanel implements MouseListener, MouseMotionListener
	{
		private Point itsCurrentMousePosition = null;
		private Point itsDraggingPosition = null;
		
		public CoordinatesEditorPanel()
		{
			super(true);
			
			setLayout(null);
			addMouseListener(this);
			addMouseMotionListener(this);

			setCursor(getBlankCursor());
		}

		private Cursor itsBlankCursor = null;

		private Cursor getBlankCursor()
		{
			if(itsBlankCursor == null)
			{
				// Transparent 16 x 16 pixel cursor image.
				BufferedImage cursorImg = new BufferedImage(16, 16, BufferedImage.TYPE_INT_ARGB);
	
				itsBlankCursor = Toolkit.getDefaultToolkit().createCustomCursor(
					    cursorImg, new Point(0, 0), "blank");
			}
			
			return itsBlankCursor;
		}

		@Override
		public void paintComponent(Graphics graphics)
		{
			super.paintComponent(graphics);

			Graphics2D g = (Graphics2D)graphics;
			
			Rectangle rect = getBounds();

			g.setColor(Color.WHITE);
			g.fillRect(0, 0, rect.width, rect.height);

			int rad = 5;
			int rad2 = 30;

			g.setStroke(new BasicStroke(1.0f));

			if(itsCurrentMousePosition != null)
			{				
				g.setColor(Color.BLACK);
				g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
				g.setPaintMode();

				g.drawOval(itsCurrentMousePosition.x-rad, itsCurrentMousePosition.y-rad, rad*2, rad*2);
				g.drawOval(itsCurrentMousePosition.x-rad2, itsCurrentMousePosition.y-rad2, rad2*2, rad2*2);

				g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
				g.setXORMode(Color.WHITE);

				g.drawLine(0, itsCurrentMousePosition.y, rect.width, itsCurrentMousePosition.y);
				g.drawLine(itsCurrentMousePosition.x, 0, itsCurrentMousePosition.x, rect.height);
			}

			if(itsDraggingPosition != null)
			{
				g.setColor(Color.RED);
				g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
				g.setPaintMode();

				g.drawOval(itsDraggingPosition.x-rad, itsDraggingPosition.y-rad, rad*2, rad*2);
				g.drawOval(itsDraggingPosition.x-rad2, itsDraggingPosition.y-rad2, rad2*2, rad2*2);

				g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);				
				g.setXORMode(Color.WHITE);
				g.drawLine(0, itsDraggingPosition.y, rect.width, itsDraggingPosition.y);
				g.drawLine(itsDraggingPosition.x, 0, itsDraggingPosition.x, rect.height);
			}
		}

		@Override
		public void mouseDragged(MouseEvent e)
		{
			itsDraggingPosition = e.getPoint();
			repaint();
		}

		@Override
		public void mouseMoved(MouseEvent e)
		{
			itsDraggingPosition = null;
			itsCurrentMousePosition = e.getPoint();
			repaint();
		}

		@Override
		public void mouseClicked(MouseEvent e)
		{
			// TODO Auto-generated method stub
			
		}

		@Override
		public void mouseEntered(MouseEvent e)
		{
			// TODO Auto-generated method stub
			
		}

		@Override
		public void mouseExited(MouseEvent e)
		{
			itsCurrentMousePosition = null;
			repaint();
		}

		@Override
		public void mousePressed(MouseEvent e)
		{
		}

		@Override
		public void mouseReleased(MouseEvent e)
		{
			dispose();
		}
	}
}
