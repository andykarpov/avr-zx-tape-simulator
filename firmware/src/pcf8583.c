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

#include "pcf8583.h"

#if USE_RTC_8583

#include "debug.h"
#include "i2c.h"



static uint8_t bcd2int(uint8_t val) {
	return ((val >> 4) & 0x0F)*10 + (val & 0x0F);
}

static uint8_t int2bcd(uint8_t val) {
	return ((val / 10) << 4) + val % 10;
}

/** 
 * Получить значение времени из PCF8583 
 */
bool pcf8583_GetTime(datetime_t *dt) {
	i2c_init();

	if ( !i2c_start() ||
		!i2c_sendAddress(PCF8583_ADDRESS | I2C_WRITE) ||
		!i2c_sendByte(PCF8583_REG_HSECONDS) ||
		!i2c_stop() ||

		!i2c_start() ||
		!i2c_sendAddress(PCF8583_ADDRESS | I2C_READ) ||
		!i2c_receiveByteACK(&dt->hsec) ||
		!i2c_receiveByteACK(&dt->sec) ||
		!i2c_receiveByteACK(&dt->min) ||
		!i2c_receiveByteNACK(&dt->hour) ||
		!i2c_stop() )
	{
		MSG("ERR getTime()");
		i2c_stop();
		return false;
	}

	dt->hsec = bcd2int(dt->hsec);
	dt->sec = bcd2int(dt->sec);
	dt->min = bcd2int(dt->min);
	dt->hour = bcd2int(dt->hour);

	MSG_DEC("hour = ", dt->hour);
	MSG_DEC("min = ", dt->min);
	MSG_DEC("sec = ", dt->sec);
	MSG_DEC("hsec = ", dt->hsec);

	return true;
}

bool pcf8583_SetTime(datetime_t *dt) {
	i2c_init();

	if (!i2c_start() ||
		!i2c_sendAddress(PCF8583_ADDRESS | I2C_WRITE) ||
		!i2c_sendByte(PCF8583_REG_HSECONDS) ||

		!i2c_sendByte(int2bcd(dt->hsec)) ||
		!i2c_sendByte(int2bcd(dt->sec)) ||
		!i2c_sendByte(int2bcd(dt->min)) ||
		!i2c_sendByte(int2bcd(dt->hour)) ||
		!i2c_stop())
	{
		MSG("ERR setTime()");
		i2c_stop();
		return false;
	}

	return true;
}



bool pcf8583_GetDate(datetime_t *dt) {
	uint8_t b1, b2;

	i2c_init();

	if ( !i2c_start() ||
		!i2c_sendAddress(PCF8583_ADDRESS | I2C_WRITE) ||
		!i2c_sendByte(PCF8583_REG_YEAR_DAYS) ||
		!i2c_stop() ||

		!i2c_start() ||
		!i2c_sendAddress(PCF8583_ADDRESS | I2C_READ) ||
		!i2c_receiveByteACK(&b1) ||
		!i2c_receiveByteNACK(&b2) ||
		!i2c_stop() )
	{
		MSG("ERR getDate()");
		i2c_stop();
		return false;
	}

	dt->year = (b1 >> 6) & 3;
	dt->month = bcd2int(b2 & 0b00011111);
	dt->day = bcd2int(b1 & 0b00111111);
	MSG_HEX("get b1 ", b1, 1);
	MSG_HEX("get b2 ", b2, 1);
	MSG_DEC("get year ", dt->year);
	MSG_DEC("get month ", dt->month);
	MSG_DEC("get day ", dt->day);

	return true;
}



bool pcf8583_SetDate(datetime_t *dt) {
	i2c_init();
	uint8_t b1 = int2bcd(dt->day) | (dt->year << 6);
	uint8_t b2 = int2bcd(dt->month);

	if ( !i2c_start() ||
		!i2c_sendAddress(PCF8583_ADDRESS | I2C_WRITE) ||
		!i2c_sendByte(PCF8583_REG_YEAR_DAYS) ||

		!i2c_sendByte(b1) ||
		!i2c_sendByte(b2) ||
		!i2c_stop() )
	{
		MSG("ERR setDate()");
		i2c_stop();
		return false;
	}
	MSG_HEX("set b1 ", b1, 1);
	MSG_HEX("set b2 ", b2, 1);
	MSG_DEC("set year ", dt->year);
	MSG_DEC("set month ", dt->month);
	MSG_DEC("set day ", dt->day);

	return true;
}


bool pcf8583_GetDateTime(datetime_t *dt) {
	return pcf8583_GetDate(dt) && pcf8583_GetTime(dt);
}


bool pcf8583_SetDateTime(datetime_t *dt) {
	return pcf8583_SetDate(dt) && pcf8583_SetTime(dt);
}

#endif
