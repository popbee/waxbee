package org.waxbee.teensy.extdata;

import java.io.IOException;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.List;

abstract public class ConfigItem 
{
	protected ConfigItem(int extDataIndex, String configId, String name, String itemComment) 
	{
		itsExtDataIndex = extDataIndex;
		itsConfigId = configId; 
		itsName = name;
		itsItemComment = itemComment;
	}

	int		itsExtDataIndex;
	String	itsConfigId;
	String	itsName;
	String	itsItemComment;
	
	public int getExtDataIndex() { return itsExtDataIndex; }
	public String getConfigId() { return itsConfigId; }
	public String getName() { return itsName; }
	public String getItemComment() { return itsItemComment; }
	public void setExtDataIndex(int extDataIndex) { itsExtDataIndex = extDataIndex; }
	public void setConfigId(String configId) { itsConfigId = configId; }
	public void setName(String name) { itsName = name; }
	public void setItemComment(String itemComment) { itsItemComment = itemComment; }

	public String[] getRequires() throws Exception { return null; }
	abstract int getExtDataSize();
	abstract String getType();
	abstract public void renderData(Appendable out) throws IOException;
	
	protected void renderItemConfigId(Appendable out) throws IOException
	{
		out.append(itsConfigId);
	}
	
	abstract public static class IntegerItem extends ConfigItem
	{
		protected IntegerItem(int extDataIndex, String configId, String name, String itemComment,
				int value, String valueComment) 
		{
			super(extDataIndex, configId, name, itemComment);
			
			itsValue = value;
			itsValueComment = valueComment;
		}

		protected int itsValue;
		protected String itsValueComment;
		
		public int getValue() { return itsValue; }
		public String getValueComment() { return itsValueComment; }
		public void setValue(int value) { itsValue = value; }
		public void setValueComment(String valueComment) { itsValueComment = valueComment; }
		abstract void validateValue(int value) throws Exception;
		
		@Override
		public void renderData(Appendable out) throws IOException
		{
			out.append(Integer.toString(getValue()));
			if(getValueComment() != null)
			{
				out.append("\t\t# ");
				out.append(getValueComment());
			}
		}
	}
	
	final public static class UInt8Item extends IntegerItem 
	{
		public UInt8Item(int extDataIndex, String configId, String name, String itemComment,
				int value, String valueComment) 
		{
			super(extDataIndex, configId, name, itemComment, value, valueComment);
		}

		@Override
		int getExtDataSize() {	return 1; }

		@Override
		String getType() { return "uint8"; }

		@Override
		void validateValue(int value) throws Exception
		{
			if(value < 0 || value > 255)
				throw new Exception("Value "+value+" (0x"+Integer.toString(value,16)+") out of range for "+
						getName()+". Should be between 0 and 255 (0xff)");
		}
	}

	final public static class UInt16Item extends IntegerItem 
	{
		protected UInt16Item(int extDataIndex, String configId, String name, String itemComment,
				int value, String valueComment) 
		{
			super(extDataIndex, configId, name, itemComment, value, valueComment);
		}

		@Override
		int getExtDataSize() {	return 2; }

		@Override
		String getType() { return "uint16"; }

		@Override
		void validateValue(int value) throws Exception
		{
			if(value < 0 || value > 65535)
				throw new Exception("Value "+value+" (0x"+Integer.toString(value,16)+") out of range for "+
						getName()+". Should be between 0 and 65535 (0xffff)");
		}
	}

	final public static class EnumItem extends IntegerItem 
	{
		protected EnumItem(int extDataIndex, String configId, String name, String itemComment,
				int value, String valueComment, List<EnumValue> enumValues) 
		{
			super(extDataIndex, configId, name, itemComment, value, valueComment);

			itsEnumValues = enumValues;
		}
		
		List<EnumValue> itsEnumValues;

		public EnumValue currentEnum() throws Exception
		{
			for(EnumValue v : itsEnumValues)
			{
				if(v.itsValue == itsValue)
					return v;
			}
			
			throw new Exception("Enum value "+itsValue+" is unrecognized");
		}

		public String[] getRequires() throws Exception 
		{
			return currentEnum().getRequires();
		}
		
		public List<EnumValue> getEnumValues() { return itsEnumValues; }
		public void setEnumValues(List<EnumValue> enumValues) { itsEnumValues = enumValues; }

		@Override
		int getExtDataSize() {	return 1; }

		@Override
		String getType() { return "enum"; }

		@Override
		void validateValue(int value) throws Exception
		{
			for(ConfigItem.EnumValue v:getEnumValues())
			{
				if(value == v.getValue())
				{
					return;
				}
			}
			throw new Exception("Wrong value '"+value+"' for "+getName());
		}
	}

	final public static class EnumValue
	{
		public EnumValue(int value, String configId, String name,
				String comment, String[] requires) 
		{
			itsValue = value;
			itsConfigId = configId;
			itsName = name;
			itsComment = comment;
			itsRequires = requires;
		}
		int		itsValue;
		String	itsConfigId;
		String	itsName;
		String	itsComment;
		String[] itsRequires;
		
		public int getValue() { return itsValue; }
		public String getConfigId() { return itsConfigId; }
		public String getName() { return itsName; }
		public String getComment() { return itsComment; }
		public String[] getRequires() { return itsRequires; }
		public void setValue(int value) { itsValue = value; }
		public void setConfigId(String configId) { itsConfigId = configId; }
		public void setName(String name) { itsName = name; }
		public void setComment(String comment) { itsComment = comment; }
		
		@Override
		public String toString()
		{
			if(itsComment == null)
				return itsName;
			
			return itsName + "   (" + itsComment + ")";
		}
	}

	/** represents a UTF-8 generic String */
	final public static class Utf8StringItem extends ConfigItem 
	{
		public Utf8StringItem(int extDataIndex, String configId, String name, String itemComment,
				String value, String valueComment) 
		{
			super(extDataIndex, configId, name, itemComment);
			
			setValue(value);
			itsValueComment = valueComment;
		}

		String itsValue;
		byte[] itsBytes;
		String itsValueComment;
		
		public String getValue() { return itsValue; }
		public String getValueComment() { return itsValueComment; }
		public void setValueComment(String valueComment) { itsValueComment = valueComment; }

		
		public void setValue(String value)
		{
			itsValue = value;

			itsBytes = null;
		}
		
		public byte[] getBytes()
		{
			if(itsBytes == null)
			{
				byte[] bytes;
				
				if(itsValue != null)
					bytes = itsValue.getBytes(Charset.forName("UTF-8"));
				else
					bytes = new byte[0]; // empty string
	
				itsBytes = bytes;
			}
			
			return itsBytes;
		}
				
		
		@Override
		int getExtDataSize() 
		{	
			if(itsValue == null) 
				return 0;
			
			return getBytes().length; 
		}

		@Override
		String getType() { return "utf8string"; }

		@Override
		public void renderData(Appendable out) throws IOException
		{
			out.append("\"");
			if(getValue() != null)
				out.append(getValue());
			out.append("\"");
			if(getValueComment() != null)
			{
				out.append("\t\t# ");
				out.append(getValueComment());
			}
		}
	}
	
	/** represents a USB descriptor String */
	final public static class USBStringItem extends ConfigItem 
	{
		public USBStringItem(int extDataIndex, String configId, String name, String itemComment,
				String value, String valueComment) 
		{
			super(extDataIndex, configId, name, itemComment);
			
			setValue(value);
			itsValueComment = valueComment;
		}

		String itsValue;
		byte[] itsBytes;
		String itsValueComment;
		
		public String getValue() { return itsValue; }
		public String getValueComment() { return itsValueComment; }
		public void setValueComment(String valueComment) { itsValueComment = valueComment; }

		
		public void setValue(String value)
		{
			itsValue = value;

			itsBytes = null;
		}
		
		public byte[] getBytes()
		{
			if(itsBytes == null)
			{
				byte[] rawStr;
				
				if(itsValue != null)
					rawStr = itsValue.getBytes(Charset.forName("UTF-16LE"));
				else
					rawStr = new byte[0]; // empty string
	
				// generate a USB descriptor string
				
				byte[] bytes = new byte[rawStr.length + 2];
				
				bytes[0] = (byte)((rawStr.length + 2)&0xFF);
				bytes[1] = 3;  // some USB string descriptor type
				
				System.arraycopy(rawStr, 0, bytes, 2, rawStr.length);
				
				itsBytes = bytes;
			}
			
			return itsBytes;
		}
				
		
		@Override
		int getExtDataSize() 
		{	
			if(itsValue == null) 
				return 0;
			
			return getBytes().length; 
		}

		@Override
		String getType() { return "usbstring"; }

		@Override
		public void renderData(Appendable out) throws IOException
		{
			out.append("\"");
			if(getValue() != null)
				out.append(getValue());
			out.append("\"");
			if(getValueComment() != null)
			{
				out.append("\t\t# ");
				out.append(getValueComment());
			}
		}
	}

	final public static class BinaryItem extends ConfigItem 
	{
		public BinaryItem(int extDataIndex, String configId, String name, String itemComment,
				byte[] value, ArrayList<BinaryComment> valueComments) 
		{
			super(extDataIndex, configId, name, itemComment);

			itsValue = value;
			itsValueComments = valueComments;
		}

		byte[] itsValue;
		ArrayList<BinaryComment> itsValueComments;
		
		public byte[] getValue() { return itsValue; }
		public ArrayList<BinaryComment> getValueComments() { return itsValueComments; }
		public void setValue(byte[] value) { itsValue = value; }
		public void setValueComments(ArrayList<BinaryComment> valueComments) { itsValueComments = valueComments; }

		@Override
		int getExtDataSize() 
		{	
			if(itsValue == null) 
				return 0;
			
			return itsValue.length; 
		}

		@Override
		String getType() { return "binary"; }

		@Override
		public void renderData(Appendable out) throws IOException 
		{
			out.append("@[\n");
			renderBinaryData(out);
			out.append("]\n");
		}
		
		public void renderBinaryData(Appendable out) throws IOException 
		{
			if(itsValue == null)
				return;
			
			int maxIndex = itsValue.length - 1;
			int curIndex = 0;
			if(itsValueComments != null)
			{	
				for (BinaryComment bincomment : itsValueComments)
				{
					int lastIndexInLine = bincomment.itsLastIndexInclusive;
					
					renderTextBytes(out, curIndex, lastIndexInLine);
	
					if(lastIndexInLine != maxIndex)
						out.append(",");
					
					out.append("\t\t# ");
					out.append(bincomment.itsValueComment);
					out.append("\n");
					
					curIndex = lastIndexInLine+1;
				}
			}

			if(curIndex <= maxIndex)
			{
				renderTextBytes(out, curIndex, maxIndex);
			}
		}
		
		private void renderTextBytes(Appendable out, int curIndex, int lastIndexInLine) throws IOException
		{
			for(int i=curIndex; i<=lastIndexInLine; i++)
			{
				int byteValue = itsValue[i]&0xff;
				out.append("0x");
				out.append(Integer.toString(byteValue,16));
				if(i!=lastIndexInLine)
					out.append(",");
			}
		}
	}

	final static public class BinaryComment 
	{
		public BinaryComment(int lastIndexInclusive, String valueComment) 
		{
			itsLastIndexInclusive = lastIndexInclusive;
			itsValueComment = valueComment;
		}
		
		int itsLastIndexInclusive;
		String itsValueComment;
		
		public int getLastIndexInclusive() { return itsLastIndexInclusive; }
		public String getValueComment() { return itsValueComment; }
		public void setLastIndexInclusive(int lastIndexInclusive) {	itsLastIndexInclusive = lastIndexInclusive; }
		public void setValueComment(String valueComment) { itsValueComment = valueComment; }
	}

	final public static class SpecialItem extends ConfigItem 
	{
		protected SpecialItem(int extDataIndex, String configId, String name, String itemComment)
		{
			super(extDataIndex, configId, name, itemComment);
		}

		@Override
		int getExtDataSize() {	return 2; }

		@Override
		String getType() { return "special"; }

		@Override
		public void renderData(Appendable out) throws IOException
		{
		}
	}
}
