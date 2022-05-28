/*
 * utils.h
 *
 * Created: 2022. 05. 18. 0:34:17
 *  Author: gabor
 */ 

#ifndef UTILS_H_
#define UTILS_H_

#include <avr/sfr_defs.h>
#include <stdbool.h>
#include "rtc3231.h"

#define set_bit(sfr, bit) (sfr |= _BV(bit))

#define clear_bit(sfr, bit) (sfr &= ~(_BV(bit)))

typedef struct CurtainSchedule {
	uint8_t daysAndType, hour, min;
} CurtainSchedule;

typedef struct SharedData {
	uint8_t state;
	bool running, direction;
	
	rtc_time currTime;
	uint8_t currDay;
	CurtainSchedule tempSchedule;
} SharedData;

const char *DayOfWeekToStr(uint8_t dayOfWeek) {
	switch(dayOfWeek) {
		case 0:
			return "Mo";
			case 1:
			return "Tu";
			case 2:
			return "We";
			case 3:
			return "Th";
			case 4:
			return "Fr";
			case 5:
			return "Sa";
			case 6:
			return "Su";
	}
	return "XX";
}

#endif /* UTILS_H_ */