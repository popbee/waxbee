
#include "frequency.h"

#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>
#include "extdata.h"

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))


namespace Frequency
{
	bool is_8Mhz; // start slow in case we are at 3.3v

	void poweronCpuFrequency(void)
	{
		// set CPU speed to 8Mhz by default
		CPU_PRESCALE(1);
		is_8Mhz = true;
	}

	void setupCpuFrequency(void)
	{
		is_8Mhz = extdata_getValue8(EXTDATA_CPU_CORE_CLOCK) == EXTDATA_CPU_CORE_CLOCK_F_8MHZ;

		if(!is_8Mhz)
		{
			// set CPU speed to 16Mhz
			CPU_PRESCALE(0);
		}
	}

	void delay_ms(double ms)
	{
		if(is_8Mhz)
			_delay_ms(ms/2.0);
		else
			_delay_ms(ms);
	}
}
