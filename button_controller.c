#include <avr/io.h>
#include "button_controller.h"
#include "task_scheduler.h"

bool update_button_states(uint8_t id);
bool process_button_states(uint8_t id);

void (*btn_change_handler)(uint8_t id, bool pressed);

void Buttons_Init(void (*handler)(uint8_t id, bool pressed)) {
    // store function pointer
    btn_change_handler = handler;

    // set pins as input
    clear_bit(BTN1_Dir, BTN1);
    clear_bit(BTN2_Dir, BTN2);
    clear_bit(BTN3_Dir, BTN3);

    // activate internal pullup resistors (writing 1 to port bit activates it)
    set_bit(BTN1_Port, BTN1);
    set_bit(BTN2_Port, BTN2);
    set_bit(BTN3_Port, BTN3);
    
    // init task scheduler
    TaskScheduler_Init();
    
    // start task for periodically checking buttons (to debounce)
    TaskScheduler_Schedule(10, 25, update_button_states);
    
    // start task for processing button changes continuously
    TaskScheduler_Schedule(11, 0, process_button_states);
}

// latest state of buttons
volatile bool btn1 = false, btn2 = false, btn3 = false;
// track whether state of buttons have changed since processing the changes
volatile bool chgbtn1 = false, chgbtn2 = false, chgbtn3 = false;

/*
 * Function that gets called every 25 ms by the TaskScheduler to update button states.
 */
bool update_button_states(uint8_t id) {
    if (bit_is_clear(BTN1_Pin, BTN1) != btn1) {
        btn1 = !btn1;
        chgbtn1 = true;
    }

    if (bit_is_clear(BTN2_Pin, BTN2) != btn2) {
        btn2 = !btn2;
        chgbtn2 = true;
    }

    if (bit_is_clear(BTN3_Pin, BTN3) != btn3) {
        btn3 = !btn3;
        chgbtn3 = true;
    }

    // keep task running
    return true;
}

/**
 * Processes the queued button changes, calling the
 * function registered in Buttons_Init() for each button change.
 */
bool process_button_states(uint8_t id) {
    if (chgbtn1) {
        btn_change_handler(1, btn1);
        chgbtn1 = false;
    }
    if (chgbtn2) {
        btn_change_handler(2, btn2);
        chgbtn2 = false;
    }
    if (chgbtn3) {
        btn_change_handler(3, btn3);
        chgbtn3 = false;
    }
    
    // keep task running
    return true;
}