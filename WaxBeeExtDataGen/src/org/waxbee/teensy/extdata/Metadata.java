/**
 * 
 */
package org.waxbee.teensy.extdata;

import java.util.Iterator;

import org.json.JSONObject;
import org.json.JSONTokener;

/**
 * @author Bernard
 *
 */
public class Metadata 
{
	JSONObject itsMetadata;
	String[] itsIndexedItems;
	private String itsIdentifierPrefix;
	
	public Metadata() throws Exception
	{
		init();
	}

	private void init() throws Exception
	{
		itsMetadata = new JSONObject(new JSONTokener(ClassLoader.getSystemResourceAsStream("org/waxbee/teensy/extdata/WaxbeeConfigMetadata.json")));
		
		JSONObject items = itsMetadata.getJSONObject("items");
		
		itsIndexedItems = new String[items.length()];
		
		Iterator<String> iter = items.keys();
		
		while(iter.hasNext())
		{
			String key = iter.next();
			
			JSONObject item = items.getJSONObject(key);
			
			int index = item.getInt("index");
			
			if(itsIndexedItems[index] != null)
				throw new Exception("Internal error: Duplicate index " + index + " in metadata");
			
			itsIndexedItems[index] = key;
		}

		itsIdentifierPrefix = itsMetadata.getString("identifier_prefix");
	}

	/**
	 * @param args
	 * @throws Exception 
	 */
	public static void main(String[] args) throws Exception 
	{
		Metadata metadata = new Metadata();
				
		System.out.println(metadata.itsMetadata.toString(4));
	}

	public String getIdentifierPrefix() 
	{
		return itsIdentifierPrefix;
	}
}
