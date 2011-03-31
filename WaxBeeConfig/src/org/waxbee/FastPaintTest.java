package org.waxbee;

import java.awt.BasicStroke;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Cursor;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Polygon;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.image.BufferedImage;

import javax.swing.JPanel;
import javax.swing.JWindow;

import org.waxbee.teensy.extdata.WaxbeeConfig;

@SuppressWarnings("serial")
public class FastPaintTest extends JWindow
{
	WaxbeeConfig itsWaxbeeConfig;
	Monitor itsMonitor;

	public FastPaintTest(MainApplication mainApplication, WaxbeeConfig waxbeeConfig, Monitor monitor)
	{
		super(mainApplication);
		
		itsWaxbeeConfig = waxbeeConfig;
		itsMonitor = monitor;
		add(new FastPaintPanel(), BorderLayout.CENTER);

		Rectangle rect = itsMonitor.itsBounds;
		
		rect = new Rectangle(rect);
		
		double size = 0.95;

		rect.x = (int)(rect.width - (rect.width * size))/2;
		rect.y = (int)(rect.height - (rect.height * size))/2;
		
		rect.width *= size;
		rect.height *= size;

		setBounds(rect);
	}

	class FastPaintPanel extends JPanel implements MouseListener, MouseMotionListener
	{
		private Point itsCurrentMousePosition = null;
		private Point itsDraggingPosition = null;
		
		public FastPaintPanel()
		{
			super(true);
			
			setLayout(null);
			addMouseListener(this);
			addMouseMotionListener(this);

			setDoubleBuffered(true);
			
//			setCursor(getBlankCursor());
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

			g.setColor(Color.GRAY);
			g.fillRect(0, 0, rect.width, rect.height);

			g.setColor(Color.WHITE);
			g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
			g.setPaintMode();

			int pointCount = rect.width;
			double radius = (double)((rect.height/2) - 10);
			int midy = rect.height/2;
			
			int[] x = new int[pointCount];
			int[] y = new int[pointCount];
			
			double timeTweak = 0;
			
			for(int i=0;i<pointCount;i++)
			{
				
				timeTweak += Math.random()*0.001;
				
				x[i] = i;
				y[i] = midy + (int)(radius*Math.cos(((double)i)*0.0101*itsCurrentMousePosition.x+timeTweak));
			}
			
			g.drawPolyline(x, y, pointCount);


			g.setColor(Color.YELLOW);
			g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
			g.setPaintMode();

			for(int i=0;i<pointCount;i++)
			{
				y[i] = midy + (int)((itsCurrentMousePosition.y/2)*(Math.sin(((double)i)*0.001*itsCurrentMousePosition.x)+(Math.random()*0.9)));
			}
			
			g.drawPolyline(x, y, pointCount);
			
/*			
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
*/
			repaint();
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
