
/* Name: main.c
 * Author: Trol
 * Copyright: (c) 2015 Oleg Trifonov, TrolSoft
 * License: <insert your license reference here>
 
 
 TASKS
  !!!! ������� �� ��������� ������� -> BAW (�������� ���������������� � ����) !!!!
  !!!! ��� ������ TAP ���� ����� ���������
	* ��������� ������ ������ � ������
   * �����, ����������������� � Windows 10 �� ��������. �������� ���� ��
	
	- �������� �� ������� � �� �����
	- ������������� ���� �����
	- ���� ����� (����, ��������, �������������, �������)
	- ����������� ���� ����������� �����
	- �������� ��������� ������
	- ������ ��������� ������
	- ����� �������� � �������� - ������� ���������
   - BAW save autostop
   - ��������� ����� ������ � �������� � �������� ��� �����
  
	? ���� �������� ������/������ �����
	? �������� �������� ���� ������ �������� ������� �������
   ? ��� ������ BAW ������������� ��������� ����������� ������������� ������� ������ �����

 */

#include "config.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#include <util/delay.h>

#include "lib/glcd/glcd.h"

#include "debug.h"


#include "fileio.h"
#include "keyboard.h"
#include "formats/tap_player.h"
#include "formats/tap_recorder.h"
#include "formats/wav_player.h"
#include "formats/wav_recorder.h"
#include "formats/baw_player.h"
#include "ui.h"
#include "settings.h"
#include "beeper.h"
#if USE_RTC_8583
	#include "pcf8583.h"
#elif USE_RTC_8563
	#include "pcf8563.h"
#endif
#include "time.h"
#include "general.h"


#define TASK_CHECK_KEYBOARD		_BV(0)

#define ADC_VOLUME					7

volatile uint8_t tasksFlag;
volatile uint16_t adcReadyVal;

extern uint8_t selectedIndex;

volatile void_proc timer1compHandler;
volatile void_proc timer1overflowHandler;

static void adcStart(uint8_t pin) {
#if AREF_INTERNAL	
	ADMUX = pin | _BV(REFS0) | _BV(REFS1);	// internal 2.56V Voltage Reference with external capacitor at AREF pin
#else	
	ADMUX = pin | _BV(REFS0);	// AVCC with external capacitor at AREF pin
#endif	
	// enable ADC, start convert, enable interrupt, prescaler = 64
	ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADIE) | _BV(ADPS1) | _BV(ADPS2);
	adcReadyVal = 0xffff;
}


//----------------------------------------------------------------------------------------------------
// ���������� ������� ���������� ������� T0 (8-�� ��������� ������) �� ������������
//----------------------------------------------------------------------------------------------------
ISR(TIMER0_OVF_vect) {
	timer1overflowHandler();
}


// ���������� ������� �������
ISR(TIMER1_CAPT_vect) {
	TapSaverCaptureHandler();	
}

ISR(TIMER1_OVF_vect) {
	TapSaverOverflowHandler();
}



ISR(TIMER2_COMP_vect) {
	OCR2 += F_CPU/1024/100;
	tasksFlag |= TASK_CHECK_KEYBOARD;	
	// ���������� 100 ��� � ���, ������ ��� ������ ����������, ���������� UI � ���������� �������
	DateTimeTick100();
}



ISR(ADC_vect) {
    adcReadyVal = ADC;
    //adcReadyChannel = ADMUX & 7;
}

// !!!!!!!!
extern uint8_t sdcard_buf_1[512];

int main(void) {
	MSG("starting...");
   set_sleep_mode(SLEEP_MODE_IDLE);
	
	DDRC |= _BV(0);		// SD card CD pin
	DDRD &= ~_BV(7);		// CD input pin
	DDRE |= _BV(3);		// tape out
	DDRE |= _BV(4);		// beeper
	DDRE |= _BV(1);		// UART TX
	DDRD &= ~_BV(4);		// tape in
	DDRA |= _BV(0);		// ��������� ������
	
	PORTD |= _BV(4);		// �������� ��� tape in
	PORTD |= _BV(7);		// �������� ��� ���� ����������� �����

	adcStart(ADC_VOLUME);

	BeeperInit();

	FioClearError();
	timer1overflowHandler = &EmptyVoidProc;

	
#if DEBUG
		uart_init();
#else
		wdt_enable(WDTO_2S);
#endif

	beep(1000, 150);
	
	TimeInit();	
	wdt_reset();
	
	SettingLoad();
	
	wdt_reset();

	// do some stuff on the LCD
	glcd_init();
	glcd_set_contrast(LCD_CONTRAST);
	glcd_set_contrast(GetDisplayContrast());
	glcd_clear();

	beep(2000, 100);
	wdt_reset();

/*
	// ------------------------------------- ���������� ��� ------------------------------
	KeyboardInit();
	if (key_down_pressed()) {
		bool in = sd_raw_init();
		if (!in) {
			MSG("MMC/SD initialization failed");
			return 1;
		}
		// ���������� ����������� �����
		SetScreen(SCREEN_PLEASE_WAIT);
		offset_t offset = 0;
		//uint8_t buffer[0x200];	

		sd_raw_init();
		uart_putc(5);
		uart_putc(25);
		uart_putc(100);
		uart_putc(50);	
		while (offset < 10L*1024L*1024) {
			wdt_reset();
			if (!sd_raw_read(offset, sdcard_buf_1, sizeof(sdcard_buf_1))) {
				beep(1000, 1000);
				wdt_reset();
				beep(100, 100);
				wdt_reset();
				beep(1000, 1000);
				wdt_reset();			
				break;
			}
			for (uint16_t i = 0; i < sizeof(sdcard_buf_1); i++) {
				uart_putc(sdcard_buf_1[i]);
				offset++;
				wdt_reset();			
			}
			//_delay_ms(25);
		}
		for (uint8_t i = 1; i < 100; i++) {
			beep(500+i*10, 50);
			wdt_reset();			
		}
		SetScreen(SCREEN_MAIN_MENU);
	}
*/

	
	
	// ������������� TC2
	TCCR2 |= _BV(CS22) | _BV(CS20);	// �������� �� 64
	TIMSK |= _BV(OCIE2);
	
	FioSetPath("/");
	//FioInit();
	FioReadDirFrom(0);	//	TODO � ���� �� ��� ������ �����?
	//FioClose();

	wdt_reset();
	sei();

	KeyboardInit();	

	// ����� ������� ��������� �� ����� �������� ������������� ������� �� ���������� LEFT + RIGHT + ENTER
	if (key_enter_pressed() && key_left_pressed() && key_right_pressed()) {
		beep(1000, 50);
		beep(1500, 30);
		beep(2000, 20);
		beep(1500, 30);
		beep(1000, 50);
		beep(700, 50);
		
		tasksFlag = 0xff;
		
		// wait until user released keyboard
		while (key_enter_pressed() || key_left_pressed() || key_right_pressed()) {
			wdt_reset();
			beep(1000, 1000);
			wdt_reset();
		}
		selectedIndex = 0;	// ����� ������� ������������ � ������� ����
		SetScreen(SCREEN_SETUP_CONTRAST);
	} else {
		SetScreen(SCREEN_MAIN_MENU);
	}
	
	tasksFlag = 0;
	Draw();
	wdt_reset();
	
	uint16_t highlightCounter = 1000;
	if (GetHighlight() != HIGHLIGHT_ON) {
		highlight_off();
	}

	for(;;) {
		// ����� ����������	
		if (tasksFlag & TASK_CHECK_KEYBOARD) {
			tasksFlag &= ~TASK_CHECK_KEYBOARD;
			wdt_reset();
			KeyboardCheck();
			
			if (GetHighlight() == HIGHLIGHT_AUTO) {
				
				if (key_any_pressed()) {
					highlightCounter = 500;	// 5 ������
					highlight_on();
				} else {
					if (highlightCounter != 0) {
						highlightCounter--;
						if (highlightCounter == 0) {
							highlight_off();
						}
					}
				}
			}
			
			//if ((adcReadyVal & _BV(12)) == 0) {

			if (adcReadyVal != 0xffff) {
				beeper_volume = adcReadyVal >> 2;
				adcStart(ADC_VOLUME);
			}
			
			static uint8_t uiCnt = 0;
			if (uiCnt++ > 5) {
				uiCnt = 0;
				UiOnIdle();
			}
		}
		
		FioOnIdle();	// TODO use idle handler
		TapOnIdle();
		TapSaveOnIdle();
    } // for
	
	
	return 0;   // never reached
}
