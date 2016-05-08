/*
 * led.h
 *
 *  Created on: 2010-11-05
 *      Author: Bernard
 */

#ifndef LED_H_
#define LED_H_

#include "avr_util.h"

#define LED_CONFIG	sbi(DDRD,6)
#define LED_ON		sbi(PORTD,6)
#define LED_OFF		cbi(PORTD,6)
#define LED_TOGGLE	tbi(PORTD,6)

#endif /* LED_H_ */
