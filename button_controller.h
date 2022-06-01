/*
 * button_controller.h
 *
 * Created: 2022. 05. 24. 19:35:55
 *  Author: gabor
 */

//////////////////////////////////////////////////////////////////////////
///
/// Button controller module
///
/// Polls the buttons every 25 ms with TaskScheduler, and calls the
/// registered handler function on every button change.
///
/// The 25 ms polling interval takes care of debouncing the switch too.
///
/// Call Buttons_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "utils.h"

#define BTN1_Dir	DDRD		// Button 1 data direction register
#define BTN1_Pin	PIND		// Button 1 input pin register
#define BTN1_Port	PORTD		// Button 1 output port register
#define BTN1		PD7			// Button 1 pin

#define BTN2_Dir	DDRD		// Button 2 data direction register
#define BTN2_Pin	PIND		// Button 2 input pin register
#define BTN2_Port	PORTD		// Button 2 output port register
#define BTN2		PD6			// Button 2 pin

#define BTN3_Dir	DDRD		// Button 3 data direction register
#define BTN3_Pin	PIND		// Button 3 input pin register
#define BTN3_Port	PORTD		// Button 3 output port register
#define BTN3		PD5			// Button 3 pin

/**
 * Initializes the button control logic, and registers the specified handler
 * that will be called on every button change.
 */
void Buttons_Init(void (*handler)(uint8_t id, bool pressed));

#endif /* BUTTONS_H_ */