/*
* serial.h
*
* Created: 2022. 05. 18. 1:07:41
*  Author: gabor
*/


#ifndef SERIAL_H_
#define SERIAL_H_

#ifndef F_CPU
#error "F_CPU needs to be defined and set to the frequency of the CPU!"
#endif

#include <stdio.h>
#include "utils.h"

//////////////////////////////////////////////////////////////////////////
/// INIT
//////////////////////////////////////////////////////////////////////////

void printf_putchar(char c, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM(printf_putchar, NULL, _FDEV_SETUP_WRITE);

/**
Initializes the serial communication, sets up USART0 to receive and send mode
and sets up stdout so that printf functions can write directly to serial.
*/
void Serial_Init(unsigned int baud) {
	// set baud rate
	unsigned int ubrr = F_CPU / 16 / baud - 1;
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	
	// enable usart rx, tx and receive interrupts
	set_bit(UCSR0B, RXEN0);
	set_bit(UCSR0B, TXEN0);
	set_bit(UCSR0B, RXCIE0);
	
	// setup stdout
	stdout = &mystdout;
}

//////////////////////////////////////////////////////////////////////////
/// DATA RECEIVE
//////////////////////////////////////////////////////////////////////////

char recvBuf[64];
unsigned char idxR0 = 0, idxR1 = 0;
bool unread = false;

/**
Interrupt vector to handle received bytes.
*/
ISR(USART0_RX_vect) {
	if (bit_is_clear(UCSR0A, FE0)) {
		if (idxR0 > 63) idxR0 = 0;
		uint8_t c = UDR0;
		recvBuf[idxR0++] = c;
		unread = true;
	}
}

/**
Get if there is any unread data.
*/
bool Serial_Has_Unread() {
	return unread;
}

/**
Reads the first unread byte, or returns -1 if no unread data is available.
*/
char Serial_Read() {
	if (idxR1 > 63) idxR1 = 0;
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

void Serial_PrintChar(char c) {
	
	// wait until USART data register becomes empty.
	while (bit_is_clear(UCSR0A, UDRE0));
	UDR0 = c;
}

void printf_putchar(char c, FILE *stream) {
	Serial_PrintChar(c);
}

/*
* Send a C (NUL-terminated) string down the UART Tx.
*/
void Serial_PrintString(const char *s) {
	while (*s) {
		if (*s == '\n') Serial_PrintChar('\r');
		Serial_PrintChar(*s++);
	}
}

#endif /* SERIAL_H_ */