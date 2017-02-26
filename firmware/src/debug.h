#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <avr/pgmspace.h>


#if DEBUG

#include <stdint.h>
#include <avr/pgmspace.h>





//#define UART_RATE_VALUE  0x340			// скорость 1200, без ошибок
#define UART_RATE_VALUE  0x10			// скорость 57600, 2.12% ошибок



void uart_init();

void uart_putc(uint8_t c);

void uart_putc_hex(uint8_t b);
void uart_putw_hex(uint16_t w);
void uart_putdw_hex(uint32_t dw);

void uart_putw_dec(uint16_t w);
void uart_putdw_dec(uint32_t dw);

void uart_puts(const char* str);
void uart_puts_p(PGM_P str);

uint8_t uart_getc();

void debug_msg_(const char *str);
void debug_msg_dec_(const char *str, uint32_t val);
void debug_msg_hex_(const char *str, uint32_t val, uint8_t bytes);

#define MSG_(str)	{ uart_puts_p(str); uart_putc('\n'); }
#define MSG_DEC_(str, val) { uart_puts_p(str); uart_putdw_dec(val); uart_putc('\n'); }
#define MSG_HEX_(str, val, bytes) { uart_puts_p(str); if (bytes == 1) uart_putc_hex(val);	else if (bytes == 2) uart_putw_hex(val); else if (bytes == 4) uart_putdw_hex(val);	uart_putc('\n'); }

#define MSG(str)								debug_msg_(PSTR(str))
#define MSG_HEX(str, val, bytes)			debug_msg_hex_(PSTR(str), (val), bytes)
#define MSG_DEC(str, val)					debug_msg_dec_(PSTR(str), (val))
#define MSG_STR(str, val)					debug_msg_str_(PSTR(str), (val))


#else
	inline void uart_init() {
	}
	
#define MSG(str) ;
#define MSG_HEX(str, val, bytes) ;
#define MSG_DEC(str, val) ;
#define MSG_STR(str, val) ;

	
#endif // DEBUG


//inline void MSG_(const char *str) {
//#if DEBUG
//	uart_puts_p(str);
//	uart_putc('\n');
//#endif
//}

//inline void MSG_HEX_(const char *str, uint32_t val, uint8_t bytes) {
//#if DEBUG
//	uart_puts_p(str);
//	if ( bytes == 1 )
//		uart_putc_hex(val);
//	else if ( bytes == 2 )
//		uart_putw_hex(val);
//	else if ( bytes == 4 )
//		uart_putdw_hex(val);
//	uart_putc('\n');
//#endif
//}


//inline void MSG_DEC_(const char *str, uint32_t val) {
//#if DEBUG
//	uart_puts_p(str);
//	uart_putdw_dec(val);
//	uart_putc('\n');
//#endif
//}

//inline void MSG_STR_(const char *str, const char *val) {
//#if DEBUG
//	uart_puts_p(str);
//	uart_puts(val);
//	uart_putc('\n');
//#endif
//}




#endif // _DEBUG_H_
