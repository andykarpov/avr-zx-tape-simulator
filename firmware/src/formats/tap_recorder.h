/*
 * tap_saver.h
 *
 * Created: 04.06.2015 18:56:50
 *  Author: Trol
 */ 

#ifndef _TAP_SAVER_H_
#define _TAP_SAVER_H_


#define TAPE_IN_WAIT_BLOCK		0	// ������� ���� ������
#define TAPE_IN_PILOT      		1	// ������ �����-���
#define TAPE_IN_DATA      		2	// ������ ������
#define TAPE_IN_ERROR			3	// ���-�� ����� �� ���
#define TAPE_IN_DONE			4	// ������ ��������� ��� �� ����������


#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


struct tap_saver_info {
	uint8_t block;	// ������� ����������� ����
	uint16_t lastBlockSize[2];	// ����� ��������� ����������� ������ (0 - �������, 1-����������)
};


bool TapSave(const char *fileName);
// ��������� ���� � ���������� ��� ������
uint32_t TapSaveStop();
void TapSaveCancel();
void TapSaverCaptureHandler();
void TapSaverOverflowHandler();
void TapSaveOnIdle();
struct tap_saver_info* TapSaverGetProgress();
uint8_t TapSaverGetStatus();

// ��������� ������ Fio �� ������ � ������������ �������
void TapSaveCd();


#endif 

