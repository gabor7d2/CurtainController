#include <avr/io.h>
#include "button_controller.h"
#include "task_scheduler.h"

void update_button_states(uint8_t id);
void process_button_changes(uint8_t id);

void (*btn_change_handler)(ButtonChange);

void Buttons_Init(void (*handler)(ButtonChange change)) {
    // store function pointer
    btn_change_handler = handler;

    // set pins as input
    clear_bit(BTN1_RDir, BTN1_Pin);
    clear_bit(BTN2_RDir, BTN2_Pin);
    clear_bit(BTN3_RDir, BTN3_Pin);

    // activate internal pullup resistors (writing 1 to port bit activates it)
    set_bit(BTN1_RPort, BTN1_Pin);
    set_bit(BTN2_RPort, BTN2_Pin);
    set_bit(BTN3_RPort, BTN3_Pin);
    
    // init task scheduler
    TaskScheduler_Init();
    
    // start task for periodically checking buttons (to debounce)
    TaskScheduler_Schedule(250, 25, update_button_states);
    
    // start task for processing button changes continuously
    TaskScheduler_Schedule(250, 0, process_button_changes);
}

// latest state of buttons
volatile bool btn1 = false, btn2 = false, btn3 = false;

// measure how long the buttons have been pressed down for
volatile uint8_t cntbtn1 = 0, cntbtn2 = 0, cntbtn3 = 0;

volatile ButtonChange changeQueue[BTN_CHANGE_QUEUE_SIZE];
uint8_t idx0 = 0, idx1 = 0;

void add_change_to_queue(Button btn, bool press, bool release, bool repeat, bool longPress) {
    if (idx0 >= BTN_CHANGE_QUEUE_SIZE) idx0 = 0;
    changeQueue[idx0].btn = btn;
    changeQueue[idx0].press = press;
    changeQueue[idx0].release = release;
    changeQueue[idx0].repeat = repeat;
    changeQueue[idx0].longPress = longPress;
    idx0++;
}

/*
 * Function that gets called every 25 ms by the TaskScheduler
 * to update button states and add new changes to the queue
 */
void update_button_states(uint8_t id) {
    ////////////////////////////// BTN 1 ////////////////////////////////////
    // check if button has changed externally
    if (bit_is_clear(BTN1_RPin, BTN1_Pin) != btn1) {
        btn1 = !btn1;
        // add to queue
        add_change_to_queue(LEFT, btn1, !btn1, false, false);
    }

    // increase counter if pressed down
    if (btn1) cntbtn1++;
    else cntbtn1 = 0;

    // check if button needs to be repeated
    if (cntbtn1 > BTN_REPEAT_THRESHOLD && (cntbtn1 - BTN_REPEAT_THRESHOLD) % BTN_REPEAT_INTERVAL == 0) {
        add_change_to_queue(LEFT, false, false, true, false);
    }

    // check if button has reached long press threshold
    if (cntbtn1 == BTN_LONG_PRESS_THRESHOLD) {
        add_change_to_queue(LEFT, false, false, false, true);
    }

    ////////////////////////////// BTN 2 ////////////////////////////////////
    // check if button has changed externally
    if (bit_is_clear(BTN2_RPin, BTN2_Pin) != btn2) {
        btn2 = !btn2;
        // add to queue
        add_change_to_queue(MIDDLE, btn2, !btn2, false, false);
    }

    // increase counter if pressed down
    if (btn2) cntbtn2++;
    else cntbtn2 = 0;

    // check if button needs to be repeated
    if (cntbtn2 > BTN_REPEAT_THRESHOLD && (cntbtn2 - BTN_REPEAT_THRESHOLD) % BTN_REPEAT_INTERVAL == 0) {
        add_change_to_queue(MIDDLE, false, false, true, false);
    }

    // check if button has reached long press threshold
    if (cntbtn2 == BTN_LONG_PRESS_THRESHOLD) {
        add_change_to_queue(MIDDLE, false, false, false, true);
    }

    ////////////////////////////// BTN 3 ////////////////////////////////////
    // check if button has changed externally
    if (bit_is_clear(BTN3_RPin, BTN3_Pin) != btn3) {
        btn3 = !btn3;
        // add to queue
        add_change_to_queue(RIGHT, btn3, !btn3, false, false);
    }

    // increase counter if pressed down
    if (btn3) cntbtn3++;
    else cntbtn3 = 0;

    // check if button needs to be repeated
    if (cntbtn3 > BTN_REPEAT_THRESHOLD && (cntbtn3 - BTN_REPEAT_THRESHOLD) % BTN_REPEAT_INTERVAL == 0) {
        add_change_to_queue(RIGHT, false, false, true, false);
    }

    // check if button has reached long press threshold
    if (cntbtn3 == BTN_LONG_PRESS_THRESHOLD) {
        add_change_to_queue(RIGHT, false, false, false, true);
    }
}

/**
 * Processes the queued button changes, calling the
 * function registered in Buttons_Init() for each button change.
 */
void process_button_changes(uint8_t id) {
    while (idx1 != idx0) {
        if (idx1 >= BTN_CHANGE_QUEUE_SIZE) idx1 = 0;
        btn_change_handler(changeQueue[idx1++]);
    }
}