#include <avr/eeprom.h>
#include "schedule_manager.h"
#include "task_scheduler.h"
#include "rtc_controller.h"

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
    if (eeprom_read_byte(EEPROM_UNIQUE_DATA_ADDR) != EEPROM_UNIQUE_DATA) {
        eeprom_write_byte(EEPROM_UNIQUE_DATA_ADDR, EEPROM_UNIQUE_DATA);
        eeprom_write_byte(EEPROM_SCHEDULE_COUNT_ADDR, 0);
    }

    // load schedules from eeprom
    scheduleCount = eeprom_read_byte(EEPROM_SCHEDULE_COUNT_ADDR);
    for (int8_t i = scheduleCount - 1; i >= 0; i--) {
        schedules[i].daysAndAction = eeprom_read_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * i);
        schedules[i].hour = eeprom_read_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * i + 1);
        schedules[i].min = eeprom_read_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * i + 2);
    }

    // init task scheduler
    TaskScheduler_Init();
    
    // start task that checks for current schedule status every 500 ms
    TaskScheduler_Schedule(250, 500, check_schedules);
}

uint8_t ScheduleManager_GetScheduleCount() {
    return scheduleCount;
}

CurtainSchedule ScheduleManager_GetNearest() {
    return nearestSchedule;
}

CurtainSchedule ScheduleManager_Get(uint8_t idx) {
    return schedules[idx];
}

void ScheduleManager_Edit(uint8_t idx, CurtainSchedule newData) {
    schedules[idx] = newData;
    
    // write to eeprom
    eeprom_write_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * idx, newData.daysAndAction);
    eeprom_write_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * idx + 1, newData.hour);
    eeprom_write_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * idx + 2, newData.min);
}

void ScheduleManager_Add(CurtainSchedule schedule) {
    schedules[scheduleCount] = schedule;
    
    // write to eeprom
    eeprom_write_byte(EEPROM_SCHEDULE_COUNT_ADDR, scheduleCount + 1);
    eeprom_write_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * scheduleCount, schedule.daysAndAction);
    eeprom_write_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * scheduleCount + 1, schedule.hour);
    eeprom_write_byte(EEPROM_SCHEDULE_ARRAY_BASE_ADDR + 3 * scheduleCount + 2, schedule.min);
    scheduleCount++;
}

void ScheduleManager_Delete(uint8_t idx) {
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
    uint16_t smallest = 15000;

    rtc_date date = RTC_GetDate();
    rtc_time time = RTC_GetTime();
    uint16_t currMin = time.min + time.hour * 60 + date.wday * 1440;

    for (uint8_t i = 0; i < scheduleCount; i++) {
        for (uint8_t j = 0; j < 7; j++) {
            // skip if schedule is inactive on this day
            if (!(schedules[i].daysAndAction & (1 << j))) continue;
            uint16_t totalMin = schedules[i].min + schedules[i].hour * 60 + j * 1440;
            if (totalMin < currMin) totalMin += 10080;
            if (totalMin < smallest) {
                smallest = totalMin;
                nearestSchedule = schedules[i];
                nearestSchedule.daysAndAction &= (0x80 | (1 << j));
            }
        }
    }
}

/**
 * Gets called every 500 ms to check if a schedule is right now, and calls the curtain_action_handler if it is.
 */
void check_schedules(uint8_t id) {
    update_nearest_schedule();

    rtc_date date = RTC_GetDate();
    rtc_time time = RTC_GetTime();

    if (nearestSchedule.daysAndAction & (1 << date.wday) && nearestSchedule.hour == time.hour && nearestSchedule.min == time.min && time.sec == 0) {
        curtain_action_handler((nearestSchedule.daysAndAction & 0x80) ? OPEN : CLOSE);
    }
}