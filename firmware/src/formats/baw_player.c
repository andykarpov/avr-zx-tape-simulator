/*
 * baw_player.c
 *
 * Created: 16.10.2015 22:03:10
 *  Author: Trol
 */ 

#include <avr/wdt.h>
#include <avr/interrupt.h>

#include "baw_player.h"
#include "baw.h"
#include "baw_player.h"

#include "../fileio.h"
#include "../debug.h"
#include "../beeper.h"
#include "../ui.h"
#include "../fileio.h"
#include "../general.h"
#include "../time.h"

//bool BawTestBlocksDescriptor();	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

volatile uint8_t bawLength;
volatile bool pause_mode;

extern uint8_t currentBlockNumber;
extern uint8_t numberOfBlocks;

extern char fio_data[DIR_DATA_SIZE];	// вместо информации о содержимом директории будем хранить длительности блоков (по 2 байта на блок в сотых долях секунды)

volatile uint32_t bawNextBlockStartTime;	// время с которого начинается воспроизведение следующего блока (в сотых долях секунды)

//#define set_tape_out()	PORTE |= _BV(3)
//#define clr_tape_out()	PORTE &= ~_BV(3)




uint8_t playBaw(const char* fileName) {
	if (!FioOpenFile(fileName)) {
		return PLAY_BAW_IO_ERROR;
	}
	FioEnableFastReadonlyMode();
	// 0: 'BWAV' - сигнатура
	if (FioReadDword() != BAW_SIGNATURE) {
		MSG("ERROR BAW 'BWAV'");
		goto wrong_format;
	}
	// 4: version
	//uint16_t version = 
	FioReadWord();	// version + reserved
	// 6: sample_rate			[2]
	uint16_t sampleRate = FioReadWord();
	
	// настраиваем TC3, канал A
	TCCR1A = 0;				// нормальный режим для таймера 1

	MSG_DEC("bps: ", sampleRate);
	uint8_t instructionsDelta = GetBawInstructionsDelta();
	

	// 44100 -> 22.675 uS		OCR1A = 0x016a
	TCNT1 = 0;		// начальное значение таймера
	OCR1A = ((uint32_t)F_CPU - (uint32_t)sampleRate*instructionsDelta) / sampleRate;
	MSG_DEC("ocr ", OCR1A);
	timer1compHandler = &BawPlayerNextSample;
	
	bawLength = 0;
	currentBlockNumber = 1;
	pause_mode = false;

	TIMSK |= _BV(OCIE1A);	// прерывание по совпадению для таймера
	TCCR1B = _BV(CS10);		// пуск без делителя частоты

	return PLAY_BAW_OK;
	
wrong_format:
	MSG("WRONG FORMAT!");
	closeBaw();
	return PLAY_BAW_WRONG_FORMAT;
}


void closeBaw() {
	MSG("closevBaw");
	TIMSK &= ~_BV(OCIE1A); // отключаем прерывание по совпадению для таймера
	timer1compHandler = &EmptyVoidProc;
	beeper_off();
	clr_tape_out();
	wdt_reset();
	FioCloseFile();
}



bool BawPlayingInProgress() {
	return TIMSK & _BV(OCIE1A);
}

uint16_t BawGetDuration(const char *fileName) {
	wdt_reset();
MSG_STR("open ", fileName);	
	if (!FioOpenFile(fileName)) {
		return BAW_PLAYER_IO_ERROR;
	}
MSG_STR("opened ", fileName);	
	// 0: 'BWAV' - сигнатура	[4]
	if (FioReadDword() != 0x56415742) {
		MSG("ERROR BAW 'BWAV'");
		goto error;
	}
	// 4: version				[2]
	uint16_t version = FioReadWord();
	if (version > 1) {
		MSG("ERROR BAW VERSION");
		goto error;
	}
	// 6: sample_rate			[2]
	uint16_t sampleRate = FioReadWord();
	if (sampleRate > 50000) {
		MSG("ERROR BAW SAMPLE RATE");
		goto error;
	}
	
	if (!FioSeekFromEnd(1)) {
		MSG("SEEK 1 ERROR");
		goto error;
	}
	MSG_DEC("sample rate ", sampleRate);
	uint8_t lastByte = FioReadByte();
	MSG_DEC("blocks ", lastByte);
	
	// если информация о блоках в конце не найдена
	if (lastByte == 0) {
		numberOfBlocks = 1;
		if (!FioSeekFromStart(8)) {
			MSG("SEEK 8 ERROR");
			goto error;
		}
		uint32_t pulseCnt = 0;
		while (true) {
			wdt_reset();

			uint8_t v = FioReadByte();
			if (v == 0) {
				break;
			}
			pulseCnt += v & (~_BV(7));
		}
		wdt_reset();
//BawTestBlocksDescriptor();	// !!!!!!!!!!!!!!!!!!!
		MSG("close no blocks");
		FioCloseFile();

		uint32_t duration = pulseCnt / sampleRate;

		// поскольку при вычислении таймингов мы округляем делитель до наименьшего целого, получаем накапливающуюся
		// погрешность вычисления длительности. выражение ниже осуществляет корректировку времени по эмпирически 
		// подобранному коэффициенту для частоты 44100Гц
		duration += duration/190;

		return (uint16_t)duration;		
	}
	
	// суммируем длину блоков
	numberOfBlocks = lastByte;
	if (!FioSeekFromEnd(12*lastByte+1)) {
		MSG("SEEK block ERROR");
		goto error;
	}
	uint32_t duration = 0;

	for (uint8_t block = 0; block < numberOfBlocks; block++) {
		uint32_t offset = FioReadDword();	// offset
		uint32_t size = FioReadDword();	// size
		uint32_t time = FioReadDword();
		MSG_DEC("TIME ", time);
		MSG_DEC("   OFFSET ", offset);
		MSG_DEC("   SIZE ", size);
		
		// поскольку при вычислении таймингов мы округляем делитель до наименьшего целого, получаем накапливающуюся
		// погрешность вычисления длительности. выражение ниже осуществляет корректировку времени по эмпирически 
		// подобранному коэффициенту для частоты 44100Гц
		// TODO	!!!!!!
//		time += time/190;
		duration += time;
		
		uint16_t* ptr = (uint16_t*)(fio_data + block*2);
		(*ptr) = time/10;
	}
	bawNextBlockStartTime = *((uint16_t*)fio_data);
	MSG_DEC("NEXT_BLOCK_TIME ", bawNextBlockStartTime);
//BawTestBlocksDescriptor();	// !!!!!!!!!!!!!!!!!!!
	FioCloseFile();
	MSG("close-get-duration");
MSG_STR("close ", fileName);	
	
	return duration / 1000;
	
	
error:
	MSG("BAW OPEN ERROR !!!");
	FioCloseFile();
	return BAW_PLAYER_FORMAT_ERROR;
}

// block: 0..n-1
bool BawPlayerGotoBlock(uint8_t block) {
	uint16_t offset = 1 + (numberOfBlocks - block + 1) * 12;
	MSG_DEC("block ", block);
	MSG_DEC("file_offset ", offset);
	if (!FioSeekFromEnd(offset)) {
		return false;
	}
	uint32_t blockOffset = FioReadDword();
	MSG_DEC("->", blockOffset);
	if (!FioSeekFromStart(blockOffset)) {
		return false;		
	}
	readNext();
	return true;
}



void BawLoadPause() {
	MSG("pause");
	TIMSK &= ~_BV(OCIE1A); // отключаем прерывание по совпадению для таймера
	beeper_off();
	clr_tape_out();	
	pause_mode = true;
	
	for (uint8_t block = 0; block < numberOfBlocks; block++) {
		uint16_t* ptr = (uint16_t*)(fio_data + block*2);
		uint16_t time = (*ptr);
		MSG_DEC("BT ", time);
	}
}


bool BawLoadResume() {
	if (!BawPlayerGotoBlock(currentBlockNumber)) {
		MSG_DEC("CAN'T CHANGE BLOCK! ", currentBlockNumber);
		return false;
	}
	TIMSK |= _BV(OCIE1A); // прерывание по совпадению для таймера
	pause_mode = false;
	return true;
}



// При ручном переключении между блоками этот метод вызывается для чтения и обновления времени, прошедшего с начала воспроизведения
bool BawUpdatePlayerTime() {
	if (!FioSeekFromEnd(numberOfBlocks*12 + 1)) {
		MSG_DEC("SEET END ERROR ", numberOfBlocks*12 + 1);
		return false;
	}
	uint32_t duration = 0;
	for (uint8_t block = 1; block <= currentBlockNumber; block++) {
		FioReadDword();	// offset
		FioReadDword();	// size
		uint32_t tm = FioReadDword();
		if (block < currentBlockNumber) {
			duration += tm;
		} else {
			bawNextBlockStartTime = duration + tm;
		}
	}
	duration /= 1000;
	uint16_t d = duration;
	GetDateTime()->min = d / 60;
	GetDateTime()->sec = d % 60;
	GetDateTime()->year = GetDateTime()->min;
	GetDateTime()->month = GetDateTime()->sec;	
/*
	uint16_t offset = 1 + (numberOfBlocks - block + 1) * 12;
	MSG_DEC("block ", block);
	MSG_DEC("file_offset ", offset);
	if (!FioSeekFromEnd(offset)) {
		return false;
	}
	uint32_t blockOffset = FioReadDword();
	MSG_DEC("->", blockOffset);
	if (!FioSeekFromStart(blockOffset)) {
		return false;		
	}
 */ 
	return true;	
}


void BawPlayerIdle(uint16_t tm) {
	if (!pause_mode) {
		if (tm >= bawNextBlockStartTime && currentBlockNumber < numberOfBlocks) {
			currentBlockNumber++;
			bawNextBlockStartTime += *(uint16_t*)(fio_data + currentBlockNumber*2 - 2);
		}
	}
}


/*
bool BawTestBlocksDescriptor() {
	if (!FioSeekFromStart(8)) {
		return false;
	}
	uint32_t size = FioGetFileSize();
	MSG_DEC("BAW_DATA_SIZE ", size);
	uint32_t t = 0;					// время в сэмплах
	uint8_t val = FioReadByte();
	t += val & ~_BV(7);
	uint32_t pauseDuration = 0;
	uint32_t blockStartOffset = 8;
	uint32_t blockStartTime = 0;
	uint8_t blocksCount = 0;
uint32_t maxPauseDuration = 0;	
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
if (pauseDuration > maxPauseDuration) {
	maxPauseDuration = pauseDuration;
	MSG_DEC("MAX DURATION= ", maxPauseDuration);
}
			if (pauseDuration >= MIN_BLOCKS_GAP_DURATION) {
				uint32_t blockDuration = t - blockStartTime - pauseDuration/2;
				blockDuration = blockDuration*1000/BAW_SAMPLE_RATE;
				uint32_t nextBlockOffset = offset - pauseDuration / 2 / 127;
				uint32_t blockSize = nextBlockOffset - blockStartOffset;
				blocksCount++;
				MSG_DEC("B-OFFSET   ", blockStartOffset);
				MSG_DEC("B-DURATION ", blockDuration);
				MSG_DEC("B-SIZE ", blockSize);
				
//				// переходим в конец файла и добавляем полученные данные
//				FioSetWriteMode();
//				if (!FioSeekFromEnd(0)) {
//					return false;
//				}
//				FioPutDword(blockStartOffset);
//				FioPutDword(blockSize);
//				FioPutDword(blockDuration);
//				FioFlush();
//				FioSetReadMode();
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
	
	MSG_DEC("total blocks = ", blocksCount);
	
	return true;
}
*/