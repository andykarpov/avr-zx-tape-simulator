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

// ��������� ������������ ��������� ������� � ��� � ����� ����� ��������
#define time_us_to_cnt(t)	(t/4)		// t * 16MHz / 64

#define init_cnt_value(t)	(255 - ((t/4) >> Speed))


volatile uint16_t blockSize;			// ������ ����� ������
volatile uint16_t leadToneCounter;	// ����� ������ �����-����
volatile uint8_t tapeOutMode;			// ����� ������ TAPE_OUT_xxx
volatile bool tapeOutput;				// ���������� ������

// ������� ���� �� ����� ���� ���������� (������������ � ��������� ����������)
volatile uint32_t loadProcessedBytesCount;

// ������� �������� ���� �� ��������� �������� ����� (������������ � ������ ����������)
volatile uint32_t remainLoadBytes;

// ����� �������� ������������ �����
volatile uint8_t currentBlockNumber;
volatile uint8_t numberOfBlocks;
volatile bool previousBlockIsHeader;

volatile uint8_t pilotPulseLen;
volatile uint8_t sync1PulseLen;
volatile uint8_t sync2PulseLen;
volatile uint8_t data0PulseLen;
volatile uint8_t data1PulseLen;

extern uint16_t playedFileDuration;


// ��������� � ��������� ����
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

	uint32_t offset = 0;		// ������� �������� � �����
	uint32_t pulsesCount = 0;	// ������� ��������� ������, ��������� ��� - ��� ��������, ������� - ����

	while (offset < FioGetFileSize()) {
		// ������ ����
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
			// ���� ����� �� ������� �����
			if (offset > FioGetFileSize()) {
MSG_DEC("offset ", offset);				
MSG_DEC("fs = ", FioGetFileSize());
				out->status = TAP_VERIFY_STATUS_FILE_CORRUPTED;
				break;
			}
			calculatedCrc ^= b;
			// ������� ������������ ����� � ���������
			for (uint8_t i = 0; i < 8; i++) {
				pulsesCount++;
				if (b & 128) {
					pulsesCount++;
				}
				b <<= 1;
			}
		}
		// TODO !!! ������� ��������� ������ ������
		// ������ � ��������� CRC
		uint8_t crcFromTapFile = FioReadByte();
		offset++;
		if (calculatedCrc != crcFromTapFile && out->firstInvalidBlock == 0) {
			out->status = TAP_VERIFY_STATUS_CRC_ERROR;
			out->firstInvalidBlock = numberOfBlocks;
		}
	}

	// ��������� ����� �������� �����
	// TODO ������� ����� ���������
	
	uint32_t totalTimeUs = (GetPauseDuration()*1000 + GetPilotBlockDuration() * 1000 + GetSync1PulseWidth() + GetSync2PulseWidth()) * numberOfBlocks;
	// ���������, ��� ����� ������-���������� ����� ������
	totalTimeUs -= numberOfShortBlocks * GetPauseDuration() * 1000;
	totalTimeUs += numberOfShortBlocks * GetShortPauseDuration() * 1000;
	totalTimeUs += pulsesCount * GetDataPulseWidth() * 2;
	totalTimeUs >>= GetTapSpeed();
	playedFileDuration = totalTimeUs / 1000000;
	
	FioCloseFile();
	
MSG_DEC("status ", out->status);	
}


// ��������� ���� � ��������������� �� ������ ����� (��������� �� 0)
bool TapFileOpen(const char* fileName, uint8_t block) {
	if (!FioOpenFile(fileName)) {
		return false;
	}
	FioOnIdle();
	loadProcessedBytesCount = 0;
	// ���������� ������ ����� ������
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
	// ����������� ������ T0
	timer1overflowHandler = &TapPlayerTimerOverflowHandler;
	TCCR0 |= _BV(CS02);	//����� ������� �������� ��������� �� 64
	TCNT0 = 0;		// ��������� �������� �������
	TIMSK |= _BV(TOIE0); // ���������� �� ������������ ������� (������ T0 8-������ � ������� �� ���������� �� 0xff)
	
	// ������ ����� �����
	blockSize = size;
	uint32_t t = 1000L * GetPilotBlockDuration() / GetPilotPulseWidth();
	leadToneCounter = t >> GetTapSpeed();

	tapeOutMode = TAPE_OUT_LEAD;
	TCNT0 = 0;		// ��������� �������� �������
}


// ���������� �� �������� ����� ��������� ��� ���������� �������� ���������������/������ TAP-�����
void TapOnIdle() {
	if (remainLoadBytes > 0 && (tapeOutMode == TAPE_OUT_STOP || tapeOutMode == TAPE_OUT_RESUME)) {
		// read next block
		// ����� ���� ��� �� ������ ���� ��� �� ������������� ���������������
		if (currentBlockNumber > 0 && (tapeOutMode != TAPE_OUT_RESUME)) {
			uint16_t pause = previousBlockIsHeader ? GetShortPauseDuration() : GetPauseDuration();
			uint16_t delay = (pause >> GetTapSpeed()) / 10;
			// ����
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
			// ���� �� ����� �������� �������� ���� �������� ��� ��������������, �� �������
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
	
	// ��������� ��������
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
	// ������������ ���� � ��������������� �� ������ ����
	char fileName[32];
	strcpy(fileName, GetOpenFileName());	
	FioCloseFile();
	
	if (currentBlockNumber > 0) {
		currentBlockNumber--;
	}
	TapFileOpen(fileName, currentBlockNumber);
	remainLoadBytes = FioGetFileSize() - loadProcessedBytesCount;
	// ����� ����� ���������������� � ����������� OnIdle()
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
	static uint8_t byte = 0;		// ���������� ����
	static uint8_t bitIndex = 0;	// ����� ����������� ����
	static uint16_t addr = 0;		// ������� �����
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
	// ������� �����-���
	if (tapeOutMode == TAPE_OUT_LEAD) {
		TCNT0 = pilotPulseLen;
		if (leadToneCounter > 0) {			// 3000 ��� ������� �������� -> 1.7 ���
			leadToneCounter--;
		} else {
			tapeOutMode = TAPE_OUT_SYNCHRO_1;
			return;
		}
	}
	// ������� ������������ 1
	if (tapeOutMode == TAPE_OUT_SYNCHRO_1) {
		TCNT0 = sync1PulseLen;
		tapeOutMode = TAPE_OUT_SYNCHRO_2;
		return;
	}
	// ������� ������������ 2
	if (tapeOutMode == TAPE_OUT_SYNCHRO_2) {
		TCNT0 = sync2PulseLen;
		tapeOutMode = TAPE_OUT_DATA;
		bitIndex = 16;
		byte = 0;
		addr = 0;
		return;
	}
	// ������� ������
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
		// ������� ���
		if (byte & 128) {
			TCNT0 = data1PulseLen;
		} else {
			TCNT0 = data0PulseLen;
		}

		// �� ������ ��� 2 ��������, ��������� � ���������� ����
		if ((bitIndex % 2) == 1)  {
			byte <<= 1;
		}
		bitIndex++;
		return;
	}
}
