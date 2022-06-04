/*
 * schedule_manager.h
 *
 * Created: 2022. 05. 31. 2:02:47
 *  Author: gabor
 */

//////////////////////////////////////////////////////////////////////////
///
/// Schedule Manager module
///
/// Module for managing schedules that are persisted in the built-in EEPROM,
/// and calculating the nearest schedule that is going to fire using TaskScheduler.
///
/// Calls the registered handler function when a schedule is fired to initiate curtain opening/closing.
///
/// Call ScheduleManager_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef SCHEDULE_MANAGER_H_
#define SCHEDULE_MANAGER_H_

#include "utils.h"

// how many curtain schedules can be scheduled
#define POSSIBLE_CURTAIN_SCHEDULES 3

#define EEPROM_UNIQUE_ID 0x4b                               // the unique id that identifies if we have previously stored data
#define EEPROM_UNIQUE_ID_ADDR (uint8_t*) 0x10               // address of unique identifier
#define EEPROM_SCHEDULE_COUNT_ADDR (uint8_t*) 0x11          // address of amount of schedules stored
#define EEPROM_SCHEDULE_ARRAY_BASE_ADDR (uint8_t*) 0x12     // address of start of schedule array

void StartMeasure2();

uint8_t GetMeasurement2();

/**
 * Stores a curtain closing/opening schedule.
 */
typedef struct CurtainSchedule {
    // Bit 0 is monday, 1 is tuesday ... 6 is sunday, bit 7 is action (0 == close, 1 == open)
	uint8_t daysAndAction;
    // hour and minute of schedule
    uint8_t hour, min;
} CurtainSchedule;

/**
 * Init schedule manager, load schedules from EEPROM.
 */
void ScheduleManager_Init(void (*handler)(CurtainAction));

/**
 * Get how many schedules are currently registered.
 */
uint8_t ScheduleManager_GetCount();

/**
 * Get the nearest schedule. This schedule only ever has 1 day set, which is
 * the nearest day the original schedule would fire on.
 */
CurtainSchedule ScheduleManager_GetNearest();

/**
 * Get the schedule at the specified index.
 */
CurtainSchedule ScheduleManager_Get(uint8_t idx);

/**
 * Edit a schedule. Saves the new schedule data to the EEPROM.
 */
void ScheduleManager_Edit(uint8_t idx, CurtainSchedule newData);

/**
 * Add a new schedule. Saves the new schedule to the EEPROM.
 */
void ScheduleManager_Add(CurtainSchedule schedule);

/**
 * Delete the schedule at the specified index. Saves the change to the EEPROM.
 */
void ScheduleManager_Delete(uint8_t idx);

#endif /* SCHEDULE_MANAGER_H_ */