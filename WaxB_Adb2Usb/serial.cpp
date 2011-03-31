/*
 * serial.cpp
 *
 * Serial port I/O
 *
 *  Created on: 2011-03-20
 *      Author: Bernard
 */

#include <avr/io.h>

#include "avr_util.h"
#include "serial.h"
#include "extdata.h"

namespace serial
{
	void setupPort(uint8_t baudRateCode)
	{
		switch(baudRateCode)
		{
			case SERIAL_BAUDRATECODE_9600_8MHZ:
				UCSR1A = 0;
				UBRR1 = 51;
				break;
			case SERIAL_BAUDRATECODE_9600_16MHZ:
				UCSR1A = 0;
				UBRR1 = 103;
				break;
			case SERIAL_BAUDRATECODE_19200_8MHZ:
				UCSR1A = 0;
				UBRR1 = 25;
				break;
			case SERIAL_BAUDRATECODE_19200_16MHZ:
				UCSR1A = 0;
				UBRR1 = 51;
				break;
			case SERIAL_BAUDRATECODE_38400_8MHZ:
				UCSR1A = 0;
				UBRR1 = 12;
				break;
			case SERIAL_BAUDRATECODE_38400_16MHZ:
				UCSR1A = 0;
				UBRR1 = 25;
				break;
		}

		UCSR1B =	BITV(RXCIE1, 0) |
				BITV(TXCIE1, 0) |
				BITV(UDRIE1, 0) |
				BITV(RXEN1, 1) |
				BITV(TXEN1, 1) |
				BITV(UCSZ12, 0);   //8 bits

		UCSR1C = 	BITV(UMSEL10, 0) | // Async USART
				BITV(UMSEL11, 0) | // Async USART
				BITV(UPM10, 0) | // No parity
				BITV(UPM11, 0) | // No parity
				BITV(USBS1, 0) |   //1 stop bit
				BITV(UCSZ11, 1) |  //8 bits
				BITV(UCSZ10, 1);   //8 bits

		cbi(PORTD,PORTD2);
	}

	void setInitialPortSpeed()
	{
		uint8_t baudRateCode =	(extdata_getValue8(EXTDATA_CPU_CORE_CLOCK) << 4) |
					(extdata_getValue8(EXTDATA_INITIAL_SERIAL_PORT_SPEED));

		setupPort(baudRateCode);
	}

	void setNormalPortSpeed()
	{
		uint8_t baudRateCode =	(extdata_getValue8(EXTDATA_CPU_CORE_CLOCK) << 4) |
					(extdata_getValue8(EXTDATA_SERIAL_PORT_SPEED));

		setupPort(baudRateCode);
	}

	// TODO: use interrupts to "lengthen" the receive buffer.

	/** callback to invoke upon receiving a data byte.
	 * NOTE: This will not be called under 'interruption'.
	 */
	static void (*itsByteReceivedCallback)(uint8_t data);

	void init(void (*byteReceivedCallback)(uint8_t data))
	{
		setInitialPortSpeed();

		itsByteReceivedCallback = byteReceivedCallback;
	}

	/** will "hold" until the TxD Buffer is ready to receive a byte */
	void sendByte(uint8_t data)
	{
		/* Wait for empty transmit buffer */
		while ( !( UCSR1A & (1<<UDRE1)) );

		/* Put data into buffer, sends the data */
		UDR1 = data;
	}

	/** must be called from the main loop, will return quickly if nothing to do */
	void serialPortProcessing()
	{
		// empty the data in the FIFO
		while(UCSR1A & (1<<RXC1))
		{
			// pop byte from FIFO
			uint8_t data = UDR1;

			// process the received byte
			itsByteReceivedCallback(data);
		}
	}

	void flush()
	{
		/** empty the internal RxD FIFO buffer */
		unsigned char dummy;
		while ( UCSR1A & (1<<RXC1) )
			dummy = UDR1;
	}
}
