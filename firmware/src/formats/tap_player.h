/*
 * tap.h
 *
 * Created: 05.06.2015 19:11:32
 *  Author: Trol
 */ 


#ifndef _TAP_PLAYER_H_
#define _TAP_PLAYER_H_


#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


struct tap_checker_out_struct {
//	uint8_t blocks;					// количество блоков
//	uint16_t time;						// длительность воспроизведени€ на стандартной скорости, сек
	uint8_t firstInvalidBlock;		// номер первого поврежденного блока или 0, если файл не поврежден
	uint8_t status;					// результат операции проверки TAP_VERIFY_xxx
};

#define TAP_VERIFY_STATUS_OK					0
#define TAP_VERIFY_STATUS_IO_ERROR			1
#define TAP_VERIFY_STATUS_FILE_CORRUPTED	2
#define TAP_VERIFY_STATUS_CRC_ERROR			3


void TapFilePreload(const char* fileName, struct tap_checker_out_struct *out);
//bool TapFileOpen(const char* fileName, uint8_t block);

void TapLoadStart();
void TapLoadStop();
void TapLoadPause();
void TapLoadResume();

//uint8_t TapGetCurrentBlock();
//void TapSetCurrentBlock(uint8_t block);

uint32_t TapGetProcessedBytes();

// ¬ызываетс€ из главного цикла программы дл€ выполнени€ итерации воспроизведени€/записи TAP-файла
void TapOnIdle();

void TapPlayerTimerOverflowHandler();

bool TapLoadIsPaused();


#endif // _TAP_PLAYER_H_