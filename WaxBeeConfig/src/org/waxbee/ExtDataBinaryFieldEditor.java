package org.waxbee;

import java.awt.Container;
import java.awt.Dialog;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.ScrollPaneConstants;

import net.miginfocom.swing.MigLayout;

import org.waxbee.teensy.extdata.ConfigFileParser;
import org.waxbee.teensy.extdata.ConfigItem;
import org.waxbee.teensy.extdata.ConfigItem.BinaryItem;

@SuppressWarnings("serial")
public class ExtDataBinaryFieldEditor extends JDialog
{
	private BinaryItem itsBinaryItem;
	
	private JButton itsOkButton;
	private JButton itsCancelButton;

	private JTextArea itsTextArea;

	public ExtDataBinaryFieldEditor(Dialog parent, ConfigItem.BinaryItem binaryItem)
	{
		super(parent, true);
		
		itsBinaryItem = binaryItem;
		
		setTitle(itsBinaryItem.getName() + " - Binary Data Editor");

		Container cp = getContentPane();
		cp.setLayout(new MigLayout("insets 0","[grow,fill]","[grow,fill][]"));

		itsTextArea = new JTextArea();
		itsTextArea.setFont(MiscUtil.getMonospacedFont());
		
		StringBuilder text = new StringBuilder();
		
		try
		{
			binaryItem.renderBinaryData(text);
		}
		catch(IOException e)
		{
		}
		
		itsTextArea.setText(text.toString());

		itsTextArea.setCaretPosition(0);
		
		JScrollPane scrollPane = new JScrollPane(itsTextArea);
		scrollPane.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
		scrollPane.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
		cp.add(scrollPane, "width 250::, growy 100, wrap");

		JPanel p = new JPanel(new MigLayout("nogrid, fillx, aligny 100%, gapy unrel"));
		cp.add(p, "growy 0");
		
		itsOkButton = new JButton("Ok");
		itsOkButton.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				
				try
				{
					save();
				}
				catch (Exception ex)
				{
					JOptionPane.showMessageDialog(ExtDataBinaryFieldEditor.this, ex.getMessage(), 
							"Error parsing text", JOptionPane.ERROR_MESSAGE);
				}
			}
		});

		itsCancelButton = new JButton("Cancel");
		itsCancelButton.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				setVisible(false);
			}
		});
		
		p.add(itsCancelButton, "tag cancel");
		p.add(itsOkButton, "tag ok");

		pack();
		
		MiscUtil.centerWithinParent(this);
	}

	protected void save() throws Exception
	{
		new ConfigFileParser().parseBinaryTextField(itsBinaryItem, itsTextArea.getText());		
		setVisible(false);
	}

}
