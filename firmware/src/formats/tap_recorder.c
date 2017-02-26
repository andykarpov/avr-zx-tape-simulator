/*
 * tap_recorder.c
 *
 * Created: 12.06.2015 15:00:41
 *  Author: Trol
 */ 
#include "tap_recorder.h"

#include <avr/io.h>
#include <avr/wdt.h>

#include "../debug.h"
#include "../fileio.h"
#include "../settings.h"
#include "../beeper.h"


const char PATH_SAVES[] PROGMEM = "/SAVES";


// ������� ������������ �������, ����� ��� ����������� ��������� ����������� ������
//volatile uint32_t overflowCnt = 0;
volatile uint8_t status = TAPE_IN_DONE;

volatile uint8_t widthLead;	// ������ ��������� �����-���� (� 1/8 ����� ���)
volatile uint8_t widthSync;	// ������ ��������� ������������� (� 1/8 ����� ���)
volatile uint8_t widthData;	// ������ ��������� ���� 0 (� 1/8 ����� ���, ��� 1 ����� ��������� ������)

struct tap_saver_info progressInfo;


volatile uint32_t blockOffset;	// �������� ������ �������� ����� � ����� (��������� �� ������ ���� ��� �����)
volatile uint16_t blockLength;	// ������ �������� �����
//uint8_t _data[450];
volatile uint16_t _ptr = 0;
volatile uint8_t maxHandleTime;
	
volatile uint8_t minDataPulseWidth;
volatile uint8_t maxDataPulseWidth;

	
	
static void updateLastBlockSize();

void TapSaverCaptureHandler() {
	// The Input Capture is updated with the counter (TCNTn) value each time an event occurs on the ICPn pin ICR1
	uint16_t pulseLen16 = ICR1;			// ������������ �������� � �������� �������� (4 ���)
	TCNT1 = 0;		// Reset Timer1 Count	 Register
	
	// ��������� �������� ������� ��������
	if (pulseLen16 > 0xff*2) {
		return;
	}
	uint8_t pulseLen = pulseLen16/2;	// ������������ ��������, ��� ��������� � ��� ���� �������� �� 8
	
	// �����-���: ������ 568 ���, ������������ 1.7 ���
	// ������������-1: 172 ���
	// ������������-2: 192 ���
	// ���-1: 448 ���
	// ���-0: 224 ���

	static uint8_t b;		// �������� ����
	static uint8_t bitIndex;	// ����� ��������� ���� � �����
	
	switch (status) {
		case TAPE_IN_WAIT_BLOCK:
			status = TAPE_IN_PILOT;
			progressInfo.block++;
			progressInfo.lastBlockSize[1] = progressInfo.lastBlockSize[0];
			progressInfo.lastBlockSize[0] = 0;
			// ���������� ���� ������ ����� �����, ����� ��������� ���� ������ ��������
			FioPutByte(0x12);
			FioPutByte(0x34);			
//if (_ptr < sizeof(_data)) {
//_data[_ptr++] = 0xff;
//}
			break;
		case TAPE_IN_PILOT:
			if (widthLead == 0 && pulseLen > 150) {	//  ������ ������� �����
				widthLead = pulseLen;
			} else if (widthLead != 0 && pulseLen < widthLead / 2) {
				widthSync = pulseLen;
				b = 0;
				bitIndex = 0;
				status = TAPE_IN_DATA;
				blockOffset += blockLength;
				blockLength = 0;
			}
			break;
		case TAPE_IN_DATA:
			if (widthData == 0 && pulseLen >= 45 && pulseLen <= 150) {
				if (pulseLen < 45*2) {	// �� 300 ���
					widthData = pulseLen;
				} else {
					widthData = pulseLen / 2;
				}
				minDataPulseWidth = pulseLen * 2 / 3;
				maxDataPulseWidth = pulseLen * 2 * 3 / 2;
			}
			// ���� ������� ������� ���������� �����, ���������� ���
			if (pulseLen < minDataPulseWidth || pulseLen > maxDataPulseWidth) {
				goto exit;
			}
//if (_ptr < sizeof(_data))
//_data[_ptr++] = pulseLen;			
			if (pulseLen > widthData*3/2) {	// ������� ��� 1
				b |= 1;
			}
			bitIndex++;
			if (bitIndex >= 8) {
				// ��������� ��������� ���������� ����
				FioPutByte(b);
				blockLength++;
				b = 0;
				bitIndex = 0;
			} else {
				b <<= 1;
			}
			break;
	}
exit:	
	if (TCNT1 > maxHandleTime) {
		maxHandleTime = TCNT1;
	}
}


void TapSaverOverflowHandler() {
	switch (status) {
		case TAPE_IN_DATA:
			// ���� ����������, ��������� ��� (���������� �����)
			updateLastBlockSize();
			status = TAPE_IN_WAIT_BLOCK;
			break;
		case TAPE_IN_PILOT:
			// ���-�� ����� �� ���, ������
			status = TAPE_IN_ERROR;
	}	
}


bool TapSave(const char *fileName) {
	TapSaveCd();
	//FioInit();
	if (!FioCreateFile(fileName)) {
		return false;
	}
	
	// ������� ����������
	progressInfo.block = 0;
	progressInfo.lastBlockSize[0] = 0;
	progressInfo.lastBlockSize[1] = 0;
	
	// ������������� ������� �������
	TCCR1A = 0;	// �� ���������� ������ COMnA1
	TCCR1B |= _BV(ICNC1) | _BV(CS11) | _BV(CS10);	// ������ ����� �������, �������� �� 64 => 250 ���, ��� � 4 ���
	
	// ICES1 - ���� 0, �� ������ �� �����, 1 - ������ �� ������
	if (GetCapture() == CAPTURE_RISE) {
		TCCR1B |= _BV(ICES1);
	} else {
		TCCR1B &= ~_BV(ICES1);
	}

	TIFR = _BV(ICF1);		// Clear pending interrupts
	
	TIMSK |= _BV(TICIE1) | _BV(TOIE1);	// enable capture interrupt, enable overflow interrupt
	//ETIMSK |= _BV(TICIE3) | _BV(TOIE3);

maxHandleTime = 0;

	status = TAPE_IN_WAIT_BLOCK;
	blockOffset = 0;
	blockLength = 0;

	widthLead = 0;	// ������������� ��������� ��� ������ ������� �����
	widthData = 0;
	
	uint16_t t = (GetPilotPulseWidth() >> GetTapSpeed()) / 4;
	widthLead = t - t/10 - 1;
	
	t = (GetDataPulseWidth() >> GetTapSpeed()) / 4;
	widthData = t ;
	minDataPulseWidth = widthData * 2 / 3;
	maxDataPulseWidth = widthData * 2 * 3 / 2;
	
	return true;	
}

void TapSaveCd() {
	FioSetPath_P(PATH_SAVES);
}



uint32_t TapSaveStop() {
	// ��������� ����������
	TIMSK &= ~(_BV(TICIE1) | _BV(TOIE1));
	status = TAPE_IN_DONE;
	
	uint32_t fileSize = FioResizeFile();
	MSG_DEC("file_size ", fileSize);
	FioCloseFile();
	//FioClose();
	
	//uart_init();
	MSG_DEC("handle time ", maxHandleTime);
	MSG_DEC("Lead ", widthLead);
	MSG_DEC("Lead.us ", widthLead*4);
	MSG_DEC("Sync ", widthSync);
	MSG_DEC("Sync.us ", widthSync*4);
	MSG_DEC("Width ", widthData);
	MSG_DEC("Width.us ", widthData*4);
//	MSG_DEC("widthDetectPulse ", widthDetectPulse);
//	MSG_DEC("widthDetectPulseVal ", widthDetectPulseVal);
	MSG_DEC("size ", _ptr);
	MSG("-----");
//#if DEBUG			
	//for (uint16_t i = 0; i < _ptr; i++) {
		//if (_data[i] == 0xff) {
			//uart_putc('\n');
		//}		
		//uart_putw_dec(_data[i]);
		//uart_putc(' ');
		////MSG_DEC(" ", _data[i]);
	//}
//#endif						
	
	
	return fileSize;
}


void TapSaveCancel() {
	// TODO ������� ������ ���� !!!!!
	FioCloseFile();
	//FioClose();
	
	// ��������� ����������
	TIMSK &= ~(_BV(TICIE1) | _BV(TOIE1));
	status = TAPE_IN_DONE;
}


static void updateLastBlockSize() {
	// ���������� ������
	FioFlush();
	// ���������� ������� ������� � �����
	uint32_t offset;
	FioGetFilePos(&offset);
MSG("path_size");
MSG_DEC("offset = ", blockOffset);
MSG_DEC("size = ", blockLength);
	// ��������� �� ������ �����
	FioSeekFromStart(blockOffset);
	// ������ ������ (������� ������� ����, ����� - �������)
	FioPutByte(blockLength & 0xff);
	FioPutByte((blockLength >> 8) & 0xff);
	FioFlush();
	
//	FioClose();
//	FioOpenFile()
	
	// ��������������� �������
	FioSeekFromStart(offset);
	blockOffset += 2;
}

uint16_t cn = 0;
void TapSaveOnIdle() {
	if (status == TAPE_IN_DATA) {
		progressInfo.lastBlockSize[0] = blockLength;
	}
//	if (status != TAPE_IN_WAIT_BLOCK) {
//		MSG_DEC("s", status);
//	} 

	if (GetSoundRec() && status != TAPE_IN_DONE) {
		if (PIND & _BV(4)) {
			//BeeperOn();
			beeper_on();
		} else {
			//BeeperOff();
			beeper_off();
		}
	}
}

struct tap_saver_info* TapSaverGetProgress() {
	return &progressInfo;
}


uint8_t TapSaverGetStatus() {
	return status;
}
