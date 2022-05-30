/*
 * serial.h
 *
 * Created: 2022. 05. 18. 1:07:41
 *  Author: gabor
 */

//////////////////////////////////////////////////////////////////////////
/// 
/// Two-way serial communication module using UART0
///
/// Registers a custom stream for stdout, so that printf calls can be used
/// to send messages directly through serial.
///
/// Call Serial_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef SERIAL_H_
#define SERIAL_H_

#ifndef F_CPU
#error "F_CPU needs to be defined and set to the frequency of the CPU!"
#endif

#include <stdio.h>
#include "task_scheduler.h"
#include "utils.h"

#define SERIAL_RECEIVE_BUFFER_SIZE 64

//////////////////////////////////////////////////////////////////////////
/// INIT
//////////////////////////////////////////////////////////////////////////

int printf_putchar(char c, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM(printf_putchar, NULL, _FDEV_SETUP_WRITE);

void (*line_received_handler)(uint8_t id, bool pressed);

/**
 * Initializes the serial communication, sets up USART0 to receive and send mode
 * and sets up stdout so that printf functions can write directly to serial.
 */
void Serial_Init(uint16_t baud) {
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
}

//////////////////////////////////////////////////////////////////////////
/// DATA RECEIVE
//////////////////////////////////////////////////////////////////////////

char recvBuf[SERIAL_RECEIVE_BUFFER_SIZE];
uint8_t idxR0 = 0, idxR1 = 0;
bool unread = false;

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

/**
 * Gets if there is any unread data.
 */
bool Serial_HasUnread() {
    return unread;
}

/**
 * Reads the first unread byte, or returns -1 if no unread data is available.
 */
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

//////////////////////////////////////////////////////////////////////////
/// DATA SEND
//////////////////////////////////////////////////////////////////////////

/**
 * Sends one byte of data through the serial port (UART0).
 * Waits until the port has finished sending the previous byte, and then sends it.
 */
void Serial_PrintChar(char c) {
    // wait until USART data register becomes empty.
    while (bit_is_clear(UCSR0A, UDRE0));
    UDR0 = c;
}

/**
 * Sends a string through the serial port (UART0).
 * For long strings, this may take a while because it waits for each character to be sent over.
 */
void Serial_PrintString(const char *str) {
    while(*str) {
        Serial_PrintChar(*str++);
    }
}

// Helper function for registering the UART0 port as stdout stream.
int printf_putchar(char c, FILE *stream) {
    Serial_PrintChar(c);
    return c;
}

#endif /* SERIAL_H_ */