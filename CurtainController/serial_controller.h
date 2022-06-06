/*
 * serial_controller.h
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
/// Calls the registered handler function for every new line received.
///
/// Call Serial_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef SERIAL_CONTROLLER_H_
#define SERIAL_CONTROLLER_H_

#include "utils.h"

#define SERIAL_RECEIVE_BUFFER_SIZE 64
#define SERIAL_RECEIVE_LINE_BUFFER_SIZE 64

/**
 * Initializes the serial communication, sets up USART0 to receive and send mode
 * and sets up stdout so that printf functions can write directly to serial.
 */
void Serial_Init(uint16_t baud, void (*handler)(char *line));

/**
 * Gets if there is any unread data.
 */
bool Serial_HasUnread();

/**
 * Reads the first unread byte, or returns -1 if no unread data is available.
 */
char Serial_Read();

/**
 * Sends one byte of data through the serial port (UART0).
 * Waits until the port has finished sending the previous byte, and then sends it.
 */
void Serial_PrintChar(char c);

/**
 * Sends a string through the serial port (UART0).
 * For long strings, this may take a while because it waits for each character to be sent over one by one.
 */
void Serial_PrintString(const char *str);

#endif /* SERIAL_CONTROLLER_H_ */