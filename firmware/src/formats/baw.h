/* 
 * File:   baw.h
 * Author: Trol
 *
 * Created on November 8, 2015, 5:15 PM
 */

#ifndef _BAW_H
#define _BAW_H

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// поправка для корректирования частоты таймера
#define BAW_INSTRUCTIONS_DELTA_44100		32

// версия файла
#define BAW_FILE_VERSION				1

// частота сэмплирования по умолчанию
//#define BAW_SAMPLE_RATE					44100

#define BAW_SAMPLE_RATE_1				8000
#define BAW_SAMPLE_RATE_2				11025
#define BAW_SAMPLE_RATE_3				16000
#define BAW_SAMPLE_RATE_4				22050
#define BAW_SAMPLE_RATE_5				32000
#define BAW_SAMPLE_RATE_6				44100



// сигнатура файла
#define BAW_SIGNATURE					0x56415742

// минимальная пауза между блоками в сэмплах
//#define MIN_BLOCKS_GAP_DURATION	(BAW_SAMPLE_RATE/5)	// 0.25 сек

uint8_t GetBawInstructionsDelta();



#endif	// _BAW_H

