package org.waxbee;

import java.awt.Rectangle;

public class Monitor
{
	@Override
	public String toString()
	{
		return itsDeviceId + " " + itsBounds.height + "x" + itsBounds.width;
	}

	String itsDeviceId;
	Rectangle itsBounds;
	
	public Monitor(String deviceId, Rectangle bounds)
	{
		itsDeviceId = deviceId;
		itsBounds = bounds;
	}
}
