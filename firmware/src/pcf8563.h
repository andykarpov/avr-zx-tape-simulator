/*
 * pcf8563.h
 *
 * Created: 15.08.2015 21:10:54
 *  Author: Trol
 */ 


#ifndef PCF8563_H_
#define PCF8563_H_

#include "config.h"


#include <stdint.h>
#include <stdbool.h>

#if USE_RTC_8563

typedef struct datetime_t {
	/**
	 * year, 0 to 3
	 */
	uint8_t year;
	/**
	 * month, 01 to 12
	 */
	uint8_t month;
	/**
	 * day of month, 01 to 28,29,30,31 
	 */
	uint8_t day;
	/**
	 * hour, 00 to 23
	 */
	uint8_t hour;
	/**
	 * minutes, 00 to 59
	 */
	uint8_t min;
	/**
	 * seconds, 00 to 59
	 */
	uint8_t sec;
	/**
	 * hundredths of a second, 00 to 99
	 */
	uint8_t hsec;
	
	/**
	 * weekdays (0 to 6)
	 */
	uint8_t weekday;
} datetime_t;


bool pcf8563_GetTime(datetime_t *dt);
bool pcf8563_SetTime(datetime_t *dt);
bool pcf8563_GetDate(datetime_t *dt);
bool pcf8563_SetDate(datetime_t *dt);
bool pcf8563_GetDateTime(datetime_t *dt);
bool pcf8563_SetDateTime(datetime_t *dt);


#endif

#endif /* PCF8563_H_ */