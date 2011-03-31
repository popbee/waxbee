/**
 * 
 */
package org.waxbee.teensy.extdata;

import java.util.ArrayList;

/**
 * 
 * @author Bernard
 *
 */
public class WaxbeeConfig 
{
	int itsConfigVersion;

	String itsName;
	String itsDescription;

	int itsSlaveTabletXMax;
	int itsSlaveTabletYMax;

	int itsUSBTabletXMax;
	int itsUSBTabletYMax;

	ArrayList<ConfigItem> itsConfigItems;

	public int getConfigVersion()
	{
		return itsConfigVersion;
	}

	public String getName()
	{
		return itsName;
	}

	public String getDescription()
	{
		return itsDescription;
	}

	public int getSlaveTabletXMax()
	{
		return itsSlaveTabletXMax;
	}

	public int getSlaveTabletYMax()
	{
		return itsSlaveTabletYMax;
	}

	public int getUSBTabletXMax()
	{
		return itsUSBTabletXMax;
	}

	public int getUSBTabletYMax()
	{
		return itsUSBTabletYMax;
	}

	public ArrayList<ConfigItem> getConfigItems()
	{
		return itsConfigItems;
	}

	public ConfigItem getConfigItem(int extDataIndex)
	{
		for(ConfigItem item : itsConfigItems)
		{
			if(item.itsExtDataIndex == extDataIndex)
				return item;
		}
		
		return null;
	}

	public int getConfigItemInt(int extDataIndex)
	{
		ConfigItem item = getConfigItem(extDataIndex);
		
		if(item == null)
			return 0;
		
		return ((ConfigItem.IntegerItem)item).getValue();
	}
	
	public void setConfigVersion(int configVersion)
	{
		itsConfigVersion = configVersion;
	}

	public void setName(String name)
	{
		itsName = name;
	}

	public void setDescription(String description)
	{
		itsDescription = description;
	}

	public void setSlaveTabletXMax(int value)
	{
		itsSlaveTabletXMax = value;
	}

	public void setSlaveTabletYMax(int value)
	{
		itsSlaveTabletYMax = value;
	}

	public void setUSBTabletXMax(int value)
	{
		itsUSBTabletXMax = value;
	}

	public void setUSBTabletYMax(int value)
	{
		itsUSBTabletYMax = value;
	}

	public void setConfigItems(ArrayList<ConfigItem> configItems)
	{
		itsConfigItems = configItems;
	}
}
