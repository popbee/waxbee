/*
 * serial.h
 *
 *  Created on: 2011-03-20
 *      Author: Bernard
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdint.h>

#define EXTDATA_CPU_CORE_CLOCK_F_8MHZ	1	// 8 Mhz - half speed
#define EXTDATA_CPU_CORE_CLOCK_F_16MHZ	0	// 16 Mhz - full speed

#define EXTDATA_SERIAL_PORT_SPEED_BAUD_9600	0	// 9600 Baud, N-8-1
#define EXTDATA_SERIAL_PORT_SPEED_BAUD_38400	2	// 38400 Baud, N-8-1
#define EXTDATA_SERIAL_PORT_SPEED_BAUD_19200	1	// 19200 Baud, N-8-1

#define CPU_FREQ_CODE_8MHZ		(EXTDATA_CPU_CORE_CLOCK_F_8MHZ<<4)
#define CPU_FREQ_CODE_16MHZ		(EXTDATA_CPU_CORE_CLOCK_F_16MHZ<<4)

#define SERIAL_BAUDRATECODE_9600_16MHZ	(EXTDATA_SERIAL_PORT_SPEED_BAUD_9600  | CPU_FREQ_CODE_16MHZ)
#define SERIAL_BAUDRATECODE_9600_8MHZ	(EXTDATA_SERIAL_PORT_SPEED_BAUD_9600  | CPU_FREQ_CODE_8MHZ)

#define SERIAL_BAUDRATECODE_19200_16MHZ	(EXTDATA_SERIAL_PORT_SPEED_BAUD_19200 | CPU_FREQ_CODE_16MHZ)
#define SERIAL_BAUDRATECODE_19200_8MHZ	(EXTDATA_SERIAL_PORT_SPEED_BAUD_19200 | CPU_FREQ_CODE_8MHZ)

#define SERIAL_BAUDRATECODE_38400_16MHZ	(EXTDATA_SERIAL_PORT_SPEED_BAUD_38400 | CPU_FREQ_CODE_16MHZ)
#define SERIAL_BAUDRATECODE_38400_8MHZ	(EXTDATA_SERIAL_PORT_SPEED_BAUD_38400 | CPU_FREQ_CODE_8MHZ)

namespace serial
{
	void init(void (*byteReceivedCallback)(uint8_t data));

	void setBaudRate(uint32_t baud);

	void sendByte(uint8_t data);
	void sendString(const char* data);
	void flush();

	void serialPortProcessing();
}

#endif /* SERIAL_H_ */
