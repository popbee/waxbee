/*
 * debugproc.h
 *
 * Module to aid in debugging
 *
 *  Created on: 2011-07-23
 *      Author: Bernard
 */

#ifndef DEBUG_PROC_H_
#define DEBUG_PROC_H_

namespace DebugProc
{
	/** return true if debug processing is enabled */
	bool init();
	void proxOutTrigger();
	void processing();

	/** 5ms timer interrupt entry point */
	void timer_5ms_intr();
}

#endif // DEBUG_PROC_H_
