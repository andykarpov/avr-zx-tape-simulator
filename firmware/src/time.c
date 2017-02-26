#include "time.h"

#include <avr/pgmspace.h>
#include "settings.h"


datetime_t dt;

/**
 * —колько дней в мес€це (дл€ високосного года в феврале 29 дней)
 */
PROGMEM const uint8_t daysPerMonth[12] = {
	31,		// €нварь
	28,		// февраль
	31,		// март
	30,		// апрель
	31,		// май
	30,		// июнь
	31,		// июль
	31,		// август
	30,		// сент€брь
	31,		// окт€брь
	30,		// но€брь
	31		// декабрь
};


void TimeInit() {
#if USE_RTC_8583
	pcf8583_GetDateTime(&dt);
#elif USE_RTC_8563
	pcf8563_GetDateTime(&dt);
#endif
}

/**
 * ¬озвращает текущие дату и врем€
 */
datetime_t* GetDateTime() {
	return &dt;
}


/**
 * »нкрементирует врем€ на 0.01 секунду
 */
void DateTimeTick100() {
	dt.hsec++;
	if (dt.hsec >= 100) {
		dt.hsec = 0;
		dt.sec++;
		if (dt.sec >= 60) {
			dt.sec = 0;
			dt.min++;
			if (dt.min >= 60) {
				dt.min = 0;
				dt.hour++;
				if (dt.hour >= 24) {
					dt.hour = 0;
					dt.day++;
					if (dt.day > GetDaysPerMonth(&dt)) {
						dt.day = 1;
						dt.month++;
						if (dt.month > 12) {
							dt.month = 1;
							dt.year++;
						}
					}
				}
			}
		}
	}
}


/**
 * ¬озвращает количество дней в мес€це
 */
uint8_t GetDaysPerMonth(datetime_t *dt) {
	if ((dt->month == 2) && ((dt->year & 3) == 0)) {
		return 29;
	}
	return pgm_read_byte(&daysPerMonth[dt->month-1]);
}


void get_datetime(uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* min, uint8_t* sec) {
	datetime_t datetime;
#if USE_RTC_8583
	pcf8583_GetDateTime(&datetime);
#elif USE_RTC_8563
	pcf8563_GetDateTime(&datetime);
#endif

	*year = GetBaseYear() + datetime.year;
	*month = datetime.month;
	*day = datetime.day;
	*hour = datetime.hour;
	*min = datetime.min;
	*sec = datetime.sec;
}


uint16_t daysInYear(uint16_t year) {
	if ((year % 4) == 0) {
		if ( ((year % 100) != 0) || ((year % 400) == 0) ) {
			return 366;
		}
	}
	return 365;
}


int32_t TimeDelta(datetime_t *dt1, datetime_t *dt2) {
	//int8_t deltaSec = dt1->sec - dt2->sec;
	//int8_t deltaMin = dt1->min - dt2->min;
	//int8_t deltaHour = dt1->hour - dt2->hour;
	//int8_t deltaDay = dt1->day - dt2->day;
	
	
	return 0;
}


void TimeReset() {
	dt.hsec = 0;
	dt.sec = 0;
	dt.min = 0;
	dt.hour = 0;
}