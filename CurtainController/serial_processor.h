/*
 * serial_processor.h
 *
 * Created: 2022. 06. 06. 1:43:06
 *  Author: gabor
 */ 

#ifndef SERIAL_PROCESSOR_H_
#define SERIAL_PROCESSOR_H_

#include "utils.h"

#define BAUD_RATE 9600

/**
 * Init serial processor. Initializes serial controller and stores the given handler.
 */
void SerialProcessor_Init(void (*handler)(CurtainAction));

#endif /* SERIAL_PROCESSOR_H_ */