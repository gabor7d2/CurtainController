/*
* drv8825.h
*
* Created: 2022. 05. 03. 9:31:52
*  Author: gabor
*/

//////////////////////////////////////////////////////////////////////////
// 
/// Controller for stepper motor driver board DRV8825
/// https://www.ti.com/lit/ds/symlink/drv8825.pdf
/// https://caxtool.hu/custom/mysticnails/image/data/Webshop/le%C3%ADr%C3%A1s/0J4232.600.png
///
/// Uses the specified port bits of the specified port for the board's
/// Enable, Step and Direction signals.
///
/// Call Motor_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef DRV8825_H
#define DRV8825_H

#include <stdbool.h>
#include <avr/io.h>
#include "utils.h"

#define Motor_Dir	DDRD		// DRV8825 data direction register
#define Motor_Port	PORTD		// DRV8825 data output register
//#define Motor_Pin	PIND		// DRV8825 data input register (used for toggling)
#define Motor_EN	PD4			// DRV8825 enable pin
#define Motor_STEP  PD3			// DRV8825 step pin
#define Motor_DIR	PD2			// DRV8825 dir pin

int delayUs = 10000;

/**
* Initialize motor pins and disable motor.
*/
void Motor_Init() {
	// Set direction as output
	set_bit(Motor_Dir, Motor_EN);
	set_bit(Motor_Dir, Motor_STEP);
	set_bit(Motor_Dir, Motor_DIR);
	
	// Disable motor
	set_bit(Motor_Port, Motor_EN);
}

/**
* Enable motor.
*/
void Motor_Enable() {
	clear_bit(Motor_Port, Motor_EN);
}

/**
* Disable motor.
*/
void Motor_Disable() {
	set_bit(Motor_Port, Motor_EN);
}

/**
* Get if motor is enabled.
*/
bool Motor_IsEnabled() {
	return bit_is_clear(Motor_Port, Motor_EN);
}

/**
* Set motor spinning direction.
*/
void Motor_SetDir(bool dir) {
	if (dir)
		set_bit(Motor_Port, Motor_DIR);
	else
		clear_bit(Motor_Port, Motor_DIR);
}

/**
* Reverse motor spinning direction.
*/
void Motor_ReverseDir() {
	//Motor_Pin |= Motor_DIR;
	if (bit_is_set(Motor_Port, Motor_DIR)) Motor_SetDir(false);
	else Motor_SetDir(true);
}

/**
* Set relative motor speed.
*/
void Motor_SetSpeed(double speed) {
	delayUs = 10000 / speed;
}

/**
* Do one step of the motor, if it is enabled.
* Should be called in the main loop continuously.
*/
void Motor_Step() {
	if (!Motor_IsEnabled()) return;
	
	set_bit(Motor_Port, Motor_STEP);
	_delay_us(delayUs);
	clear_bit(Motor_Port, Motor_STEP);
	_delay_us(delayUs);
}

#endif