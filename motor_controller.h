/*
* motor_controller.h
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
/// Automatically steps the motor with the set speed, using TIMER4.
/// Using a timer and interrupts for stepping makes sure that the motor's
/// speed is always constant and accurate.
///
/// Call Motor_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef DRV8825_H
#define DRV8825_H

#include "utils.h"

#define Motor_Dir	DDRD		// DRV8825 data direction register
#define Motor_Port	PORTD		// DRV8825 data output register
#define Motor_EN	PD4			// DRV8825 enable pin
#define Motor_STEP  PD3			// DRV8825 step pin
#define Motor_DIR	PD2			// DRV8825 dir pin

/**
* Initialize motor pins, set motor speed and disable motor.
*/
void Motor_Init(double rpm);

/**
* Enable motor.
*/
void Motor_Enable();

/**
* Disable motor.
*/
void Motor_Disable();

/**
* Get if motor is enabled.
*/
bool Motor_IsEnabled();

/**
* Set motor spinning direction.
*/
void Motor_SetDir(bool dir);

/**
* Reverse motor spinning direction.
*/
void Motor_ReverseDir();

/**
* Set motor speed using rpm value.
*/
void Motor_SetSpeed(double rpm);

#endif