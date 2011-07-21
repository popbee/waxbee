/*
 * gpioinit.h
 *
 * This is the module that is used to "initialize" GPIOs based on a custom string configuration
 *
 *  Created on: 2011-07-20
 *      Author: Bernard
 */

#ifndef GPIO_INIT_H_
#define GPIO_INIT_H_

namespace GpioInit
{
	/** return false on fatal error */
	bool customGpioInit(bool errors_only);
}

#endif // GPIO_INIT_H_
