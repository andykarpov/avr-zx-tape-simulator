/*
 * beeper.h
 *
 * Created: 27.06.2015 21:07:59
 *  Author: Trol
 */ 


#ifndef BEEPER_H_
#define BEEPER_H_

#include <stdint.h>
#include <avr/io.h>

extern volatile uint8_t beeper_volume;

#define beeper_on()			OCR3BL = beeper_volume// PORTE |= _BV(4)
#define beeper_off()			OCR3BL = 0//PORTE &= ~_BV(4)


#if DISPLAY_HIGHLIGHT_INVERT
#	define highlight_on()		PORTA &= ~_BV(0)
#	define highlight_off()		PORTA |= _BV(0)
#else
#	define highlight_on()		PORTA |= _BV(0)
#	define highlight_off()		PORTA &= ~_BV(0)
#endif

void beep(uint16_t f, uint16_t d);


void BeeperInit();
void BeeperOn();
void BeeperOff();
//void BeeperSetVolume(uint8_t volume);
//uint8_t BeeperGetVolume();


#endif // BEEPER_H_ 