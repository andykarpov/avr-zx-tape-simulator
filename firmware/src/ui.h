/*
 * IncFile1.h
 *
 * Created: 02.06.2015 20:59:13
 *  Author: Trol
 */ 


#ifndef _UI_H_
#define _UI_H_

#include "config.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


#define SCREEN_MAIN_MENU					0
#define SCREEN_BROWSE						1	// ��������� �� ������
#define SCREEN_TAP_LOAD						2	// �������� ����� � ����� � Spectrum
#define SCREEN_TAP_SAVE						3	// ���������� ����� �� Spectrum-� �� �����
#define SCREEN_SETTINGS						4	// ����� ��������
#define SCREEN_ABOUT							5	// � ���������
#define SCREEN_WRONG_TAP					6	// ����� ���������� � ���, ��� TAP-���� ���������
#define SCREEN_WAIT_SAVE					7	// �������� ������� ��� ������ ����� �� Spectrum-�
#define SCREEN_SAVE_TAP_DONE				8	// ���������� � ����� ����� ����������
#define SCREEN_CARD_INFO					9	// ���������� � ����� ������
#define SCREEN_TAP_SETTINGS			10	// ��������� ��������� TAP
#define SCREEN_CONFIRM_DEFAULT_TIMES	11	// ������������� ������ �������� �������
#define SCREEN_KEYBOARD						12	// ����� �������� ����������
#define SCREEN_DATETIME						13	// ����� ����� ���� � �������
#define SCREEN_FILE_MENU					14	// ���� �������������� ������ ��� �����
#define SCREEN_OVERWRITE_FILE				15	// ������������� ���������� �����
#define SCREEN_LOADING						16	// ����� ��������
#define SCREEN_INSERT_CARD					17	// ����� "�������� �����"
#define SCREEN_SETUP_CONTRAST				18	// ����� ��������� ������������� �������
#define SCREEN_ERROR							19	// ����� ����������� ������ ������
#define SCREEN_PLAY_WAV						20	// ��������������� wav-�����
#define SCREEN_SELECT_SAVE_FORMAT		21	// ����� ������� ���������� �����
#define SCREEN_WAV_SAVE						22	// ������� ���������� wav-�����
#define SCREEN_PLAY_BAW						23	// ��������������� baw-�����
#define SCREEN_BAW_SAVE						24	// ������� ���������� baw-�����
#define SCREEN_PLEASE_WAIT					25	// ����� �������� (��������� ������ baw)
#define SCREEN_BAW_SETTINGS				26	// ��������� ��������� BAW
#define SCREEN_RKR_SETTINGS				27 // ��������� ��������� RKR
#define SCREEN_PLAY_RKR						28	// ����� ��������������� RKR

#define ERROR_CREATE_FILE					1	// i/o-������ ��� �������� �����
#define ERROR_READ_FILE						2	// i/o-������ ��� ������ �����
#define ERROR_FIO_ERROR						3	// i/o-������ ������ fio. ��������� �������� �������� � ���������� ������
#define ERROR_WRONG_FILE_FORMAT			4	// ������ ������� �����
#define ERROR_WRITE_FILE					5	// i/o-������ ��� ����������/�������� �����
#define ERROR_WRONG_CRC						6	// ������ crc


uint8_t GetScreen();
void SetScreen(uint8_t screen);

void Draw();

void onKeyPressed(uint8_t key);

void UiOnIdle();

const char * UiGetCurrentFileName();

void UiShowError(uint8_t msg, uint8_t backToScreen);

#endif	// _UI_H_