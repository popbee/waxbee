package org.waxbee;

import java.awt.Checkbox;
import java.awt.Container;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.List;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.ScrollPaneConstants;
import javax.swing.SwingWorker;

import org.waxbee.teensy.FirmwareImage;
import org.waxbee.teensy.IProgressMonitor;
import org.waxbee.teensy.TeensyProgrammer;
import org.waxbee.teensy.extdata.WaxbeeConfig;

import net.miginfocom.swing.MigLayout;

@SuppressWarnings("serial")
public class ProgramDialog extends JDialog
{
	private JTextArea itsTextArea;
	private SwingWorker<String,String> itsWorker = null;
	private JButton itsCancelButton;
	private Checkbox itsAutoProgram;
	
	private boolean itsKeepReprogramming = false;
	private WaxbeeConfig itsWaxbeeConfig;
	private String itsFirmwareName;
	private JScrollPane itsScrollPane;

	public ProgramDialog(Window owner)
	{
		super(owner);
		
		setModal(true);
		
		setTitle("Device Firmware Programming");

		Container cp = getContentPane();
		cp.setLayout(new MigLayout("insets 0","[grow,fill]","[grow,fill][shrink]"));

		itsTextArea = new JTextArea();
		itsTextArea.setFont(MiscUtil.getMonospacedFont());
		
		itsTextArea.setEditable(false);
		itsTextArea.setText("Device Firmware Programming\n\n");
		itsTextArea.setCaretPosition(0);
		itsTextArea.setLineWrap(true);
		itsTextArea.setWrapStyleWord(true);

		itsScrollPane = new JScrollPane(itsTextArea);
		itsScrollPane.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
		itsScrollPane.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
		cp.add(itsScrollPane, "width 250::,wrap");

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

		itsAutoProgram = new Checkbox("Keep reprogramming", false);
		itsAutoProgram.addItemListener(new ItemListener()
		{
			@Override
			public void itemStateChanged(ItemEvent e)
			{
				itsKeepReprogramming = (e.getStateChange() == ItemEvent.SELECTED);
			}
		});
		
		p.add(itsAutoProgram, "align left");

		p.add(itsCancelButton, "tag cancel");

		pack();
		
		MiscUtil.centerWithinParent(this);
		
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
						boolean firstPass = true;
						
						while(true)
						{
							FirmwareImage firmwareImage = null;
							
							if(firstPass)
								firmwareImage = encodeFirmware();
							
							if(!TeensyProgrammer.checkIfTeensyPresent())
							{
								publish("Waiting for Teensy bootloader (halfkay)\n\n - Press the button on the Teensy...\n");
								
								while(!TeensyProgrammer.checkIfTeensyPresent() && !isCancelled())
								{
									Thread.sleep(100);
								}
							}

							if(isCancelled())
								return null;

							publish("\nTeensy Bootloader (Halfkay) found.\n");

							firmwareImage = encodeFirmware();

							TeensyProgrammer programmer = new TeensyProgrammer();
						
							programmer.program(firmwareImage, new IProgressMonitor()
							{
								@Override
								public void setProgress(int blockCount)
								{
									publish(".");
								}
								
								@Override
								public void setMaximum(int maxBlockCount)
								{
								}
								
								@Override
								public void setFinished()
								{
									publish("!");
								}
							});
	
							if(itsKeepReprogramming)
							{
								publish("\nFinished Successfully; Rebooting Device.\n\n");
								Thread.sleep(2000);
								firstPass = false;
								continue;
							}
							
							// exit processing
							break;
						}
							
						return "Finished Successfully; Rebooting Device.";
					}

					protected FirmwareImage encodeFirmware() throws Exception
					{
						StringBuilder report = new StringBuilder();
						FirmwareImage firmwareImage = MainApplication.encodeFirmware(itsFirmwareName, itsWaxbeeConfig, report);
						publish(report.toString());
						return firmwareImage;
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
							JOptionPane.showMessageDialog(ProgramDialog.this, message, null, JOptionPane.INFORMATION_MESSAGE);
						}
						catch (InterruptedException e)
						{
							// do nothing
						}
						catch (ExecutionException e)
						{
							JOptionPane.showMessageDialog(ProgramDialog.this, e.getCause().getMessage(), null, JOptionPane.ERROR_MESSAGE);
						}
						
						setVisible(false);
					}
				}).execute();
			}
		});
	}

	public void addText(String text)
	{
		itsTextArea.append(text);
		
		itsTextArea.setCaretPosition(itsTextArea.getDocument().getLength()-1);
	}

	/** this is a blocking "modal" call */
	public void program(String firmwarename, WaxbeeConfig waxbeeConfig)
	{
		itsWaxbeeConfig = waxbeeConfig;
		itsFirmwareName = firmwarename;

		setVisible(true);
		
		if(itsWorker != null)
		{
			try
			{
				itsWorker.cancel(true);
			}
			catch(CancellationException e)
			{
			}
		}
	}
}
