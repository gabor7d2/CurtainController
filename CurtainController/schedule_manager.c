#include <avr/eeprom.h>
#include "schedule_manager.h"
#include "task_scheduler.h"
#include "rtc_controller.h"
#include <stdio.h>

void check_schedules(uint8_t id);

CurtainSchedule schedules[POSSIBLE_CURTAIN_SCHEDULES];

// this is the nearest schedule, it only ever has 1 day set, which is the nearest day
// the original schedule would fire on.
CurtainSchedule nearestSchedule;

uint8_t scheduleCount = 0;

void (*curtain_action_handler)(CurtainAction);

void ScheduleManager_Init(void (*handler)(CurtainAction)) {
    curtain_action_handler = handler;

    // if there was no data saved before (checking for 0x4b at location 0x10), initialize eeprom storage
    if (eeprom_read_byte(EEPROM_UNIQUE_ID_ADDR) != EEPROM_UNIQUE_ID) {
        eeprom_write_byte(EEPROM_UNIQUE_ID_ADDR, EEPROM_UNIQUE_ID);
        eeprom_write_byte(EEPROM_SCHEDULE_COUNT_ADDR, 0);
    }

    // get schedule count from eeprom
    scheduleCount = eeprom_read_byte(EEPROM_SCHEDULE_COUNT_ADDR);

    // if schedule count is bigger than the max, set it to max
    if (scheduleCount > POSSIBLE_CURTAIN_SCHEDULES) scheduleCount = POSSIBLE_CURTAIN_SCHEDULES;

    // load schedules from eeprom
    for (uint8_t i = 0; i < scheduleCount; i++) {
        schedules[i].daysAndAction = eeprom_read_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * i);
        schedules[i].hour = eeprom_read_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * i + 1);
        schedules[i].min = eeprom_read_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * i + 2);
    }

    // init task scheduler
    TaskScheduler_Init();
    
    // start task that checks for current schedule status every 500 ms
    TaskScheduler_Schedule(250, 500, check_schedules);
}

uint8_t ScheduleManager_GetCount() {
    return scheduleCount;
}

CurtainSchedule ScheduleManager_GetNearest() {
    return nearestSchedule;
}

CurtainSchedule ScheduleManager_Get(uint8_t idx) {
    if (idx >= scheduleCount) {
        CurtainSchedule error = {0, 255, 255};
        return error;
    }
    return schedules[idx];
}

void ScheduleManager_Edit(uint8_t idx, CurtainSchedule newData) {
    if (idx >= scheduleCount) return;
    schedules[idx] = newData;
    
    // write to eeprom
    eeprom_write_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * idx, newData.daysAndAction);
    eeprom_write_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * idx + 1, newData.hour);
    eeprom_write_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * idx + 2, newData.min);
}

void ScheduleManager_Add(CurtainSchedule schedule) {
    if (scheduleCount >= POSSIBLE_CURTAIN_SCHEDULES) return;
    schedules[scheduleCount] = schedule;
    
    // write to eeprom
    eeprom_write_byte(EEPROM_SCHEDULE_COUNT_ADDR, scheduleCount + 1);
    eeprom_write_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * scheduleCount, schedule.daysAndAction);
    eeprom_write_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * scheduleCount + 1, schedule.hour);
    eeprom_write_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * scheduleCount + 2, schedule.min);
    scheduleCount++;
}

void ScheduleManager_Delete(uint8_t idx) {
    if (idx >= scheduleCount) return;

    // shift schedules down by 1 to "delete" the schedule
    for (uint8_t i = idx; i < scheduleCount - 1; i++) {
        schedules[idx] = schedules[idx + 1];
    }
    scheduleCount--;
    // write to eeprom
    eeprom_write_byte(EEPROM_SCHEDULE_COUNT_ADDR, scheduleCount);
}

/**
 * Search for the nearest next schedule and update nearestSchedule.
 */
void update_nearest_schedule() {
    uint16_t smallest = 30000;

    rtc_time time = RTC_GetTime();
    uint16_t currMin = time.min + time.hour * 60 + time.wday * 1440;

    for (uint8_t i = 0; i < scheduleCount; i++) {
        for (uint8_t j = 0; j < 7; j++) {
            // skip if schedule is inactive on this day
            if (!(schedules[i].daysAndAction & (1 << j))) continue;

            // calculate total minutes distance from start of week
            uint16_t totalMin = schedules[i].min + schedules[i].hour * 60 + j * 1440;

            // check if it is behind the current time, if it is,
            // add a week's worth of minutes to the total so that it will be further from current time
            if (time.sec == 0) {
                // if current sec is 0 and totalMin == currMin, we want this
                // to be the nearest schedule so that it will get fired in check_schedules
                if (totalMin < currMin) totalMin += 10080;
            } else {
                // if current sec is not 0 and totalMin == currMin, we want this
                // to be the furthest schedule so that it will not show on the screen anymore and not trigger curtain actions.
                if (totalMin <= currMin) totalMin += 10080;
            }

            // check if this is smaller than the smallest found yet
            if (totalMin < smallest) {
                smallest = totalMin;
                nearestSchedule = schedules[i];
                nearestSchedule.daysAndAction &= (0x80 | (1 << j));
            }
        }
    }
}

bool measureTime2 = false;

uint16_t t2, tf = 0;

void StartMeasure2() {
	measureTime2 = true;
    tf = 0;
}

uint8_t GetMeasurement2() {
	return tf;
}

/**
 * Gets called every 500 ms to check if a schedule is right now, and calls the curtain_action_handler if it is.
 */
void check_schedules(uint8_t id) {
    if (measureTime2) t2 = TCNT0;
    update_nearest_schedule();

    rtc_time time = RTC_GetTime();

    // check if nearest schedule's time matches current time
    if (nearestSchedule.daysAndAction & (1 << time.wday) && nearestSchedule.hour == time.hour && nearestSchedule.min == time.min) {
        curtain_action_handler((nearestSchedule.daysAndAction & 0x80) ? OPEN : CLOSE);
    }
    if (measureTime2) {
        tf = TCNT0 - t2;
        measureTime2 = false;
        printf("hello\n");
    }
}