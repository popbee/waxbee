/**
 * 
 */
package org.waxbee.teensy.extdata;

import java.io.FileWriter;
import java.io.IOException;
import java.util.Iterator;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * @author Bernard
 *
 */
public class GenerateSource 
{
	Metadata itsMetadata;
	private String itsIdentifierPrefix;
	
	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception
	{
		
		new GenerateSource().generate();
	}
	
	public void generate() throws Exception
	{
		itsMetadata = new Metadata();

		generateCHeader();
		generateJavaClass();
		generateJavaClassConstants();
	}

	private void generateCHeader() throws IOException, JSONException 
	{
		//----------------------------
		// Generate the C Header file
		//----------------------------
		
		FileWriter writer = new FileWriter("extdata_gen.h");
		
		itsIdentifierPrefix = itsMetadata.getIdentifierPrefix();
		writer.write("//================================================================================\n" +
				"// CONSTANTS - note: this file has been generated do not modify\n" +
				"//================================================================================\n\n");

		JSONObject constants = itsMetadata.itsMetadata.getJSONObject("constants");

		for(Iterator<String> keys=constants.keys();keys.hasNext();)
		{
			String key = keys.next();
			Object value = constants.get(key);
			writer.write("#define ");
			writer.write(itsIdentifierPrefix);
			writer.write(key);
			writer.write(" ");
			if(value instanceof String)
				writer.write("\"");
			writer.write(value.toString());
			if(value instanceof String)
				writer.write("\"");
			writer.write("\n\n");
		}

		JSONObject items = itsMetadata.itsMetadata.getJSONObject("items");
		
		for(String key : itsMetadata.itsIndexedItems)
		{
			JSONObject itemobject = items.getJSONObject(key);
			
			writer.write("#define ");
			writer.write(itsIdentifierPrefix);
			writer.write(key);
			writer.write("\t");

			writer.write(Integer.toString(itemobject.getInt("index")));
			writer.write("\t// ");

			if(itemobject.has("type"))
			{
				writer.write(itemobject.optString("type"));
				writer.write(" - ");
			}
			
			writer.write(itemobject.getString("name"));
			writer.write("\n");
			
			if(itemobject.optJSONObject("enum") != null)
			{
				writer.write("\n");

				writer.write("// ");
				writer.write(itsIdentifierPrefix);
				writer.write(key);
				writer.write(" should contain one of:\n");

				JSONObject enumobj = itemobject.optJSONObject("enum");
				
				for(Iterator<String> enumitems=enumobj.keys();enumitems.hasNext();)
				{
					String enumkey = enumitems.next();
					JSONObject enumitem = enumobj.getJSONObject(enumkey);
					
					writer.write("#define ");
					writer.write(itsIdentifierPrefix);
					writer.write(key);
					writer.write("_");
					writer.write(enumkey);
					writer.write("\t");

					writer.write(Integer.toString(enumitem.getInt("value")));
					writer.write("\t// ");

					writer.write(enumitem.getString("name"));
					writer.write("\n");
				}				

				writer.write("\n");
			}
		}

		writer.close();
	}
	
	private void generateJavaClass() throws Exception 
	{
		//-------------------------------
		// Generate the Java Class file
		//-------------------------------
		
		FileWriter writer = new FileWriter("ConfigTemplate.java");

		itsIdentifierPrefix = itsMetadata.getIdentifierPrefix();

		writer.write("// NOTE: FILE CONTENT GENERATED - DO NOT MODIFY\n\n");

		writer.write("package org.waxbee.teensy.extdata;\n\n" +
					"import java.util.ArrayList;\n" +
					"import java.util.Arrays;\n\n" +	
					"public class ConfigTemplate\n{\n");
		
		writer.write("//================================================================================\n" +
					 "// CONSTANTS\n" +
					 "//================================================================================\n\n");

		JSONObject constants = itsMetadata.itsMetadata.getJSONObject("constants");

		for(Iterator<String> keys=constants.keys();keys.hasNext();)
		{
			String key = keys.next();
			Object value = constants.get(key);
			writer.write("\tpublic static final ");
			if(value instanceof String)
				writer.write("String ");
			else
				writer.write("int ");
			writer.write(itsIdentifierPrefix);
			writer.write(key);
			writer.write(" = ");
			if(value instanceof String)
				writer.write("\"");
			writer.write(value.toString());
			if(value instanceof String)
				writer.write("\"");
			writer.write(";\n\n");
		}
		
		writer.write("\tpublic static WaxbeeConfig create()\n\t{\n" +
					"\t\tWaxbeeConfig cfg = new WaxbeeConfig();\n\n" +
					"\t\tArrayList<ConfigItem> items = new ArrayList<ConfigItem>(50);\n" +
					"\t\tcfg.itsConfigItems = items;\n\n");

		JSONObject items = itsMetadata.itsMetadata.getJSONObject("items");
		
		for(String key : itsMetadata.itsIndexedItems)
		{
			JSONObject itemobject = items.getJSONObject(key);
			
			writer.write("\t\titems.add(new ConfigItem.");

			String typeClass = "Unknown_Type";

			String type = itemobject.optString("type", "uint8");

			if(type.equals("uint8"))
				typeClass = "UInt8Item";
			else if(type.equals("uint16"))
				typeClass = "UInt16Item";
			else if(type.equals("enum"))
				typeClass = "EnumItem";
			else if(type.equals("string"))
				typeClass = "StringItem";
			else if(type.equals("binary"))
				typeClass = "BinaryItem";
			else if(type.equals("special"))
				typeClass = "SpecialItem";
			
			writer.write(typeClass);
			writer.write("(");
			writer.write(Integer.toString(itemobject.getInt("index")));
			writer.write(", ");
			writeStringWithQuotes(writer, key);
			writer.write(", ");
			writeStringWithQuotes(writer, itemobject.optString("name", null));
			writer.write(", ");
			writeStringWithQuotes(writer, itemobject.optString("comment", null));

			if(typeClass.equals("UInt8Item"))
			{
				writer.write(", 0, null"); // value & value comment
			}
			else if(typeClass.equals("UInt16Item"))
			{
				writer.write(", 0, null"); // value & value comment
			}
			else if(typeClass.equals("EnumItem"))
			{				
				writer.write(", ");
				
				JSONObject enumobj = itemobject.optJSONObject("enum");
				
				boolean isFirst = true;
				for(String enumkey : indexedKeys(enumobj))
				{
					JSONObject enumitem = enumobj.getJSONObject(enumkey);
					
					if(!isFirst)
					{
						writer.write(",\n");
					}
					else
					{
						isFirst = false;

						writer.write(Integer.toString(enumitem.getInt("value")));
						writer.write(", null,\n"); // value & value comment
						writer.write("\t\t\tArrays.asList(\n");
					}

					writer.write("\t\t\t\tnew ConfigItem.EnumValue(");
					writer.write(Integer.toString(enumitem.getInt("value")));
					writer.write(", ");
					writeStringWithQuotes(writer, enumkey);
					writer.write(", ");
					writeStringWithQuotes(writer, enumitem.getString("name"));
					writer.write(", ");
					writeStringWithQuotes(writer, enumitem.optString("comment", null));
					writer.write(")");
				}				
				
				writer.write("\n\t\t\t)");
			}
			else if(typeClass.equals("StringItem"))
			{
				writer.write(", null, null"); // value & value comment
			}
			else if(typeClass.equals("BinaryItem"))
			{
				writer.write(", null, null"); // value & value comment
			}
			
			writer.write("));\n");
		}		
	
		writer.write("\t\treturn cfg;\n\t}\n}\n");
		
		writer.close();
	}
	
	/** properly escape special characters, outputs null if string is null */
	private void writeStringWithQuotes(FileWriter writer, String string) throws IOException 
	{
		if(string == null)
		{
			writer.write("null");		
		}
		else
		{
			string = string.replace("\n", "\\n").replace("\t", "\\t").replace("\"", "\\\"");
			
			writer.write("\"");
			writer.write(string);
			writer.write("\"");
		}
	}

	/**
	 * Sort items based on enum value and return the keys in a string array.
	 * 
	 */
	private String[] indexedKeys(JSONObject items) throws Exception
	{
		// NOTE: the technique used here is not very good. 
		// If values are large or negative, it might fail.
		// The best is to really use a "sort" method.
		
		JSONArray array = new JSONArray();
				
		Iterator<String> iter = items.keys();
		
		while(iter.hasNext())
		{
			String key = iter.next();
			
			JSONObject item = items.getJSONObject(key);
			
			int index = item.getInt("value");
	
			if(!array.isNull(index))
			{
				throw new Exception("Internal error: Duplicate enum value " + index + 
							"(" + item.optString("name") + ") in metadata");
			}
			
			array.put(index, key);
		}
		
		int itemCount = 0;
		for(int i=0;i<array.length();i++)
		{
			if(array.isNull(i))
				continue;
				
			itemCount++;
		}

		String[] indexedItems = new String[itemCount];
		
		int itemIndex = 0;
		for(int i=0;i<array.length();i++)
		{
			if(array.isNull(i))
				continue;
				
			indexedItems[itemIndex] = array.getString(i);
			
			itemIndex++;
		}
		
		return indexedItems;
	}

	private void generateJavaClassConstants() throws IOException, JSONException 
	{
		//-------------------------------
		// Generate the Java Class file
		//-------------------------------
		
		FileWriter writer = new FileWriter("ExtDataConstant.java");
		
		itsIdentifierPrefix = itsMetadata.getIdentifierPrefix();
		writer.write("//================================================================================\n" +
				"// CONSTANTS - note: this file has been generated do not modify\n" +
				"//================================================================================\n\n");

		writer.write("package org.waxbee.teensy.extdata;\n\n");

		writer.write("public class ExtDataConstant\n{\n");

		JSONObject constants = itsMetadata.itsMetadata.getJSONObject("constants");

		for(Iterator<String> keys=constants.keys();keys.hasNext();)
		{
			String key = keys.next();
			Object value = constants.get(key);
			writer.write("\tpublic static final ");
			if(value instanceof String)
				writer.write("String ");
			else
				writer.write("int ");
			writer.write(itsIdentifierPrefix);
			writer.write(key);
			writer.write(" = ");
			if(value instanceof String)
				writer.write("\"");
			writer.write(value.toString());
			if(value instanceof String)
				writer.write("\"");
			writer.write(";\n\n");
		}

		JSONObject items = itsMetadata.itsMetadata.getJSONObject("items");
		
		for(String key : itsMetadata.itsIndexedItems)
		{
			JSONObject itemobject = items.getJSONObject(key);

			writer.write("\tpublic static final int ");
			writer.write(itsIdentifierPrefix);
			writer.write(key);
			writer.write(" =\t");

			writer.write(Integer.toString(itemobject.getInt("index")));
			writer.write(";\t// ");

			if(itemobject.has("type"))
			{
				writer.write(itemobject.optString("type"));
				writer.write(" - ");
			}

			writer.write(itemobject.getString("name"));
			writer.write("\n");

			if(itemobject.optJSONObject("enum") != null)
			{
				writer.write("\n");
				
				writer.write("// ");
				writer.write(itsIdentifierPrefix);
				writer.write(key);
				writer.write(" should contain one of:\n");

				JSONObject enumobj = itemobject.optJSONObject("enum");

				for(Iterator<String> enumitems=enumobj.keys();enumitems.hasNext();)
				{
					String enumkey = enumitems.next();
					JSONObject enumitem = enumobj.getJSONObject(enumkey);

					writer.write("\tpublic static final int ");
					writer.write(itsIdentifierPrefix);
					writer.write(key);
					writer.write("_");
					writer.write(enumkey);
					writer.write(" =\t");

					writer.write(Integer.toString(enumitem.getInt("value")));
					writer.write(";\t// ");

					writer.write(enumitem.getString("name"));
					writer.write("\n");
				}				

				writer.write("\n");
			}
		}

		writer.write("\n}\n");
		writer.close();
	}
}
