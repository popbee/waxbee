package org.waxbee.teensy;

/**
 * This will parse Intel Hex files (only records types 0,1 and 2). This code only supports 64kbytes.
 * 
 * @author Bernard
 */
public interface IProgressMonitor 
{
	void setMaximum(int maxBlockCount);
	void setProgress(int blockCount);
	void setFinished();
}
