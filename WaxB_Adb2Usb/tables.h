/*
 * tables.h
 *
 *  Created on: 2010-11-06
 *      Author: Bernard
 */

#ifndef TABLES_H_
#define TABLES_H_

#include <stdint.h>
#include <avr/pgmspace.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern uint8_t PROGMEM string0[4];

#define RAWHID_RX_SIZE		8	// receive packet size

/**************************************************************************
 *  Endpoint Buffer Configuration
 **************************************************************************/

#define RAWHID_INTERFACE	0

#define RAWHID_TX_ENDPOINT	1	// endpoint1 output


#if defined(__AVR_AT90USB162__)
#define RAWHID_TX_BUFFER	EP_SINGLE_BUFFER
#define RAWHID_RX_BUFFER	EP_SINGLE_BUFFER
#else
#define RAWHID_TX_BUFFER	EP_DOUBLE_BUFFER
#define RAWHID_RX_BUFFER	EP_DOUBLE_BUFFER
#endif


#ifdef __cplusplus
}
#endif

#endif /* TABLES_H_ */
