/*
 * utils.h
 *
 * Created: 2022. 05. 18. 0:34:17
 *  Author: gabor
 */ 

#ifndef UTILS_H_
#define UTILS_H_

#define F_CPU 16000000
#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/sfr_defs.h>
#include <stdbool.h>

#define set_bit(sfr, bit) (sfr |= _BV(bit))

#define clear_bit(sfr, bit) (sfr &= ~(_BV(bit)))

typedef enum CurtainAction {
    OPEN, CLOSE, STOP
} CurtainAction;

/**
 * Stores a curtain closing/opening schedule.
 */
typedef struct CurtainSchedule {
    // Bit 0 is monday, 1 is tuesday ... 6 is sunday, bit 7 is action (0 == close, 1 == open)
	uint8_t daysAndAction;
    // hour and minute of schedule
    uint8_t hour, min;
} CurtainSchedule;

typedef struct CommonData {
    
} CommonData;

/**
 * Returns a 2 character day name of the give day of week number.
 * @param dayOfWeek Number between 0 and 6 (Monday to Sunday)
 * @return The 2 character name of the day, or "XX" if dayOfWeek isn't between 0 and 6.
 */
const char *DayOfWeekToStr(uint8_t dayOfWeek);

#endif /* UTILS_H_ */