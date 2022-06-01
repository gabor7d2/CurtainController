#include "utils.h"

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