/*
 * config.h
 *
 * Created: 15.08.2015 22:05:48
 *  Author: Trol
 */ 



#ifndef _CONFIG_H_
#define _CONFIG_H_


#define F_CPU					16000000

#ifndef OPTIONS_FORM_MAKE_FILE

#	define USE_RTC_8563	1
#	define USE_RTC_8583	0
	

// ���� 1, �� ������������ ���������� ������� �������� ������� ��� ��� 2.56�. ����� - ���������� ������� 3.3�
// ������������ ��� ������ ����� 1.2 � ����.
#	define AREF_INTERNAL		1

#	define DISPLAY_HIGHLIGHT_INVERT		1

//!!!!

#endif

#ifndef LCD_CONTRAST
#	define LCD_CONTRAST	70
#endif

#endif // _CONFIG_H_ 