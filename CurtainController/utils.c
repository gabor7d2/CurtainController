#include "utils.h"

const char *day_name_short(uint8_t dayOfWeek) {
    switch (dayOfWeek) {
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

const char *day_name(uint8_t dayOfWeek) {
    switch (dayOfWeek) {
        case 0:
            return "Monday";
        case 1:
            return "Tuesday";
        case 2:
            return "Wednesday";
        case 3:
            return "Thursday";
        case 4:
            return "Friday";
        case 5:
            return "Saturday";
        case 6:
            return "Sunday";
    }
    return "Unknown";
}