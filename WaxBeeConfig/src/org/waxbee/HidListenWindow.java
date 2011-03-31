package org.waxbee;

import java.awt.Container;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.List;
import java.util.concurrent.ExecutionException;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.ScrollPaneConstants;
import javax.swing.SwingWorker;

import net.miginfocom.swing.MigLayout;

@SuppressWarnings("serial")
public class HidListenWindow extends JFrame
{
	private JTextArea itsTextArea;
	private JButton itsCancelButton;
	protected SwingWorker<String, String> itsWorker;

	public HidListenWindow(Window owner)
	{
		setTitle("Hid Listen debug console");
		Container cp = getContentPane();
		cp.setLayout(new MigLayout("insets 0","[grow,fill]","[grow,fill][shrink]"));

		itsTextArea = new JTextArea();
		itsTextArea.setFont(MiscUtil.getMonospacedFont());
		
		itsTextArea.setEditable(false);
		itsTextArea.setText("Hid Listen not implemented yet!");
		itsTextArea.setCaretPosition(0);
		
		JScrollPane scrollPane = new JScrollPane(itsTextArea);
		scrollPane.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
		scrollPane.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
		cp.add(scrollPane, "width 250::,wrap");

		JPanel p;
		
		cp.add(p = new JPanel(new MigLayout("nogrid, fillx, aligny 100%, gapy unrel")));

		itsCancelButton = new JButton("Close");
		itsCancelButton.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				setVisible(false);
			}
		});

		p.add(itsCancelButton, "tag cancel");

		pack();
		
		MiscUtil.centerWithinParent(this, owner);
		
		addWindowListener(new WindowAdapter()
		{
			@Override
			public void windowOpened(WindowEvent e)
			{
				(itsWorker = new SwingWorker<String, String>()
				{
					@Override
					protected String doInBackground() throws Exception
					{
						publish("\nSomething is happening.\n");
												
						return null;
					}
					
					@Override
					protected void process(List<String> chunks)
					{
						for(String s : chunks)
						{
							addText(s);
						}
					}

					@Override
					protected void done()
					{
						String message;
						try
						{
							message = get();
							if(message != null)
								JOptionPane.showMessageDialog(HidListenWindow.this, message, null, JOptionPane.INFORMATION_MESSAGE);
						}
						catch (InterruptedException e)
						{
							// do nothing
						}
						catch (ExecutionException e)
						{
							JOptionPane.showMessageDialog(HidListenWindow.this, e.getCause().getMessage(), null, JOptionPane.ERROR_MESSAGE);
						}
						
					}
				}).execute();
			}
		});
	}

	public void addText(String text)
	{
		itsTextArea.append(text);
	}
}
