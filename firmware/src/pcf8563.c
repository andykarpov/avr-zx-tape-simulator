/*
 * pcf8563.c
 *
 * Created: 15.08.2015 21:11:13
 *  Author: Trol
 */ 

#include "pcf8563.h"

#include "debug.h"
#include "i2c.h"

#define PCF8563_ADDRESS		0xA2


// addresses of memory cells for PCF8563
#define PCF8563_REG_CONTROL_STATUS_1				0
#define PCF8563_REG_CONTROL_STATUS_2				1
#define PCF8563_REG_SECONDS							2
#define PCF8563_REG_MINUTES							3
#define PCF8563_REG_HOURS							4
#define PCF8563_REG_YEAR_DAYS						5
#define PCF8563_REG_WEEK_DAYS						6
#define PCF8563_REG_MONTH							7
#define PCF8563_REG_YEAR							8

#define PCF8563_REG_ALARM_MINUTE					9
#define PCF8563_REG_ALARM_HOUR						10
#define PCF8563_REG_ALARM_DAY						11
#define PCF8563_REG_ALARM_WEEKDAY					12

#define PCF8563_REG_CLKOUT							13
#define PCF8563_REG_TIMER_CONTROL					14
#define PCF8563_REG_TIMER							15


#if USE_RTC_8563

static uint8_t bcd2int(uint8_t val) {
	return ((val >> 4) & 0x0F)*10 + (val & 0x0F);
}

static uint8_t int2bcd(uint8_t val) {
	return ((val / 10) << 4) + val % 10;
}


static void pcf8563_Init() {
	i2c_init();
	
	uint8_t control1, control2, vl_sec;
	if (!i2c_start() ||
		!i2c_sendAddress(PCF8563_ADDRESS | I2C_WRITE) ||
		!i2c_sendByte(PCF8563_REG_CONTROL_STATUS_1) ||
		!i2c_stop() ||
		
		!i2c_start() ||
		!i2c_sendAddress(PCF8563_ADDRESS | I2C_READ) ||
		!i2c_receiveByteACK(&control1) ||
		!i2c_receiveByteACK(&control2) ||		
		!i2c_receiveByteNACK(&vl_sec) ) {
			MSG("ERR init0()");
		}
	i2c_stop();		
	if (control1 == 0 && control2 == 0 && !(vl_sec & _BV(7)) ) {
		return;
	}
	MSG("INIT PCF8563");

	if (!i2c_start() ||
		!i2c_sendAddress(PCF8563_ADDRESS | I2C_WRITE) ||
		!i2c_sendByte(PCF8563_REG_CONTROL_STATUS_1) ||

		!i2c_sendByte(0) ||
		!i2c_sendByte(0) ||
		!i2c_sendByte(vl_sec & 0b01111111) ||		
		!i2c_stop())
		{
			MSG("ERR init()");
			i2c_stop();
	}
}


bool pcf8563_GetTime(datetime_t *dt) {
	//i2c_init();
	pcf8563_Init();

	if ( !i2c_start() ||
		!i2c_sendAddress(PCF8563_ADDRESS | I2C_WRITE) ||
		!i2c_sendByte(PCF8563_REG_SECONDS) ||
		!i2c_stop() ||

		!i2c_start() ||
		!i2c_sendAddress(PCF8563_ADDRESS | I2C_READ) ||
		!i2c_receiveByteACK(&dt->sec) ||
		!i2c_receiveByteACK(&dt->min) ||
		!i2c_receiveByteNACK(&dt->hour) ||
		!i2c_stop() )
	{
		MSG("ERR getTime()");
		i2c_stop();
		return false;
	}

	MSG_HEX("b1 = ", dt->sec, 1);
	MSG_HEX("b2 = ", dt->min, 1);
	MSG_HEX("b3 = ", dt->hour, 1);

//	dt->hsec = bcd2int(dt->hsec);
	dt->sec = bcd2int(dt->sec & 0b01111111);
	dt->min = bcd2int(dt->min & 0b01111111);
	dt->hour = bcd2int(dt->hour & 0b00111111);

	MSG_DEC("hour = ", dt->hour);
	MSG_DEC("min = ", dt->min);
	MSG_DEC("sec = ", dt->sec);
	MSG_DEC("hsec = ", dt->hsec);

	return true;
}


bool pcf8563_SetTime(datetime_t *dt) {
	pcf8563_Init();
	
	if (!i2c_start() ||
		!i2c_sendAddress(PCF8563_ADDRESS | I2C_WRITE) ||
		!i2c_sendByte(PCF8563_REG_SECONDS) ||

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


bool pcf8563_GetDate(datetime_t *dt) {
	pcf8563_Init();

	if (!i2c_start() ||
		!i2c_sendAddress(PCF8563_ADDRESS | I2C_WRITE) ||
		!i2c_sendByte(PCF8563_REG_YEAR_DAYS) ||
		!i2c_stop() ||

		!i2c_start() ||
		!i2c_sendAddress(PCF8563_ADDRESS | I2C_READ) ||
		!i2c_receiveByteACK(&dt->day) ||
		!i2c_receiveByteACK(&dt->weekday) ||
		!i2c_receiveByteACK(&dt->month) ||
		!i2c_receiveByteNACK(&dt->year) ||
		!i2c_stop())
	{
		MSG("ERR getDate()");
		i2c_stop();
		return false;
	}
	dt->day = bcd2int(dt->day & 0b00111111);
	dt->month = bcd2int(dt->month & 0b00011111);	
	dt->year = bcd2int(dt->year);	

	MSG_DEC("get year ", dt->year);
	MSG_DEC("get month ", dt->month);
	MSG_DEC("get day ", dt->day);
	MSG_DEC("get weekday ", dt->weekday);

	return true;

}


bool pcf8563_SetDate(datetime_t *dt) {
	pcf8563_Init();

	if (!i2c_start() ||
		!i2c_sendAddress(PCF8563_ADDRESS | I2C_WRITE) ||
		!i2c_sendByte(PCF8563_REG_YEAR_DAYS) ||

		!i2c_sendByte(int2bcd(dt->day)) ||
		!i2c_stop() )
	{
		MSG("ERR setDate()");
		i2c_stop();
		return false;
	}
	if (!i2c_start() ||
		!i2c_sendAddress(PCF8563_ADDRESS | I2C_WRITE) ||
		!i2c_sendByte(PCF8563_REG_MONTH) ||

		!i2c_sendByte(int2bcd(dt->month)) ||
		!i2c_sendByte(int2bcd(dt->year)) ||
		!i2c_stop() )
	{
		MSG("ERR setDate()");
		i2c_stop();
		return false;
	}	
	MSG("set date");

	return true;	
}


bool pcf8563_GetDateTime(datetime_t *dt) {
	return pcf8563_GetDate(dt) && pcf8563_GetTime(dt);
}


bool pcf8563_SetDateTime(datetime_t *dt) {
	return pcf8563_SetDate(dt) && pcf8563_SetTime(dt);
}


#endif