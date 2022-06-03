/*
 * curtain_controller.h
 *
 * Created: 2022. 05. 30. 22:34:24
 *  Author: gabor
 */

#ifndef CURTAIN_CONTROLLER_H_
#define CURTAIN_CONTROLLER_H_

#include <stdio.h>
#include "utils.h"
#include "task_scheduler.h"
#include "button_controller.h"
#include "serial_controller.h"
#include "rtc_controller.h"
#include "motor_controller.h"
#include "lcd_controller.h"
#include "schedule_manager.h"
#include "menu.h"

#define BAUD_RATE 9600
#define MOTOR_SPEED_RPM 35

// Curtain fully closed detector sensor (reed switch)
#define Sensor_Closed_RDir    DDRC      // data direction register
#define Sensor_Closed_RPin    PINC      // input pin register
#define Sensor_Closed_RPort   PORTC     // output port register
#define Sensor_Closed_Pin     PC0       // pin number

// Curtain fully open detector sensor (reed switch)
#define Sensor_Open_RDir      DDRC      // data direction register
#define Sensor_Open_RPin      PINC      // input pin register
#define Sensor_Open_RPort     PORTC     // output port register
#define Sensor_Open_Pin       PC1       // pin number

bool direction = false;
bool sensorClosed = false, sensorOpen = false;

void CurtainController_RefreshScreen();
void CurtainController_ButtonChangeHandler(ButtonChange chg);
void CurtainController_DoCurtainAction(CurtainAction a);
void process_serial_input(char *line);
void check_reed_switches(uint8_t id);

void CurtainController_Init() {
    Buttons_Init(CurtainController_ButtonChangeHandler);
    Serial_Init(BAUD_RATE, process_serial_input);
    Motor_Init(MOTOR_SPEED_RPM);
    RTC_Init();
    LCD_Init();
    ScheduleManager_Init(CurtainController_DoCurtainAction);
    Menu_Init(CurtainController_DoCurtainAction);

    // set reed switch pins as input
    clear_bit(Sensor_Closed_RDir, Sensor_Closed_Pin);
    clear_bit(Sensor_Open_RDir, Sensor_Open_Pin);
    
    // enable reed switch pullup resistors
    set_bit(Sensor_Closed_RPort, Sensor_Closed_Pin);
    set_bit(Sensor_Open_RPort, Sensor_Open_Pin);

    // init task scheduler
    TaskScheduler_Init();
    
    // start task to check reed switches
    TaskScheduler_Schedule(250, 0, check_reed_switches);

    CurtainController_RefreshScreen();
}

void CurtainController_DoCurtainAction(CurtainAction a) {
    check_reed_switches(0);
    switch (a) {
        case CLOSE:
            if (sensorClosed) break;
            direction = false;
            Motor_SetDir(false);
            Motor_Enable();
            break;
        case OPEN:
            if (sensorOpen) break;
            direction = true;
            Motor_SetDir(true);
            Motor_Enable();
            break;
        case STOP:
            Motor_Disable();
            break;
    }
    // display the new state
    CurtainController_RefreshScreen();
}

void CurtainController_ButtonChangeHandler(ButtonChange chg) {
    if (chg.press) {
        StartMeasure();
    }
    if (chg.release) {
        printf("%d\n", GetMeasurement());
    }
    
    if (Motor_IsEnabled()) {
        // motor is running, override button change handling
        if (chg.press) {
            switch (chg.btn) {
                case LEFT:
                    CurtainController_DoCurtainAction(CLOSE);
                    break;
                case MIDDLE:
                    CurtainController_DoCurtainAction(STOP);
                    break;
                case RIGHT:
                    CurtainController_DoCurtainAction(OPEN);
                    break;
            }
        }
    } else {
        // call menu button handler
        Menu_ButtonChangeHandler(chg);
    }
}

void CurtainController_RefreshScreen() {
    if (Motor_IsEnabled()) {
        // motor is running, override screen refresh
        if (direction) {
            LCD_PrintStringAt("OPENING...      ", 0, 0);
        } else {
            LCD_PrintStringAt("CLOSING...      ", 0, 0);
        }
        LCD_PrintStringAt("\x7e\x7f    stop    \x7f\x7e", 1, 0);
    }
}

void check_reed_switches(uint8_t id) {
    sensorClosed = bit_is_clear(Sensor_Closed_RPin, Sensor_Closed_Pin);
    sensorOpen = bit_is_clear(Sensor_Open_RPin, Sensor_Open_Pin);

    if (sensorClosed && Motor_IsEnabled() && !direction) {
        CurtainController_DoCurtainAction(STOP);
    }

    if (sensorOpen && Motor_IsEnabled() && direction) {
        CurtainController_DoCurtainAction(STOP);
    }
}

void process_serial_input(char *line) {
    printf("\nUnknown command.\n\n");
}

#endif /* CURTAIN_CONTROLLER_H_ */