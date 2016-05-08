/*
 * frequency.h
 *
 * takes care of CPU frequency and related stuff
 *
 *  Created on: 2011-07-20
 *      Author: Bernard
 */

#ifndef FREQUENCY_H_
#define FREQUENCY_H_

namespace Frequency
{
	void poweronCpuFrequency(void);
	void setupCpuFrequency(void);

	/** wait for a delay. This method account for the current CPU prescaler setting */
	void delay_ms(double ms);
}

#endif // FREQUENCY_H_
