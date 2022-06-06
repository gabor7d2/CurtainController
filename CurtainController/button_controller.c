#include "button_controller.h"
#include <avr/io.h>
#include "task_scheduler.h"

void update_button_states(uint8_t id);
void process_button_changes(uint8_t id);

void (*btn_change_handler)(ButtonChange);

void Buttons_Init(void (*handler)(ButtonChange change)) {
    // store function pointer
    btn_change_handler = handler;

    // set pins as input
    clear_bit(BTN_RDir, BTN1_Pin);
    clear_bit(BTN_RDir, BTN2_Pin);
    clear_bit(BTN_RDir, BTN3_Pin);

    // activate internal pullup resistors (writing 1 to port bit activates it)
    set_bit(BTN_RPort, BTN1_Pin);
    set_bit(BTN_RPort, BTN2_Pin);
    set_bit(BTN_RPort, BTN3_Pin);
    
    // init task scheduler
    TaskScheduler_Init();
    
    // start task for periodically checking buttons (to debounce)
    TaskScheduler_Schedule(250, 25, update_button_states);
    
    // start task for processing button changes continuously
    TaskScheduler_Schedule(250, 0, process_button_changes);
}

// latest state of buttons
volatile bool btnStates[3];

// measure how long the buttons have been pressed down for
volatile uint16_t btnCounters[3];

// whether to ignore button changes unless a press occurs for the particular button
volatile bool ignoreBtnChanges[3];

volatile ButtonChange changeQueue[BTN_CHANGE_QUEUE_SIZE];
volatile uint8_t idx0 = 0, idx1 = 0;

void Buttons_IgnoreBtnChanges(Button btn) {
    ignoreBtnChanges[btn] = true;
}

void Buttons_IgnoreAllBtnChanges() {
    Buttons_IgnoreBtnChanges(LEFT);
    Buttons_IgnoreBtnChanges(MIDDLE);
    Buttons_IgnoreBtnChanges(RIGHT);
}

void add_change_to_queue(Button btn, bool press, bool release, bool repeat, bool longPress) {
    if (ignoreBtnChanges[btn]) {
        if (press) ignoreBtnChanges[btn] = false;
        else return;
    }

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
    // go through the 3 buttons
    for (uint8_t i = 0; i < 3; i++) {
        // check if stored state differs from live state
        if (bit_is_clear(BTN_RPin, BTN_Pin(i)) != btnStates[i]) {
            btnStates[i] = !btnStates[i];
            // add to queue
            add_change_to_queue(i, btnStates[i], !btnStates[i], false, false);
        }

        // increase counter if pressed down
        if (btnStates[i]) btnCounters[i]++;
        else btnCounters[i] = 0;

        // check if button needs to be repeated
        if (btnCounters[i] > BTN_REPEAT_THRESHOLD && (btnCounters[i] - BTN_REPEAT_THRESHOLD) % BTN_REPEAT_INTERVAL == 0) {
            add_change_to_queue(i, false, false, true, false);
        }

        // check if button has reached long press threshold
        if (btnCounters[i] == BTN_LONG_PRESS_THRESHOLD) {
            add_change_to_queue(i, false, false, false, true);
        }
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