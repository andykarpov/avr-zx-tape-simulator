/*
 * general.h
 *
 * Created: 19.10.2015 22:07:36
 *  Author: Trol
 */ 


#ifndef GENERAL_H_
#define GENERAL_H_

volatile bool pause_mode;

#define set_tape_out()	PORTE |= _BV(3)
#define clr_tape_out()	PORTE &= ~_BV(3)


#define LOW(x)			((x) & 0xFF)
#define HIGH(x)		(((x)>>8) & 0xFF)

typedef void (*void_proc)();
typedef void (*uint8_proc)(uint8_t);
typedef uint8_t (*uint8_func)();



extern volatile void_proc timer1compHandler;
extern volatile void_proc timer1overflowHandler;

extern volatile void_proc fio_read_eof_handler;
extern volatile uint8_t fio_error_mask;
extern volatile uint16_t fio_error_cnt;

//extern char* sdcard_ls_data;

volatile bool pause_mode;


void EmptyVoidProc();

#endif // GENERAL_H_