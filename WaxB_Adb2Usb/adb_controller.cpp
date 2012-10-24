
/*
 * ADB Tablet device protocol controller
 *
 * This emulates an ADB host talking to an ADB Tablet device.
 *
 * NOTE: Only meant to work with a single ADB device on the bus.
 */

//---------------------------------------------------------
// Usage:
//
// ADB::adb_controller() needs to be called repetitively.
//
//---------------------------------------------------------

#include <avr/interrupt.h>
#include "led.h"
#include "avr_util.h"
#include "console.h"
#include "adb_codec.h"
#include "adb_controller.h"
#include "wacom_usb.h"
#include "strings.h"

#include <util/delay.h>

//uncomment the following to perform some ADB analysis for GD tablets
//#define GD_TRYOUTS

namespace ADB
{
	ADB::AdbPacket adbPacket;
	Pen::PenEvent penEvent;
	uint8_t adbLastErrorStatus = 0;

	// true when the adb controller has work to do
	bool adbTransactionPending = true;
	bool adbTransactionJustFinished = false;

	void adb_transaction_done(uint8_t errorCode)
	{
		// watch that we are running in interrupt mode here

		// keep the error code
		adbLastErrorStatus = errorCode;

		// flag that we have been called back
		adbTransactionJustFinished = true;

		// trig adb controller
		adbTransactionPending = true;
	}

	void dump_adb_packet(bool newline)
	{
		console::print("[");
		console::printHex(adbPacket.address, 1);
		if(adbPacket.command == ADB_COMMAND_TALK)
			console::print('T');
		else if(adbPacket.command == ADB_COMMAND_LISTEN)
			console::print('L');
		else
			console::print('X');

		console::printHex(adbPacket.parameter, 1);
		if(adbPacket.datalen > 0)
		{
			console::print(':');
			for(int i=0; i<adbPacket.datalen; i++)
			{
				console::printHex(adbPacket.data[i], 2);
			}
		}

		console::print(']');
		if(newline)
			console::println();
	}

	static void dump_adb_event()
	{
		if(!console::console_enabled)
			return;

		uint16_t x = (adbPacket.r0.x_msb << 8) | adbPacket.r0.x_lsb;
		uint16_t y = (adbPacket.r0.y_msb << 8) | adbPacket.r0.y_lsb;

		console::print(" ADB Event: ");
		console::printBinaryByte(adbPacket.r0.rawstatus);
		console::print("  status=1:");
		console::printbit(adbPacket.r0.always_one);
		console::print(" p:");
		console::printbit(adbPacket.r0.proximity);
		console::print(" s:");
		console::printbit(adbPacket.r0.is_stylus);
		console::print(" down:");
		console::printbit(adbPacket.r0.button_down);
		console::print(" b3:");
		console::printbit(adbPacket.r0.button3);
		console::print(" b2:");
		console::printbit(adbPacket.r0.button2);
		console::print(" b1:");
		console::printbit(adbPacket.r0.button1);
		console::print(" b0:");
		console::printbit(adbPacket.r0.button0);
		console::print(", x=");
		console::printNumber(x);
		console::print(", y=");
		console::printNumber(y);
		console::print(", pressure=");
		console::printNumber(adbPacket.r0.pressure);
		console::print(", tilt_x=");
		console::printNumber(adbPacket.r0.tiltx);
		console::print(", tilt_y=");
		console::printNumber(adbPacket.r0.tilty);
		console::println();
	}

	enum adbControllerState
	{
		poweron,
		identify,  	// [4T1:384850003C000617]
		identify_response,
		_4L3, 	   	// [4L3:6F68]
		_4L3_response,
		_4T3,		// [4T3:6968]
		_4T3_response,
		_4L1,		// [4L1:6768]
		_4L1_response,
		_4T1_check,	// [4T1:386B50003C000617]
		_4T1_check_response,
#ifdef GD_TRYOUTS
		_4L2_538C,
		_4L2_538C_response,
		_4L2_308C,
		_4L2_308C_response,
#endif
		status,		// [4T0:800000003CFF0000], [4T0]
		status_response
#ifdef GD_TRYOUTS
		,
		status2,
		status_response2
#endif
	};

	adbControllerState itsCurAdbState = poweron;

	static inline void debug_pulse()
	{
		sbi(PORTC,6);
		_delay_us(1);
		cbi(PORTC,6);
	}

	bool in_proximity;
	bool eraser_mode;

	void resetToolState()
	{
		in_proximity = false;
		eraser_mode = false;
	}

	void detectToolState()
	{
		if(!in_proximity)
		{
			if(adbPacket.r0.proximity)
			{
				// entering proximity
				in_proximity = true;

				if(adbPacket.r0.buttoncode == 4 ||
				   adbPacket.r0.buttoncode == 5)
				{
					eraser_mode = true;
				}
				else
				{
					eraser_mode = false;
				}

				console::printP(STR_ENTERING_PROXIMITY_MODE);
				if(eraser_mode)
					console::printlnP(STR_ERASER);
				else
					console::printlnP(STR_PEN);
			}
		}
		else if(!adbPacket.r0.proximity)
		{
			console::printlnP(STR_EXITING_PROXIMITY);
			resetToolState();
		}
	}

	bool adb_controller_internal()
	{
		if(!adbTransactionPending)
			return true;

		adbTransactionPending = false;

		if(adbTransactionJustFinished)
		{
			adbTransactionJustFinished = false;

			if(!adbLastErrorStatus == 0)
			{
				console::printP(STR_ERROR_CODE);
				console::printNumber(adbLastErrorStatus);
				console::println();
			}
		}

		switch(itsCurAdbState)
		{
			case poweron:
				console::printlnP(STR_POWERON);
				resetToolState();
			case identify:
				debug_pulse();
				console::printlnP(STR_IDENTIFY);
				// [4T1:384850003C000617]
				adbPacket.address = 4;
				adbPacket.command = ADB_COMMAND_TALK;
				adbPacket.parameter = 1;
				adbPacket.datalen = 0;

				ADB::initiateAdbTransfer(&adbPacket, adb_transaction_done);

				itsCurAdbState = identify_response;
				break;

			case identify_response:
			{
				console::printlnP(STR_IDENTIFY_RESPONSE);
				dump_adb_packet(true);

				if(adbLastErrorStatus != 0 || adbPacket.datalen == 0)
				{
					itsCurAdbState = identify;
					return false; // re-enter immediately the controller
				}

				uint16_t max_x = (adbPacket.r1.max_x_msb << 8) | (adbPacket.r1.max_x_lsb);
				uint16_t max_y = (adbPacket.r1.max_y_msb << 8) | (adbPacket.r1.max_y_lsb);

				console::printP(STR_ADB_TABLET_INFO_MAX_X);
				console::printNumber(max_x);
				console::printP(STR_MAX_Y);
				console::printNumber(max_y);
				console::println();
			}
			case _4L3:
				console::printlnP(STR_4L3);
				// [4L3:6F68]

				adbPacket.address = 4;
				adbPacket.command = ADB_COMMAND_LISTEN;
				adbPacket.parameter = 3;
				adbPacket.datalen = 2;
				adbPacket.data[0] = 0x6F;
				adbPacket.data[1] = 0x68;

				ADB::initiateAdbTransfer(&adbPacket, adb_transaction_done);

				itsCurAdbState = _4L3_response;
				break;

			case _4L3_response:
				console::printlnP(STR_4L3_RESPONSE);
				dump_adb_packet(true);
				if(adbLastErrorStatus != 0)
				{
					itsCurAdbState = _4L3;
					return false; // re-enter immediately the controller
				}

			case _4T3:
				console::printlnP(STR_4T3);
				// [4T3:6968]

				adbPacket.address = 4;
				adbPacket.command = ADB_COMMAND_TALK;
				adbPacket.parameter = 3;
				adbPacket.datalen = 0;

				ADB::initiateAdbTransfer(&adbPacket, adb_transaction_done);

				itsCurAdbState = _4T3_response;
				break;

			case _4T3_response:
				console::printlnP(STR_4T3_RESPONSE);
				dump_adb_packet(true);
				if(adbLastErrorStatus != 0 || adbPacket.datalen == 0)
				{
					itsCurAdbState = _4T3;
					return false; // re-enter immediately the controller
				}

			case _4L1:
				console::printlnP(STR_4L1);
				// [4L1:6768]

				adbPacket.address = 4;
				adbPacket.command = ADB_COMMAND_LISTEN;
				adbPacket.parameter = 1;
				adbPacket.datalen = 2;
				adbPacket.data[0] = 0x67;
				adbPacket.data[1] = 0x68;

				ADB::initiateAdbTransfer(&adbPacket, adb_transaction_done);

				itsCurAdbState = _4L1_response;
				break;

			case _4L1_response:
				console::printlnP(STR_4L1_RESPONSE);
				dump_adb_packet(true);
				if(adbLastErrorStatus != 0)
				{
					itsCurAdbState = _4L1;
					return false; // re-enter immediately the controller
				}

			case _4T1_check:
				console::printlnP(STR_4T1_CHECK);
				// [4T1:386B50003C000617]

				adbPacket.address = 4;
				adbPacket.command = ADB_COMMAND_TALK;
				adbPacket.parameter = 1;
				adbPacket.datalen = 0;

				ADB::initiateAdbTransfer(&adbPacket, adb_transaction_done);

				itsCurAdbState = _4T1_check_response;
				break;

			case _4T1_check_response:
				console::printlnP(STR_4T1_CHECK_RESPONSE);
				dump_adb_packet(true);
				if(adbLastErrorStatus != 0 || adbPacket.datalen == 0)
				{
					itsCurAdbState = _4T1_check;
					return false; // re-enter immediately the controller
				}
#ifdef GD_TRYOUTS
			case _4L2_538C:
				adbPacket.address = 4;
				adbPacket.command = ADB_COMMAND_LISTEN;
				adbPacket.parameter = 2;
				adbPacket.datalen = 2;
				adbPacket.data[0] = 0x53;
				adbPacket.data[1] = 0x8C;

				ADB::initiateAdbTransfer(&adbPacket, adb_transaction_done);

				itsCurAdbState = _4L2_538C_response;
				break;

			case _4L2_538C_response:
				dump_adb_packet(true);
				if(adbLastErrorStatus != 0)
				{
					itsCurAdbState = _4L2_538C;
					return false; // re-enter immediately the controller
				}

			case _4L2_308C:
				adbPacket.address = 4;
				adbPacket.command = ADB_COMMAND_LISTEN;
				adbPacket.parameter = 2;
				adbPacket.datalen = 2;
				adbPacket.data[0] = 0x30;
				adbPacket.data[1] = 0x8C;

				ADB::initiateAdbTransfer(&adbPacket, adb_transaction_done);

				itsCurAdbState = _4L2_308C_response;
				break;

			case _4L2_308C_response:
				dump_adb_packet(true);
				if(adbLastErrorStatus != 0)
				{
					itsCurAdbState = _4L2_308C;
					return false; // re-enter immediately the controller
				}

#endif
			case status:
//				console::println("-- status");
				// [4T0]

				adbPacket.address = 4;
				adbPacket.command = ADB_COMMAND_TALK;
				adbPacket.parameter = 0;
				adbPacket.datalen = 0;

				ADB::initiateAdbTransfer(&adbPacket, adb_transaction_done);

				itsCurAdbState = status_response;
				break;

			case status_response:
			{
//				console::println("-- status_response");

				if(adbLastErrorStatus != 0)
				{
					itsCurAdbState = status;
					return false; // re-enter immediately the controller
				}
#ifdef GD_TRYOUTS
				dump_adb_packet(true);
#endif
				if(adbPacket.datalen == 0)
				{
					itsCurAdbState = status;
					return false; // re-enter immediately the controller
				}
#ifndef GD_TRYOUTS
				dump_adb_packet(false);
				dump_adb_event();
#endif
				detectToolState();

				// detect eraser mode
				// first time the pen comes into proximity,
				// check if button2 is set.

				penEvent.proximity = adbPacket.r0.proximity;
				penEvent.touch = adbPacket.r0.button0;

				penEvent.eraser = eraser_mode;
				if(eraser_mode)
				{
					// side buttons inactive in eraser mode,
					// so hard-code them in the pen event.
					penEvent.button0 = 0;
					penEvent.button1 = 0;
				}
				else
				{
					penEvent.button0 = adbPacket.r0.button1;
					penEvent.button1 = adbPacket.r0.button2;
				}

				penEvent.is_mouse = !adbPacket.r0.is_stylus;

				penEvent.pressure = adbPacket.r0.pressure + 128;

				penEvent.x = (adbPacket.r0.x_msb << 8) | adbPacket.r0.x_lsb;
				penEvent.y = (adbPacket.r0.y_msb << 8) | adbPacket.r0.y_lsb;

				penEvent.tilt_x = adbPacket.r0.tiltx;
				penEvent.tilt_y = adbPacket.r0.tilty;

				penEvent.rotation_z = 0;

				Pen::input_pen_event(penEvent);

				// right after proximity == 0, send a nullified packet.

				itsCurAdbState = status;
				return false; // re-enter immediately the controller
			}
#ifdef GD_TRYOUTS			
			case status2:
//				console::println("-- status");
				// [4T2]

				adbPacket.address = 4;
				adbPacket.command = ADB_COMMAND_TALK;
				adbPacket.parameter = 1;
				adbPacket.datalen = 0;

				ADB::initiateAdbTransfer(&adbPacket, adb_transaction_done);

				itsCurAdbState = status_response2;
				break;

			case status_response2:
			{
				if(adbLastErrorStatus != 0)
				{
					itsCurAdbState = status;
					return false; // re-enter immediately the controller
				}

				dump_adb_packet(true);

				itsCurAdbState = status;
				return false; // re-enter immediately the controller
			}
#endif
		}

		// done for now
		return true;
	}

	void adb_controller()
	{
		if(!adb_controller_internal())
		{
			// shortcut to be called on the next processing loop
			adbTransactionPending = true;
		}
	}
}
