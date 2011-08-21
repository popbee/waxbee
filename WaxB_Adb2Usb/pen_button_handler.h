/*
 * pen_button_handler.h
 *
 * Handles the state of a button driven by pen events.
 *
 * Created on: 2011-08-10
 *      Author: Bernard
 */

#ifndef PEN_BUTTON_HANDLER_H_
#define PEN_BUTTON_HANDLER_H_

#include "button_event.h"
#include "pen_event.h"

namespace PenButtonHandler
{
	void init();

	typedef struct
	{
		uint16_t x1;
		uint16_t y1;
		uint16_t x2;
		uint16_t y2;
		uint8_t buttonNumber;
	} ActiveArea;

	bool findPenButton(Pen::PenEvent& penEvent, ActiveArea& activeArea);

	void handle_pen_button_event(Pen::PenEvent& penEvent, ActiveArea& activeArea);
}

#endif /* PEN_BUTTON_HANDLER_H_ */
