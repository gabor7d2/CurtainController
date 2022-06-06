#include "serial_controller.h"
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "task_scheduler.h"

#ifndef F_CPU
#error "F_CPU needs to be defined and set to the frequency of the CPU!"
#endif

//////////////////////////////////////////////////////////////////////////
/// INIT
//////////////////////////////////////////////////////////////////////////

int printf_putchar(char c, FILE *stream);

void process_received_chars(uint8_t id);

static FILE mystdout = FDEV_SETUP_STREAM(printf_putchar, NULL, _FDEV_SETUP_WRITE);

void (*line_received_handler)(char *line);

void Serial_Init(uint16_t baud, void (*handler)(char *line)) {
    // store function pointer
    line_received_handler = handler;

    // set baud rate
    uint16_t ubrr = F_CPU / 16 / baud - 1;
    UBRR0H = (uint8_t) (ubrr >> 8);
    UBRR0L = (uint8_t) ubrr;

    // enable usart rx, tx, and receive interrupts
    set_bit(UCSR0B, RXEN0);
    set_bit(UCSR0B, TXEN0);
    set_bit(UCSR0B, RXCIE0);

    // setup stdout
    stdout = &mystdout;
    
    // init task scheduler
    TaskScheduler_Init();
    
    // start task for processing received chars and lines continuously
    TaskScheduler_Schedule(250, 0, process_received_chars);
}

//////////////////////////////////////////////////////////////////////////
/// DATA RECEIVE
//////////////////////////////////////////////////////////////////////////

char recvBuf[SERIAL_RECEIVE_BUFFER_SIZE];
uint8_t idxR0 = 0, idxR1 = 0;
bool unread = false;

char recvLine[SERIAL_RECEIVE_LINE_BUFFER_SIZE];
uint8_t lineidx = 0;
bool hasNewLine = false;

/**
 * Interrupt vector to handle received bytes.
 */
ISR(USART0_RX_vect) {
    uint8_t c = UDR0;
    if (bit_is_clear(UCSR0A, FE0)) {
        if (idxR0 >= SERIAL_RECEIVE_BUFFER_SIZE) idxR0 = 0;
        recvBuf[idxR0++] = c;
        unread = true;
    }
}

bool Serial_HasUnread() {
    return unread;
}

char Serial_Read() {
    if (idxR1 >= SERIAL_RECEIVE_BUFFER_SIZE) idxR1 = 0;
    if (unread) {
        char c = recvBuf[idxR1++];
        if (idxR0 == idxR1) unread = false;
        return c;
    } else {
        return -1;
    }
}

/**
 * Processes the received characters, and calls the function registered
 * in Serial_Init() if an entire line has accumulated.
 */
void process_received_chars(uint8_t id) {
    while (Serial_HasUnread()) {
        if (lineidx >= SERIAL_RECEIVE_LINE_BUFFER_SIZE) lineidx = 0;
        char c = Serial_Read();
        
        // ignore CR
        if (c == '\r') continue;
        // detect LF and put a null-terminator into the array
        if (c == '\n') {
            hasNewLine = true;
            recvLine[lineidx++] = '\0';
            break;
        }
        recvLine[lineidx++] = c;
    }

    if (hasNewLine) {
        line_received_handler(recvLine);
        hasNewLine = false;
        lineidx = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
/// DATA SEND
//////////////////////////////////////////////////////////////////////////

void Serial_PrintChar(char c) {
    // wait until USART data register becomes empty.
    while (bit_is_clear(UCSR0A, UDRE0));
    UDR0 = c;
}

void Serial_PrintString(const char *str) {
    while (*str) {
        Serial_PrintChar(*str++);
    }
}

// Helper function for registering the UART0 port as stdout stream.
int printf_putchar(char c, FILE *stream) {
    Serial_PrintChar(c);
    return c;
}