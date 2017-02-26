/*
 * Copyright (c) 2010-2013 by Oleg Trifonov <otrifonow@gmail.com>
 *
 * http://trolsoft.ru
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef _PCF8583_H_
#define _PCF8583_H_

#include "config.h"

#include <stdint.h>
#include <stdbool.h>


#define PCF8583_ADDRESS		0b10100000

// addresses of memory cells for PCF8583
#define PCF8583_REG_CONTROL				0
#define PCF8583_REG_HSECONDS			1
#define PCF8583_REG_SECONDS				2
#define PCF8583_REG_MINUTES				3
#define PCF8583_REG_HOURS				4
#define PCF8583_REG_YEAR_DAYS			5
#define PCF8583_REG_MONTHS_WEEK_DAYS	6

/*
#define PCF8583_REG_MONTHS				7
#define PCF8583_REG_YEARS				8
#define PCF8583_REG_ALARM_MIN			9
#define PCF8583_REG_ALARM_HOUR			10
#define PCF8583_REG_ALARM_DAY			11
#define PCF8583_REG_ALARM_WEEK_DAY		12
*/

// configure bits (for the cell PCF8583_REG_CONTROL)
//#define PCF8583_CFG_TIMER_FLAG			0	// 50% duty factor seconds flag if alarm enable bit is 0)
//#define PCF8583_CFG_ALARM_FLAG			1	// 50% duty factor minutes flag if alarm enable bit is 0)
#define PCF8583_CFG_ALARM_ENABLE_BIT	2	// установка этого флага включает будильник

#if USE_RTC_8583
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
	 * day of mounth, 01 to 28,29,30,31 
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
} datetime_t;

bool pcf8583_GetTime(datetime_t *dt);
bool pcf8583_SetTime(datetime_t *dt);
bool pcf8583_GetDate(datetime_t *dt);
bool pcf8583_SetDate(datetime_t *dt);
bool pcf8583_GetDateTime(datetime_t *dt);
bool pcf8583_SetDateTime(datetime_t *dt);

#endif

#endif // _PCF8583_H_
