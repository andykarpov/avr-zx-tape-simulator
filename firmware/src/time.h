#ifndef _DATETIME_H_
#define _DATETIME_H_

#include "config.h"

#include "pcf8563.h"
#include "pcf8583.h"

/************************************************************************/
/* �������� ������� �����                                               */
/************************************************************************/
void TimeInit();

/**
 * ���������� ������� ���� � �����
 */
datetime_t* GetDateTime();


/**
 * �������������� ����� �� 0.01 �������
 */
void DateTimeTick100();

/**
 * ���������� ���������� ���� � ������
 */
uint8_t GetDaysPerMonth(datetime_t *dt);

/************************************************************************/
/* ���������� ������� �����                                             */
/* ������������ ��� ����������� ����� �� ������� �������� � ����������  */
/************************************************************************/
void TimeReset();


#endif	// _DATETIME_H_