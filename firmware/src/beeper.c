/*
 * beeper.c
 *
 * Created: 29.06.2015 21:34:58
 *  Author: Trol
 */ 
#include "beeper.h"

#include <util/delay.h>

volatile uint8_t beeper_volume = 128;

//void BeeperOn() {
	//OCR3B = beeper_volume;// PORTE |= _BV(4)
//}
//
//void BeeperOff() {
	//OCR3B = 0;
//}



void beep(uint16_t f, uint16_t d) {
	for (uint16_t i = 0; i < d; i++) {
		//BeeperOn();
		beeper_on();
		_delay_us(500000/f);
		//BeeperOff();
		beeper_off();
		_delay_us(500000/f);
	}
}


//void BeeperSetVolume(uint8_t v) {
	//beeper_volume = v;
//}
//
//uint8_t BeeperGetVolume() {
	//return beeper_volume;
//}


void BeeperInit() {
	// быстрая ШИМ без делителя, OC3B - выход
	TCCR3A |= _BV(WGM30) | _BV(COM3B1);
	TCCR3B |= _BV(WGM32) | _BV(CS30);
}
