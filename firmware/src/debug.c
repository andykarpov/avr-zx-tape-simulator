//#define DEBUG 1

#if DEBUG

#include "debug.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sfr_defs.h>

#ifdef SIMULATION
#	include <stdio.h>
#endif

#define    RXEN         4
#define    TXEN         3
#define    RXC          7
#define    UDRE         5



void uart_init() {
#ifndef SIMULATION	
//	UBRR0 = UART_RATE_VALUE;
	UBRR0H = UART_RATE_VALUE >> 8;
	UBRR0L = (uint8_t)UART_RATE_VALUE;

	//UCSRB = _BV(TXEN) | _BV(RXEN) | _BV(TXCIE) | _BV(RXCIE); /* tx/rx enable, interrupts enable */
	UCSR0B = _BV(TXEN) | _BV(RXEN); // tx/rx enable
#endif
}


void uart_putc(uint8_t c) {
#ifdef SIMULATION
	printf("%c", c);
#else
	while ( !(UCSR0A & (1 << UDRE)) );
	UDR0 = c;
#endif	
}




void uart_putc_hex(uint8_t b) {
	// upper nibble
	if ( (b >> 4) < 0x0a )
		uart_putc((b >> 4) + '0');
	else
		uart_putc((b >> 4) - 0x0a + 'a');

	// lower nibble
	if ( (b & 0x0f) < 0x0a )
		uart_putc((b & 0x0f) + '0');
	else
		uart_putc((b & 0x0f) - 0x0a + 'a');
}



void uart_putw_hex(uint16_t w) {
	uart_putc_hex((uint8_t) (w >> 8));
	uart_putc_hex((uint8_t) (w & 0xff));
}


void uart_putdw_hex(uint32_t dw) {
	uart_putw_hex((uint16_t) (dw >> 16));
	uart_putw_hex((uint16_t) (dw & 0xffff));
}


void uart_putw_dec(uint16_t w) {
	uint16_t num = 10000;
	uint8_t started = 0;

	while ( num > 0 ) {
		uint8_t b = w / num;
		if ( b > 0 || started || num == 1 ) {
			uart_putc('0' + b);
			started = 1;
		}
		w -= b * num;
		num /= 10;
	}
}


void uart_putdw_dec(uint32_t dw) {
	uint32_t num = 1000000000;
	uint8_t started = 0;

	while ( num > 0 ) {
		uint8_t b = dw / num;
		if( b > 0 || started || num == 1 ) {
			uart_putc('0' + b);
			started = 1;
		}
		dw -= b * num;
		num /= 10;
	}
}


void uart_puts(const char* str) {
	while( *str )
		uart_putc(*str++);
}



void uart_puts_p(PGM_P str) {
	while ( 1 ) {
		uint8_t b = pgm_read_byte_near(str++);
		if ( !b )
			break;
		uart_putc(b);
	}
}


uint8_t uart_getc() {
#ifndef SIMULATION	
	while(!(UCSR0A & (1 << RXC)));

	uint8_t b = UDR0;
	if ( b == '\r' )
		b = '\n';

	return b;
#else
	return 0;
#endif
}


void debug_msg_(const char *str)	{ 
	uart_puts_p(str); 
	uart_putc('\n'); 
}

void debug_msg_dec_(const char *str, uint32_t val) { 
	uart_puts_p(str); 
	uart_putdw_dec(val); 
	uart_putc('\n');
}

void debug_msg_hex_(const char *str, uint32_t val, uint8_t bytes) { 
	uart_puts_p(str); 
	if (bytes == 1) 
		uart_putc_hex(val);
	else if (bytes == 2) 
		uart_putw_hex(val); 
	else if (bytes == 4) 
		uart_putdw_hex(val);	
	uart_putc('\n'); 
}

void debug_msg_str_(const char *str, const char *val) {
	uart_puts_p(str);
	uart_puts(val);
	uart_putc('\n');
}




//EMPTY_INTERRUPT(USART_RXC_vect)

#endif
