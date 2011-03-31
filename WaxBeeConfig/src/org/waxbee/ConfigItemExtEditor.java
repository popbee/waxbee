package org.waxbee;

import java.awt.Container;
import java.awt.Dialog;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JTextField;

import org.waxbee.teensy.extdata.ConfigItem;

abstract public class ConfigItemExtEditor 
{
	ConfigItem itsItem;
	
	protected ConfigItemExtEditor(ConfigItem item) 
	{
		itsItem = item;
	}
	
	abstract public void addItemEditor(Container p, Dialog parentDialog) throws Exception;
	abstract public void saveField() throws Exception;

	protected void addLabel(Container p)
	{
		JLabel label = new JLabel(itsItem.getName() + " :");
		p.add(label, "gap para");
	}

	protected void addItemComment(Container p)
	{
		if(itsItem.getItemComment() != null)
		{
			String comment = itsItem.getItemComment();
			
			comment = comment.replace("&", "&amp;");
			comment = comment.replace("<", "&lt;");
			comment = comment.replace("\n", "<P>");
			
			comment = "<html><i>"+comment;
			
			JLabel text = new JLabel(comment);
			
			p.add(text, "gap 50, span");
		}
	}
	
	abstract public static class IntegerItem extends ConfigItemExtEditor
	{
		protected IntegerItem(ConfigItem.IntegerItem item) 
		{
			super(item);
		}

		private JTextField itsTextField;

		@Override
		public void addItemEditor(Container p, Dialog parentDialog) throws Exception
		{
			addLabel(p);
			
			itsTextField = new JTextField(Integer.toString(getIntegerItem().getValue()));
			
			p.add(itsTextField, "width 40:40:80, wrap");
			addItemComment(p);
		}

		protected ConfigItem.IntegerItem getIntegerItem()
		{
			return (ConfigItem.IntegerItem)itsItem;
		}
		
		protected int getValueFromField() throws Exception
		{
			try
			{
				return Integer.parseInt(itsTextField.getText());
			}
			catch(NumberFormatException e)
			{
				throw new Exception("Value '" + itsTextField.getText() + "' is not a number for " + itsItem.getName());
			}
		}
	}
	
	final public static class UInt8Item extends IntegerItem 
	{
		public UInt8Item(ConfigItem.UInt8Item item) 
		{
			super(item);
		}
		
		@Override
		public void saveField() throws Exception
		{
			int v = getValueFromField();
			
			if(v < 0 || v > 255)
				throw new Exception("Value '" + v + "' outside valid range (0 to 255) for " + itsItem.getName());
			
			getIntegerItem().setValue(v);
		}
	}

	final public static class UInt16Item extends IntegerItem 
	{
		public UInt16Item(ConfigItem.UInt16Item item) 
		{
			super(item);
		}

		@Override
		public void saveField() throws Exception
		{
			int v = getValueFromField();

			if(v < 0 || v > 0xFFFF)
				throw new Exception("Value '" + v + "' outside valid range (0 to "+0xFFFF+") for " + itsItem.getName());
			
			getIntegerItem().setValue(v);
		}
	}

	final public static class EnumItem extends IntegerItem
	{
		public EnumItem(ConfigItem.EnumItem item) 
		{
			super(item);
		}

		protected JComboBox itsComboBox;
		
		protected ConfigItem.EnumItem getEnumItem()
		{
			return (ConfigItem.EnumItem)itsItem;
		}

		@Override
		public void addItemEditor(Container p, Dialog parentDialog) throws Exception
		{
			addLabel(p);
			itsComboBox = new JComboBox(new Vector<ConfigItem.EnumValue>(getEnumItem().getEnumValues()));
			itsComboBox.setSelectedItem(getEnumItem().currentEnum());
			p.add(itsComboBox, "width 40:40:, wrap");
			
			addItemComment(p);
		}
		
		@Override
		public void saveField() throws Exception
		{
			getIntegerItem().setValue(((ConfigItem.EnumValue)itsComboBox.getSelectedItem()).getValue());
		}		
	}

	final public static class StringItem extends ConfigItemExtEditor 
	{
		public StringItem(ConfigItem.StringItem item) 
		{
			super(item);
		}

		protected ConfigItem.StringItem getStringItem()
		{
			return (ConfigItem.StringItem)itsItem;
		}
		
		private JTextField itsTextField;

		@Override
		public void addItemEditor(Container p, Dialog parentDialog)
		{
			addLabel(p);
			itsTextField = new JTextField(getStringItem().getValue());
			
			p.add(itsTextField, "width 200:200:, wrap");
			addItemComment(p);
		}
		@Override
		public void saveField() throws Exception
		{
			getStringItem().setValue(itsTextField.getText());
		}
	}

	final public static class BinaryItem extends ConfigItemExtEditor 
	{
		ConfigItem.BinaryItem itsItemCopy;
		
		public BinaryItem(ConfigItem.BinaryItem item) 
		{
			super(item);
			
			itsItemCopy = new ConfigItem.BinaryItem(item.getExtDataIndex(), item.getConfigId(), 
					item.getName(), item.getItemComment(), item.getValue(), item.getValueComments());
		}

		protected ConfigItem.BinaryItem getBinaryItem()
		{
			return (ConfigItem.BinaryItem)itsItem;
		}

		@Override
		public void addItemEditor(Container p, final Dialog parentDialog)
		{
			addLabel(p);

			final JLabel label = new JLabel();

			label.setText(buildSummaryText());
			
			JButton button = new JButton("Edit...");
			button.addActionListener(new ActionListener()
			{
				@Override
				public void actionPerformed(ActionEvent e)
				{
					new ExtDataBinaryFieldEditor(parentDialog, itsItemCopy).setVisible(true);
					label.setText(buildSummaryText());
				}
			});

			p.add(label, "split 2");

			p.add(button, "grow 0, wrap");
			addItemComment(p);
		}
	
		private String buildSummaryText()
		{
			int length = 0;
			
			if(itsItemCopy.getValue() != null)
				length = itsItemCopy.getValue().length;
			
			return "Binary data ("+length+" bytes)";
		}

		@Override
		public void saveField() throws Exception
		{
			getBinaryItem().setValue(itsItemCopy.getValue());
			getBinaryItem().setValueComments(itsItemCopy.getValueComments());
		}
	}
}
