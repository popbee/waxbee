/*
 * adb_tablet_defs.h
 *
 *  Created on: 2011-01-17
 *      Author: Bernard
 */

#ifndef ADB_TABLET_DEFS_H_
#define ADB_TABLET_DEFS_H_

namespace ADB
{
	struct AdbWacomUD_R0
	{
		struct
		{
			union
			{
				struct
				{
					unsigned button0:1;
					unsigned button1:1;
					unsigned button2:1;
					unsigned button3:1;
					unsigned button_down:1;
					unsigned is_stylus:1;
					unsigned proximity:1;
					unsigned always_one:1;
				};
				struct
				{
					unsigned buttoncode:4;
					unsigned spacer:4;
				};
				uint8_t	rawstatus;
			};
			uint8_t x_msb;
			uint8_t x_lsb;
			uint8_t y_msb;
			uint8_t y_lsb;
			int8_t pressure;
			int8_t tiltx;
			int8_t tilty;
		};
	};

	struct AdbWacomUD5_R0
	{
		struct
		{
			struct
			{
				unsigned x_msb:6;
				unsigned unknown:1; // always one
				unsigned proximity:1; // wild guess
			};
			uint8_t x_lsb;
			struct
			{
				unsigned y_msb:6;
				unsigned touch:1;
				unsigned button:1; // side button
			};
			uint8_t y_lsb;
			struct
			{
				unsigned pressure:7;
				unsigned unused:1;
			};
		};
	};

	struct AdbWacomUD_R1
	{
		struct
		{
			uint8_t unknown1;
			uint8_t unknown2;
			uint8_t max_x_msb;
			uint8_t max_x_lsb;
			uint8_t max_y_msb;
			uint8_t max_y_lsb;
			uint8_t unknown3;
			uint8_t unknown4;
		};
	};
}


#endif /* ADB_TABLET_DEFS_H_ */
