package org.waxbee;

import java.awt.Color;
import java.awt.Container;
import java.awt.Frame;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JTextField;

import net.miginfocom.swing.MigLayout;

import org.waxbee.teensy.extdata.ConfigItem;
import org.waxbee.teensy.extdata.WaxbeeConfig;

@SuppressWarnings("serial")
public class ExtDataEditor extends JDialog
{
	WaxbeeConfig itsWaxbeeConfig;
	private JTextField itsNameField;
	private JTextField itsDescriptionField;
	
	private ArrayList<ConfigItemExtEditor> itsConfigItemEditors;
	
	private JButton itsSaveButton;
	private JButton itsCancelButton;
	private JTextField itsSlaveXMaxField;
	private JTextField itsSlaveYMaxField;
	private JTextField itsUSBXMaxField;
	private JTextField itsUSBYMaxField;
	
	public ExtDataEditor(Frame parent, WaxbeeConfig waxbeeConfig) throws Exception
	{
		super(parent, true);

		itsWaxbeeConfig = waxbeeConfig;

		setTitle("WaxBee Config Editor");

		Container cp = getContentPane();
		cp.setLayout(new MigLayout("insets 0","[grow,fill]",""));

		JPanel p = new JPanel(new MigLayout("","[pref!][grow,fill]",""));

		JScrollPane scrollPane = new JScrollPane(p);
		cp.add(scrollPane, "wrap");

		addSeparator(p, "Identification");

		JPanel subp = new JPanel(new MigLayout("insets 0","[pref!][grow,fill]",""));
		p.add(subp,"span 2, growx, wrap");
		
		subp.add(new JLabel("Name :"), "gap para");
		subp.add(itsNameField = new JTextField(itsWaxbeeConfig.getName()), "growx, wrap");

		subp.add(new JLabel("Description :"), "gap para");
		subp.add(itsDescriptionField = new JTextField(itsWaxbeeConfig.getDescription()), "w 200::, growx, wrap");

		addSeparator(p, "Tablet Info");

		subp = new JPanel(new MigLayout("insets 0","",""));
		p.add(subp,"span 2, wrap");
		
		subp.add(new JLabel("ADB/Serial Tablet Max Coordinates :"), "gap para");
		subp.add(new JLabel("X="), "");
		subp.add(itsSlaveXMaxField = new JTextField(Integer.toString(itsWaxbeeConfig.getSlaveTabletXMax())), "");
		subp.add(new JLabel("Y="), "");
		subp.add(itsSlaveYMaxField = new JTextField(Integer.toString(itsWaxbeeConfig.getSlaveTabletYMax())), "wrap");

		subp.add(new JLabel("USB Tablet Max Coordinates :"), "gap para");
		subp.add(new JLabel("X="), "");
		subp.add(itsUSBXMaxField = new JTextField(Integer.toString(itsWaxbeeConfig.getUSBTabletXMax())), "");
		subp.add(new JLabel("Y="), "");
		subp.add(itsUSBYMaxField = new JTextField(Integer.toString(itsWaxbeeConfig.getUSBTabletYMax())), "wrap");

		addSeparator(p, "Firmware Configuration");

		itsConfigItemEditors = new ArrayList<ConfigItemExtEditor>(itsWaxbeeConfig.getConfigItems().size());

		for(ConfigItem item : itsWaxbeeConfig.getConfigItems())
		{
			if(item instanceof ConfigItem.USBStringItem)
			{
				itsConfigItemEditors.add(new ConfigItemExtEditor.USBStringItem((ConfigItem.USBStringItem)item));
			}
			else if(item instanceof ConfigItem.Utf8StringItem)
			{
				itsConfigItemEditors.add(new ConfigItemExtEditor.Utf8StringItem((ConfigItem.Utf8StringItem)item));
			}
			else if(item instanceof ConfigItem.UInt8Item)
			{
				itsConfigItemEditors.add(new ConfigItemExtEditor.UInt8Item((ConfigItem.UInt8Item)item));
			}
			else if(item instanceof ConfigItem.EnumItem)
			{
				itsConfigItemEditors.add(new ConfigItemExtEditor.EnumItem((ConfigItem.EnumItem)item));
			}
			else if(item instanceof ConfigItem.UInt16Item)
			{
				itsConfigItemEditors.add(new ConfigItemExtEditor.UInt16Item((ConfigItem.UInt16Item)item));
			}
			else if(item instanceof ConfigItem.BinaryItem)
			{
				itsConfigItemEditors.add(new ConfigItemExtEditor.BinaryItem((ConfigItem.BinaryItem)item));
			}
			else if(item instanceof ConfigItem.SpecialItem)
			{
				// no editors for special items
			}
		}

		for(ConfigItemExtEditor itemEditor : itsConfigItemEditors)
		{
			itemEditor.addItemEditor(p, this);
		}
		
		cp.add(p = new JPanel(new MigLayout("nogrid, fillx, aligny 100%, gapy unrel")));

		itsSaveButton = new JButton("Save");
		itsSaveButton.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				save();
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
		p.add(itsSaveButton, "tag ok");

		pack();
		
		centerWithinParent();
	}

	protected void save()
	{
		try
		{
			itsWaxbeeConfig.setName(itsNameField.getText());
			itsWaxbeeConfig.setDescription(itsDescriptionField.getText());
			itsWaxbeeConfig.setSlaveTabletXMax(extractInteger(itsSlaveXMaxField, "ADB/Serial X Resolution"));
			itsWaxbeeConfig.setSlaveTabletYMax(extractInteger(itsSlaveYMaxField, "ADB/Serial Y Resolution"));
			itsWaxbeeConfig.setUSBTabletXMax(extractInteger(itsUSBXMaxField, "USB X Resolution"));
			itsWaxbeeConfig.setUSBTabletYMax(extractInteger(itsUSBYMaxField, "USB Y Resolution"));
			
			for(ConfigItemExtEditor itemEditor : itsConfigItemEditors)
			{
					itemEditor.saveField();
			}

			setVisible(false);
		}
		catch (Exception e)
		{
			JOptionPane.showMessageDialog(this, e.getMessage());
			return;
		}
	}

	private int extractInteger(JTextField field, String label) throws Exception
	{
		try
		{
			return Integer.parseInt(field.getText());
		}
		catch(NumberFormatException e)
		{
			throw new Exception("Value '" + field.getText() + "' should be a number for " + label);
		}
	}

	static final Color LABEL_COLOR = new Color(0, 70, 213);

	private void addSeparator(Container panel, String text)
	{
		JLabel l = new JLabel(text);
		l.setForeground(LABEL_COLOR);

		panel.add(l, "gapbottom 1, span, split 2, aligny center");
		panel.add(new JSeparator(), "gapleft rel, growx");
	}

	private void centerWithinParent()
	{
		Rectangle rect = getBounds();
		setSize(rect.width+30, 710);
		rect = getBounds();
		
		Rectangle parentrect = getParent().getBounds();
		
		int middlex = parentrect.x + (parentrect.width/2);
		int middley = parentrect.y + (parentrect.height/2);
		
		int newx = middlex - (rect.width/2);
		int newy = middley - (rect.height/2);
		
		if(newx < 5)
			newx = 5;
		if(newy < 5)
			newy = 5;
		
		setLocation(newx,newy);
	}
}
