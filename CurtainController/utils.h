/*
 * utils.h
 *
 * Created: 2022. 05. 18. 0:34:17
 *  Author: gabor
 */ 

#ifndef UTILS_H_
#define UTILS_H_

#define F_CPU 16000000UL
#define __DELAY_BACKWARD_COMPATIBLE__

#define FIRMWARE_VERSION_MAJOR 2
#define FIRMWARE_VERSION_MINOR 0

#include <avr/sfr_defs.h>
#include <stdbool.h>

#define set_bit(sfr, bit) (sfr |= _BV(bit))

#define clear_bit(sfr, bit) (sfr &= ~(_BV(bit)))

typedef enum CurtainAction {
    OPEN, CLOSE, STOP
} CurtainAction;

#endif /* UTILS_H_ */