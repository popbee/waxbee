/*
 * adb_codec.h
 *
 *  Created on: 2010-11-13
 *      Author: Bernard
 */

#ifndef ADB_CODEC_H_
#define ADB_CODEC_H_

#include "adb_tablet_defs.h"

namespace ADB
{
	struct AdbPacket
	{
		union
		{
			struct
			{
				// note: bit order is msb first
				unsigned int parameter:2;
				unsigned int command:2;
				unsigned int address:4;
			};
			byte headerRawByte;
		};
		union
		{
			byte data[8];
			struct AdbWacomUD_R0 r0;
			struct AdbWacomUD_R1 r1;
		};

		byte datalen;
	};

	void setup();
	void initiateAdbTransfer(struct AdbPacket* adbPacket, void (*done_callback)(uint8_t errorCode));
}

#define ADB_COMMAND_RESETFLUSH 	0
#define ADB_COMMAND_LISTEN 	2
#define ADB_COMMAND_TALK 	3


#endif /* ADB_CODEC_H_ */
