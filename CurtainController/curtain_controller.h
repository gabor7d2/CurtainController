/*
 * curtain_controller.h
 *
 * Created: 2022. 05. 30. 22:34:24
 *  Author: gabor
 */

#ifndef CURTAIN_CONTROLLER_H_
#define CURTAIN_CONTROLLER_H_

#include "utils.h"

#define MOTOR_SPEED_RPM 35

// Curtain fully closed detector sensor (reed switch)
#define Sensor_Closed_RDir    DDRC      // data direction register
#define Sensor_Closed_RPin    PINC      // input pin register
#define Sensor_Closed_RPort   PORTC     // output port register
#define Sensor_Closed_Pin     PC0       // pin number

// Curtain fully open detector sensor (reed switch)
#define Sensor_Open_RDir      DDRC      // data direction register
#define Sensor_Open_RPin      PINC      // input pin register
#define Sensor_Open_RPort     PORTC     // output port register
#define Sensor_Open_Pin       PC1       // pin number

/**
 * Init curtain controller. Initializes all modules.
 */
void CurtainController_Init();

#endif /* CURTAIN_CONTROLLER_H_ */