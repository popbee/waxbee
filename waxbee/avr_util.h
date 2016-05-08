/*
 * avr_util.h
 *
 *  Created on: 2010-11-10
 *      Author: Bernard
 */

#ifndef AVR_UTIL_H_
#define AVR_UTIL_H_

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
#ifndef tbi
// toggle bit
#define tbi(sfr, bit) (_SFR_BYTE(sfr) ^= _BV(bit))
#endif

#ifndef HIGH
#define HIGH	1
#endif
#ifndef LOW
#define LOW	0
#endif

// define handy "byte" type
typedef uint8_t byte;

#ifndef __cplusplus

#ifndef true
#define true	1
#endif

#ifndef false
#define false	0
#endif

#ifndef bool
#define bool uint8_t
#endif

#endif

#define BITV(bit, val)  (val << bit)

/** Suppresses all interrupts */
#define __BEGIN_CRITICAL_SECTION \
	{\
		uint8_t save_statusreg = SREG;\
		cli();

/** Restore global interrupt status */
#define __END_CRITICAL_SECTION \
		SREG = save_statusreg;\
	}


#endif /* AVR_UTIL_H_ */
