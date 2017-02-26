/*
 * keyboard.c
 *
 * Created: 05.06.2015 20:36:16
 *  Author: Trol
 */ 
#include "keyboard.h"

#include "ui.h"
#include "beeper.h"
#include "settings.h"

#define KEY_PRESS_TIME		5
#define KEY_REPEAT_TIME		50


uint8_t keyRepeatCount[5];


static void beepKey() {
	beep(1000, 10);
}

void KeyboardCheck() {
	if (key_left_pressed()) {
		if (keyRepeatCount[KEY_LEFT] < 0xff) {
			keyRepeatCount[KEY_LEFT]++;
		}
	} else {
		keyRepeatCount[KEY_LEFT] = 0;
	}
	
	if (key_right_pressed()) {
		if (keyRepeatCount[KEY_RIGHT] < 0xff) {
			keyRepeatCount[KEY_RIGHT]++;
		}
	} else {
		keyRepeatCount[KEY_RIGHT] = 0;
	}
	
	if (key_up_pressed()) {
		if (keyRepeatCount[KEY_UP] < 0xff) {
			keyRepeatCount[KEY_UP]++;
		}
	} else {
		keyRepeatCount[KEY_UP] = 0;
	}
	
	if (key_down_pressed()) {
		if (keyRepeatCount[KEY_DOWN] < 0xff) {
			keyRepeatCount[KEY_DOWN]++;
		}
	} else {
		keyRepeatCount[KEY_DOWN] = 0;
	}
	
	if (key_enter_pressed()) {
		if (keyRepeatCount[KEY_ENTER] < 0xff) {
			keyRepeatCount[KEY_ENTER]++;
		}
	} else {
		keyRepeatCount[KEY_ENTER] = 0;
	}

	
	bool beep = GetSoundKey();
	
	if (keyRepeatCount[KEY_LEFT] == KEY_PRESS_TIME) {
		onKeyPressed(KEY_LEFT);
		if (beep) {
			beepKey();
			beep = false;
		}
	}
	if (keyRepeatCount[KEY_RIGHT] == KEY_PRESS_TIME) {
		onKeyPressed(KEY_RIGHT);
		if (beep) {
			beepKey();
			beep = false;
		}
	}
	if (keyRepeatCount[KEY_UP] == KEY_PRESS_TIME) {
		onKeyPressed(KEY_UP);
		if (beep) {
			beepKey();
			beep = false;
		}
	}
	if (keyRepeatCount[KEY_DOWN] == KEY_PRESS_TIME) {
		onKeyPressed(KEY_DOWN);
		if (beep) {
			beepKey();
			beep = false;
		}
	}
	if (keyRepeatCount[KEY_ENTER] == KEY_PRESS_TIME) {
		onKeyPressed(KEY_ENTER);		
		if (beep) {// && beeper_volume != 0) {	// TODO !!! без этой проверки почему-то не работает регулировка громкости 
			beepKey();
			beep = false;
		}
	}

}



void KeyboardInit() {
	DDRC |= _BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6);
	PORTC |= _BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6);
	keyRepeatCount[KEY_LEFT] = 0;
	keyRepeatCount[KEY_RIGHT] = 0;
	keyRepeatCount[KEY_UP] = 0;
	keyRepeatCount[KEY_DOWN] = 0;
	keyRepeatCount[KEY_ENTER] = 0;
}


bool KeyboardRepeated(uint8_t key) {
	return keyRepeatCount[key] > KEY_REPEAT_TIME;
}