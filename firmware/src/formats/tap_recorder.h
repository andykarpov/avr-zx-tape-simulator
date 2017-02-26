/*
 * tap_saver.h
 *
 * Created: 04.06.2015 18:56:50
 *  Author: Trol
 */ 

#ifndef _TAP_SAVER_H_
#define _TAP_SAVER_H_


#define TAPE_IN_WAIT_BLOCK		0	// ожидаем блок данных
#define TAPE_IN_PILOT      		1	// читаем пилот-тон
#define TAPE_IN_DATA      		2	// читаем данные
#define TAPE_IN_ERROR			3	// что-то пошло не так
#define TAPE_IN_DONE			4	// запись завершена или не начиналась


#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


struct tap_saver_info {
	uint8_t block;	// текущий сохраняемый блок
	uint16_t lastBlockSize[2];	// длины последних загруженных блоков (0 - текущий, 1-предыдущий)
};


bool TapSave(const char *fileName);
// сохраняет файл и возвращает его размер
uint32_t TapSaveStop();
void TapSaveCancel();
void TapSaverCaptureHandler();
void TapSaverOverflowHandler();
void TapSaveOnIdle();
struct tap_saver_info* TapSaverGetProgress();
uint8_t TapSaverGetStatus();

// настройка модуля Fio на работу с сохраняемыми файлами
void TapSaveCd();


#endif 

