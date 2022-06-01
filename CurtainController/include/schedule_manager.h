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
/// and calculating the nearest schedule that is going to fire.
///
/// Calls the registered handler function when a schedule is fired to initiate curtain opening/closing.
///
/// Call ScheduleManager_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef SCHEDULE_MANAGER_H_
#define SCHEDULE_MANAGER_H_

#include "utils.h"

#define POSSIBLE_CURTAIN_SCHEDULES 10

#define EEPROM_UNIQUE_DATA 0x4b
#define EEPROM_UNIQUE_DATA_ADDR (uint8_t*) 0x10
#define EEPROM_SCHEDULE_COUNT_ADDR (uint8_t*) 0x11
#define EEPROM_SCHEDULE_ARRAY_BASE_ADDR (uint8_t*) 0x12

/**
 * Init schedule manager, load schedules from EEPROM.
 */
void ScheduleManager_Init(void (*handler)(CurtainAction));

/**
 * Get how many schedules are currently registered.
 */
uint8_t ScheduleManager_GetScheduleCount();

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