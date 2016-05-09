package org.waxbee.teensy.extdata;

public class BuildVariant
{
	protected BuildVariant(String name, String description, String[] supports) 
	{
		itsName = name;
		itsDescription = description; 
		itsSupports = supports;
	}

	String	itsName;
	String	itsDescription;
	String[]	itsSupports;
	
	public String getDescription() { return itsDescription; }
	public String getName() { return itsName; }
	public String[] getSupports() { return itsSupports; }
}
