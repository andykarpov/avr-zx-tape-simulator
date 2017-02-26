/*
 * tap.c
 *
 * Created: 05.06.2015 20:20:25
 *  Author: Trol
 */ 
#include "tap_player.h"

#include <avr/io.h>
#include <avr/wdt.h>

#include <util/delay.h>

#include <string.h>

#include "../fileio.h"
#include "../debug.h"
#include "../settings.h"
#include "../beeper.h"
#include "../keyboard.h"
#include "../ui.h"
#include "../time.h"
#include "../general.h"


#define TAPE_OUT_LEAD			0
#define TAPE_OUT_SYNCHRO_1		1
#define TAPE_OUT_SYNCHRO_2		2
#define TAPE_OUT_DATA			3
#define TAPE_OUT_STOP			4
#define TAPE_OUT_FINISH			5
#define TAPE_OUT_PAUSE			6
#define TAPE_OUT_RESUME			7


//#define set_tape_out()	PORTE |= _BV(3)
//#define clr_tape_out()	PORTE &= ~_BV(3)

// переводит длительность интервала времени в мкС в число тиков счетчика
#define time_us_to_cnt(t)	(t/4)		// t * 16MHz / 64

#define init_cnt_value(t)	(255 - ((t/4) >> Speed))


volatile uint16_t blockSize;			// размер блока данных
volatile uint16_t leadToneCounter;	// время выдачи пилот-тона
volatile uint8_t tapeOutMode;			// режим вывода TAPE_OUT_xxx
volatile bool tapeOutput;				// выдаваемый сигнал

// сколько байт из файла было обработано (используется в отрисовке интерфейса)
volatile uint32_t loadProcessedBytesCount;

// сколько осталось байт до окончания загрузки файла (используется в логике загрузчика)
volatile uint32_t remainLoadBytes;

// номер текущего загружаемого блока
volatile uint8_t currentBlockNumber;
volatile uint8_t numberOfBlocks;
volatile bool previousBlockIsHeader;

volatile uint8_t pilotPulseLen;
volatile uint8_t sync1PulseLen;
volatile uint8_t sync2PulseLen;
volatile uint8_t data0PulseLen;
volatile uint8_t data1PulseLen;

extern uint16_t playedFileDuration;


// Загружает и проверяет файл
void TapFilePreload(const char* fileName, struct tap_checker_out_struct *out) {
	out->status = TAP_VERIFY_STATUS_OK;
//	out->blocks = 0;
//	out->time = 0;
	out->firstInvalidBlock = 0;
	
	numberOfBlocks = 0;
	playedFileDuration = 0;
	uint8_t numberOfShortBlocks = 0;

	if (!FioOpenFile(fileName)) {
		out->status = TAP_VERIFY_STATUS_IO_ERROR;
		return;
	}

	uint32_t offset = 0;		// текущее смещение в файле
	uint32_t pulsesCount = 0;	// счетчик импульсов данных, единичный бит - два импульса, нулевой - один

	while (offset < FioGetFileSize()) {
		// читаем блок
		uint8_t lo = FioReadByte();
		uint8_t hi = FioReadByte();
		offset += 2;

		if ((hi == 0 && lo == 0) || offset > FioGetFileSize()) {
MSG_DEC("offset ", offset);
MSG_DEC("fs = ", FioGetFileSize());
			out->status = TAP_VERIFY_STATUS_FILE_CORRUPTED;
			break;
		}
		wdt_reset();
		uint16_t sz = lo + (hi << 8);
		if (sz > FioGetFileSize() - offset) {
			out->status = TAP_VERIFY_STATUS_FILE_CORRUPTED;
			break;
		}
		if (sz == 0x13) {
			numberOfShortBlocks ++;
		}
		numberOfBlocks++;
		uint8_t calculatedCrc = 0;
		for (uint16_t blockOffset = 0; blockOffset < sz-1; blockOffset++) {
			uint8_t b = FioReadByte();

			offset++;
			// если вышли за границы файла
			if (offset > FioGetFileSize()) {
MSG_DEC("offset ", offset);				
MSG_DEC("fs = ", FioGetFileSize());
				out->status = TAP_VERIFY_STATUS_FILE_CORRUPTED;
				break;
			}
			calculatedCrc ^= b;
			// считаем длительность байта в импульсах
			for (uint8_t i = 0; i < 8; i++) {
				pulsesCount++;
				if (b & 128) {
					pulsesCount++;
				}
				b <<= 1;
			}
		}
		// TODO !!! сначала проверить ошибки чтения
		// читаем и проверяем CRC
		uint8_t crcFromTapFile = FioReadByte();
		offset++;
		if (calculatedCrc != crcFromTapFile && out->firstInvalidBlock == 0) {
			out->status = TAP_VERIFY_STATUS_CRC_ERROR;
			out->firstInvalidBlock = numberOfBlocks;
		}
	}

	// вычисляем время звучания файла
	// TODO вынести общие константы
	
	uint32_t totalTimeUs = (GetPauseDuration()*1000 + GetPilotBlockDuration() * 1000 + GetSync1PulseWidth() + GetSync2PulseWidth()) * numberOfBlocks;
	// учитываем, что после блоков-заголовков паузы короче
	totalTimeUs -= numberOfShortBlocks * GetPauseDuration() * 1000;
	totalTimeUs += numberOfShortBlocks * GetShortPauseDuration() * 1000;
	totalTimeUs += pulsesCount * GetDataPulseWidth() * 2;
	totalTimeUs >>= GetTapSpeed();
	playedFileDuration = totalTimeUs / 1000000;
	
	FioCloseFile();
	
MSG_DEC("status ", out->status);	
}


// Открывает файл и позиционируется на нужном блоке (нумерация от 0)
bool TapFileOpen(const char* fileName, uint8_t block) {
	if (!FioOpenFile(fileName)) {
		return false;
	}
	FioOnIdle();
	loadProcessedBytesCount = 0;
	// пропускаем нужное число блоков
	for (uint8_t b = 0; b < block; b++) {
		uint8_t lo = FioNextByte();
		uint8_t hi = FioNextByte();
		loadProcessedBytesCount += 2;
		uint16_t sz = lo + (hi << 8);
		for (uint16_t i = 0; i < sz; i++) {
			FioNextByte();
			loadProcessedBytesCount++;
			FioOnIdle();
		}
	}
	return true;
}



static void loadBlock(uint16_t size) {
	// настраиваем таймер T0
	timer1overflowHandler = &TapPlayerTimerOverflowHandler;
	TCCR0 |= _BV(CS02);	//режим деления тактовых импульсов на 64
	TCNT0 = 0;		// начальное значение таймера
	TIMSK |= _BV(TOIE0); // прерывание по переполнению таймера (таймер T0 8-битный и считает на увеличение до 0xff)
	
	// читаем длину блока
	blockSize = size;
	uint32_t t = 1000L * GetPilotBlockDuration() / GetPilotPulseWidth();
	leadToneCounter = t >> GetTapSpeed();

	tapeOutMode = TAPE_OUT_LEAD;
	TCNT0 = 0;		// начальное значение таймера
}


// Вызывается из главного цикла программы для выполнения итерации воспроизведения/записи TAP-файла
void TapOnIdle() {
	if (remainLoadBytes > 0 && (tapeOutMode == TAPE_OUT_STOP || tapeOutMode == TAPE_OUT_RESUME)) {
		// read next block
		// пауза если это не первый блок или не возобновление воспроизведения
		if (currentBlockNumber > 0 && (tapeOutMode != TAPE_OUT_RESUME)) {
			uint16_t pause = previousBlockIsHeader ? GetShortPauseDuration() : GetPauseDuration();
			uint16_t delay = (pause >> GetTapSpeed()) / 10;
			// ждем
			volatile datetime_t *dt = GetDateTime();
			uint16_t time0 = (dt->min * 60*100) + (dt->sec * 100) + dt->hsec;
			uint8_t prevHsec = dt->hsec;
			while (dt->min * 60*100 + dt->sec * 100 + dt->hsec - time0 < delay) {
				if (dt->hsec != prevHsec) {
					wdt_reset();
					KeyboardCheck();
					UiOnIdle();
					prevHsec = dt->hsec;
				}
			}
			// если за время задержки загрузка была прервана или приостановлена, то выходим
			if (tapeOutMode == TAPE_OUT_PAUSE || tapeOutMode == TAPE_OUT_FINISH || remainLoadBytes == 0) {
				return;
			}
		}
		tapeOutMode = TAPE_OUT_STOP;
		MSG("\n ----- load -----");
		uint8_t lo = FioNextByte();
		uint8_t hi = FioNextByte();
		loadProcessedBytesCount += 2;
		uint16_t sz = lo + (hi << 8);
		remainLoadBytes -= sz + 2;
		currentBlockNumber++;
		previousBlockIsHeader = sz == 0x13;
		MSG_DEC("load block ", sz);
		loadBlock(sz);
	} else if (remainLoadBytes == 0 && (tapeOutMode == TAPE_OUT_STOP || tapeOutMode == TAPE_OUT_RESUME)) {
		TapLoadStop();
//		FioCloseFile();
//		if (GetSoundPlay()) {
//			//BeeperOff();
//			beeper_off();
//		}
//		tapeOutMode = TAPE_OUT_FINISH;
	}
}



void TapLoadStart() {
	currentBlockNumber = 0;
	previousBlockIsHeader = false;
	tapeOutMode = TAPE_OUT_STOP;
	loadProcessedBytesCount = 0;
	remainLoadBytes = FioGetFileSize();
	
	// вычисляем тайминги
	uint8_t speed = GetTapSpeed();
	
	uint16_t t = GetPilotPulseWidth() / 4;
	pilotPulseLen = 0xff - (t >> speed);
	
	t = GetSync1PulseWidth() / 4;
	sync1PulseLen = 0xff - (t >> speed);
	
	t = GetSync2PulseWidth() / 4;
	sync2PulseLen = 0xff - (t >> speed);
	
	t = GetDataPulseWidth() / 4;
	data0PulseLen = 0xff - (t >> speed);
	data1PulseLen = 0xff - (2*t >> speed);
	
	pause_mode = false;
}


void TapLoadStop() {
	timer1overflowHandler = &EmptyVoidProc;
	MSG("tap-stop");
	tapeOutMode = TAPE_OUT_FINISH;
	remainLoadBytes = 0;
	if (GetSoundPlay()) {
		//BeeperOff();
		beeper_off();
	}
	tapeOutput = false;
	FioCloseFile();
}

void TapLoadPause() {
	MSG("tap-pause");
	pause_mode = true;
	tapeOutMode = TAPE_OUT_PAUSE;
	if (GetSoundPlay()) {
		//BeeperOff();
		beeper_off();
	}
	tapeOutput = false;
}

void TapLoadResume() {
	MSG("tap-resume");
	pause_mode = false;
	// перечитываем файл и позиционируемся на нужный блок
	char fileName[32];
	strcpy(fileName, GetOpenFileName());	
	FioCloseFile();
	
	if (currentBlockNumber > 0) {
		currentBlockNumber--;
	}
	TapFileOpen(fileName, currentBlockNumber);
	remainLoadBytes = FioGetFileSize() - loadProcessedBytesCount;
	// номер блока инкрементируется в обработчике OnIdle()
	tapeOutMode = TAPE_OUT_RESUME;
}

//uint8_t TapGetCurrentBlock() {
//	return currentBlockNumber;
//}
//
//void TapSetCurrentBlock(uint8_t block) {
//	currentBlockNumber = block;
//}

uint32_t TapGetProcessedBytes() {
	return loadProcessedBytesCount;
}


bool TapLoadIsPaused() {
	return tapeOutMode == TAPE_OUT_PAUSE;
}

void TapPlayerTimerOverflowHandler() {
	static uint8_t byte = 0;		// выдаваемый байт
	static uint8_t bitIndex = 0;	// номер выдаваемого бита
	static uint16_t addr = 0;		// текущий адрес
	TCNT0 = 0;

	if (tapeOutMode >= TAPE_OUT_STOP) {	// STOP | FINISH | PAUSE | RESUME
		clr_tape_out();
		return;
	}
	if (tapeOutput) {
		set_tape_out();
		if (GetSoundPlay()) {
			//BeeperOn();
			beeper_on();
		}
		tapeOutput = false;
	} else {
		clr_tape_out();
		if (GetSoundPlay()) {
			//BeeperOff();
			beeper_off();
		}
		tapeOutput = true;
	}
	// выводим пилот-тон
	if (tapeOutMode == TAPE_OUT_LEAD) {
		TCNT0 = pilotPulseLen;
		if (leadToneCounter > 0) {			// 3000 для обычной скорости -> 1.7 сек
			leadToneCounter--;
		} else {
			tapeOutMode = TAPE_OUT_SYNCHRO_1;
			return;
		}
	}
	// выводим синхросигнал 1
	if (tapeOutMode == TAPE_OUT_SYNCHRO_1) {
		TCNT0 = sync1PulseLen;
		tapeOutMode = TAPE_OUT_SYNCHRO_2;
		return;
	}
	// выводим синхросигнал 2
	if (tapeOutMode == TAPE_OUT_SYNCHRO_2) {
		TCNT0 = sync2PulseLen;
		tapeOutMode = TAPE_OUT_DATA;
		bitIndex = 16;
		byte = 0;
		addr = 0;
		return;
	}
	// передаём данные
	if (tapeOutMode == TAPE_OUT_DATA) {
		if (bitIndex >= 16) {
			if (addr >= blockSize) {
				tapeOutMode = TAPE_OUT_STOP;
				return;
			}
			bitIndex = 0;
			byte = FioNextByte();
			loadProcessedBytesCount++;
			addr++;
		}
		// выводим бит
		if (byte & 128) {
			TCNT0 = data1PulseLen;
		} else {
			TCNT0 = data0PulseLen;
		}

		// на каждый бит 2 импульса, переходим к следующему биту
		if ((bitIndex % 2) == 1)  {
			byte <<= 1;
		}
		bitIndex++;
		return;
	}
}
