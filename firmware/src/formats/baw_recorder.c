/*
 * baw_recorder.c
 *
 * Created: 16.10.2015 23:10:55
 *  Author: Trol
 */ 

#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "baw_recorder.h"
#include "baw_player.h"
#include "tap_recorder.h"
#include "baw.h"
#include "../fileio.h"
#include "../debug.h"
#include "../beeper.h"
#include "../general.h"
#include "../config.h"
#include "../ui.h"
#include "../time.h"
#include "../settings.h"




#define get_tape_in_level()		((PIND & _BV(4)) ? 0x80 : 0)

volatile uint8_t bawLastLevel;

// Счетчик для автостопа. Если автостоп отключен, устанавливается в 0xffff. 
// Иначе, содержит количество сэмплов, деленное на 0x7f оставшееся до автоматического прекращения записи при неизменном 
// уровне сигнала
volatile uint16_t bawAutoStopCounter;
volatile uint16_t bawAutoStopInitCounter;

extern volatile uint8_t bawLength;

bool BawSaveWriteBlocksDescriptor();

bool BawSave(const char* fileName) {
	TapSaveCd();
	if (!FioCreateFile(fileName)) {
		return false;
	}
	
	// добавляем кластеры к файлу
//	if (!FioAllocSize(2L*1024*1024)) {
//		return false;
//	}		  
			  
	//uint16_t sampleRate = BAW_SAMPLE_RATE;
	uint16_t sampleRate = GetBawSampleRate();
	uint8_t instructionsDelta = GetBawInstructionsDelta();
	MSG_DEC("sample rate ", sampleRate);
	MSG_DEC("delta ", instructionsDelta);
	
	FioPutDword(BAW_SIGNATURE);
	FioPutWord(BAW_FILE_VERSION);
	FioPutWord(sampleRate);
	
	bawLength = 1;
	bawLastLevel = get_tape_in_level();	// для метода BawSaveIsBegan


	if (GetBawAutoStop()) {
		uint32_t asc = GetBawAutoStopInterval();
		asc *= sampleRate;
		asc /= 1000;
		asc /= 0x7f;
		bawAutoStopCounter = asc;//GetBawAutoStopInterval() * sampleRate / 1000 / 0x7f;
		bawAutoStopInitCounter = asc;
		timer1compHandler = &BawRecorderNextSampleWithAutostop;
	} else {
		bawAutoStopCounter = 0xffff;
		bawAutoStopInitCounter = 0xffff;
		timer1compHandler = &BawRecorderNextSampleNoAutostop;
	}
	// настраиваем TC1, канал A	

	TCCR1A = 0;				// нормальный режим для таймера 3
	TCNT1 = 0;				// начальное значение таймера
	//OCR1A = F_CPU / BAW_SAMPLE_RATE;
	OCR1A = ((uint32_t)F_CPU - (uint32_t)sampleRate*instructionsDelta) / sampleRate;
	MSG_DEC("OCR1A ", OCR1A);
	

	return true;
}


void BawSaveStart() {
	bawLastLevel = get_tape_in_level();
	datetime_t *dt = GetDateTime();
	dt->min = 0;
	dt->sec = 0;
	dt->hsec = 0;
	TIMSK |= _BV(OCIE1A);	// прерывание по совпадению для таймера
	TCCR1B = _BV(CS10);		// пуск без делителя частоты
}


// TODO -> assembler !!!!
void BawRecorderNextSampleWithAutostop() {
	TCNT1 = 0;
	uint8_t level = get_tape_in_level();
	if (level == bawLastLevel) {
		bawLength++;
		if (bawLength >= 0x7f) {
			FioPutByte(bawLength + bawLastLevel);
			bawLength = 0;
			if (!(HIGH(bawAutoStopCounter) & _BV(7))) {
				// если автостоп включен, то при истечении интервала ожидания в этой переменной будет значение 0xFFFF
				// это служит сигналом для прекращения записи в обработчике onIdle())
				bawAutoStopCounter--;
			}
		}
	} else {
		FioPutByte(bawLength + bawLastLevel);
		if (level) {
			bawAutoStopCounter = bawAutoStopInitCounter;
			beeper_on();
		} else {
			beeper_off();
		}
		bawLastLevel = level;	// TODO тут, возможно, можно оптимизировать вынося в if
		bawLength = 1;
	}
}


void BawRecorderNextSampleNoAutostop() {
	TCNT1 = 0;
	uint8_t level = get_tape_in_level();
	if (level == bawLastLevel) {
		bawLength++;
		if (bawLength >= 0x7f) {
			FioPutByte(bawLength + bawLastLevel);
			bawLength = 0;
		}
	} else {
		FioPutByte(bawLength + bawLastLevel);
		if (level) {
			beeper_on();
		} else {
			beeper_off();
		}
		bawLastLevel = level;	// TODO тут, возможно, можно оптимизировать вынося в if
		bawLength = 1;
	}
}


bool BawSaveStop(bool cancel) {
	MSG("baw_stop");
	timer1compHandler = &EmptyVoidProc;
	TIMSK &= ~_BV(OCIE1A); // отключаем прерывание по совпадению для таймера
	TCCR1B &= ~_BV(CS10);	// выключаем таймер
	beeper_off();
	FioPutByte(0);
	SetScreen(SCREEN_PLEASE_WAIT);
	Draw();
	FioResizeFile();
	bool result = cancel || BawSaveWriteBlocksDescriptor();
	//uint32_t fileSize = 
	FioResizeFile();
	FioCloseFile();
	return result;
}


void BawSaveCancel() {
	BawSaveStop(true);
	// TODO удалить файл
}


// дописывает информацию о блоках к сохраненному файлу 
bool BawSaveWriteBlocksDescriptor() {
	FioFlush();
	FioSetReadMode();
	
	if (!FioSeekFromStart(8)) {
		return false;
	}
	uint32_t size = FioGetFileSize();
	MSG_DEC("BAW_DATA_SIZE ", size);
	
// #define MIN_BLOCKS_GAP_DURATION	(BAW_SAMPLE_RATE/5)	// 0.25 сек
	uint32_t minBlockGapDuration = GetBawSampleRate();
	minBlockGapDuration = minBlockGapDuration * GetBawMinBlockInterval() / 1000;
	MSG_DEC("MIN GAP DURATION ", minBlockGapDuration);
	//sampleRate*interval / 1000
			  

	uint8_t val = FioReadByte();
	uint32_t t = val & ~_BV(7);	// время в сэмплах
	uint32_t pauseDuration = 0;
	uint32_t blockStartOffset = 8;
	uint32_t blockStartTime = 0;
	uint8_t blocksCount = 0;
	for (uint32_t offset = 8+1; offset < size; offset++) {
		uint8_t newVal = FioReadByte();
		if ((offset % 512) == 0) {
			wdt_reset();
		}
		t += newVal & ~_BV(7);
		if ((newVal & _BV(7)) == (val & _BV(7))) {
			pauseDuration += newVal & ~_BV(7);
		} else {
			// если закончилась пауза
			if (pauseDuration >= minBlockGapDuration) {
				MSG_DEC("B-T ", t);
				MSG_DEC("B-ST ", blockStartTime);
				MSG_DEC("B-PAUSE ", pauseDuration);
				uint32_t blockDuration = t - blockStartTime - pauseDuration/2;
				uint64_t blockDuration64 = blockDuration;
				blockDuration64 *= 1000;
				blockDuration64 /= GetBawSampleRate();
				blockDuration = (uint32_t)blockDuration64;
				//blockDuration = blockDuration*1000/BAW_SAMPLE_RATE;		ТУТ ВОЗНИКАЛО ПЕРЕПОЛНЕНИЕ
				//blockDuration = blockDuration*10/441;	// TODO !!! magic const !!!!
													//	1000/44100
				uint32_t nextBlockOffset = offset - pauseDuration / 2 / 127;
				uint32_t blockSize = nextBlockOffset - blockStartOffset;
				blocksCount++;
				MSG_DEC("B-OFFSET   ", blockStartOffset);
				MSG_DEC("B-DURATION ", blockDuration);
				MSG_DEC("B-SIZE ", blockSize);
				
				// переходим в конец файла и добавляем полученные данные
				FioSetWriteMode();
				if (!FioSeekFromEnd(0)) {
					return false;
				}
				FioPutDword(blockStartOffset);
				FioPutDword(blockSize);
				FioPutDword(blockDuration);
				FioFlush();
				FioSetReadMode();
				if (!FioSeekFromStart(offset+1)) {
					return false;
				}
				
				blockStartOffset = nextBlockOffset;
				blockStartTime = t - pauseDuration / 2;
			}
			pauseDuration = 0;
		}
		val = newVal;
	}
	
	FioSetWriteMode();
	// последний байт файл - количество блоков
	if (!FioSeekFromEnd(0)) {
		return false;
	}
	FioPutByte(blocksCount);

	FioFlush();
	FioSetReadMode();
	
	return true;
}

bool BawSaveIsBegan() {
	return get_tape_in_level() != bawLastLevel;
}
