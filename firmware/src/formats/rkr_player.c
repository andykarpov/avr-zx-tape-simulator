#include "rkr_player.h"

#include "../config.h"
#include "../debug.h"
#include "../fileio.h"
#include "../general.h"
#include "../beeper.h"
#include "../ui.h"
#include "../time.h"
#include "../settings.h"

#include <avr/io.h>
#include <avr/wdt.h>

#define SYNC_BYTE				0xE6
#define FILESIZE_LIMIT		0x7600
#define PADDING_BYTE			0
//#define PADDING_SIZE			0x100

//#define DEFAULT_BAUDRATE    1300


#define RKR_STATE_STARTING		0	// передача стартовых нулей
#define RKR_STATE_START_SYNC	1	// передача стартового синхробайта
#define RKR_STATE_DATA			2	// передача блока всех данных (без стартового синхробайта)
#define RKR_STATE_STOP_NULL	3	// передача нуля после всех данных
#define RKR_STATE_COMPLETE		4	// воспроизведение завершено но файл ещё не закрыт
#define RKR_STATE_FINISH		5	// воспроизведение полностью завершено или прервано, файл закрыт


volatile uint16_t rkr_remains;		// счётчик оставшихся байт
volatile uint8_t rkr_data_byte;		// текущий передаваемый байт
volatile uint8_t rkr_bit_counter;	// счётчик сэмплов в бите: 0..15
volatile uint16_t rkr_data_size;		// размер файла без стартового синхробайта
volatile uint8_t rkr_play_status;
uint16_t rkr_start_address;			// стартовый адрес, инициализируется функцией прелоада
uint16_t rkr_end_address;				// конечный адрес, инициализируется функцией прелоада

static void RkrPlayerTimerOverflowHandler_start_0();
static void RkrPlayerTimerOverflowHandler_sendSync();
static void RkrPlayerTimerOverflowHandler_data();
static void RkrPlayerTimerOverflowHandler_sendEnd();

uint8_t RkrFilePreload(const char *fileName) {
	uint8_t error;
	if (!FioOpenFile(fileName)) {
		MSG("can't open RKR file");
		error = RKR_STATUS_READ_ERROR;
		goto error;
	}
	uint8_t first = FioReadByte();
	
	bool has_start_sync_byte;
	if (first != SYNC_BYTE) {
		rkr_start_address = (first << 8) + FioReadByte();
		has_start_sync_byte = false;
	} else {
		has_start_sync_byte = true;
		rkr_start_address = FioReadByte() << 8;
		rkr_start_address += FioReadByte();
	}
	rkr_end_address = FioReadByte() << 8;
	rkr_end_address += FioReadByte();
	
	uint32_t fileSize32 = FioGetFileSize();
	if (fileSize32 > FILESIZE_LIMIT) {
		MSG("file to big");
		error = RKR_STATUS_FORMAT_ERROR;
		goto error;
	}
	uint16_t fileSize = (uint16_t)fileSize32;
	uint16_t remain_data_bytes = fileSize - 10;
	if (!has_start_sync_byte) {
		remain_data_bytes++;
	}
	
	uint16_t crc_lo = 0;
	uint16_t crc_hi = 0;
	for (;;) {
		uint8_t b = FioReadByte();
		crc_lo += b;
		if (--remain_data_bytes == 0) {
			crc_lo &= 0xff;
			break;
		}
		crc_hi += b;
		crc_hi += (crc_lo >> 8) & 1;
		crc_lo &= 0xff;
		crc_hi &= 0xff;
	}
	uint16_t calculated_crc = crc_lo + (crc_hi << 8);
	
	uint8_t zero_1 = FioReadByte();
	uint8_t zero_2 = FioReadByte();
	uint8_t sync = FioReadByte();
	uint16_t read_crc = FioReadByte() << 8;
	read_crc += FioReadByte();
	
	MSG("");
	MSG_HEX("CRC-CALC = ", calculated_crc, 2);
	MSG_HEX("CRC-READ = ", read_crc, 2);
	if (zero_1) {
		MSG("!!!! ZERO-1");
		error = RKR_STATUS_FORMAT_ERROR;
		goto error;
	}
	if (zero_2) {
		MSG("!!!! ZERO-1");
		error = RKR_STATUS_FORMAT_ERROR;
		goto error;
	}
	if (sync != SYNC_BYTE) {
		MSG("!!!! SYNC-END");
		error = RKR_STATUS_FORMAT_ERROR;
		goto error;
	}
	
	MSG_DEC("has_start_sync ", has_start_sync_byte);
	MSG_DEC("first byte ", first);
	MSG_HEX("start_address ", rkr_start_address, 2);
	MSG_HEX("end_address ", rkr_end_address, 2);
	MSG_DEC("file_size ", fileSize);
	
	
	error = calculated_crc == read_crc ? RKR_STATUS_OK : RKR_STATUS_CRC_ERROR;
	
error:
	FioCloseFile();
	return error;
}



bool RkrPlayFile(const char *fileName) {
	if (!FioOpenFile(fileName)) {
		return false;
	}

	pause_mode = false;
	rkr_data_size = FioGetFileSize();
	
	// настраиваем TC3, канал A
	TCCR1A = 0;				// нормальный режим для таймера 1
	TCNT1 = 0;				// начальное значение таймера
	
	rkr_play_status = RKR_STATE_STARTING;
	
	uint16_t baudRate = GetRkrBaudrate();	// бит/сек
	uint16_t sampleRate = baudRate * 2;		// бит кодируется двумя значениями
	MSG_DEC("bps: ", sampleRate);

	OCR1A = F_CPU / sampleRate;
	
	rkr_bit_counter = 16;
	rkr_remains = GetRkrPadding();
	timer1compHandler = &RkrPlayerTimerOverflowHandler_start_0;
	
	GetDateTime()->min = 0;
	GetDateTime()->sec = 0;

	TIMSK |= _BV(OCIE1A);	// прерывание по совпадению для таймера
	TCCR1B = _BV(CS10);		// пуск без делителя частоты
	
	// Длительность в сек: data_size * 8 / baud_rate

	return true;
}


bool RkrPlayingInProgress() {
	if (rkr_play_status == RKR_STATE_COMPLETE) {
		RkrClose();
	}
	return rkr_play_status != RKR_STATE_FINISH;
}

// 0..100
uint8_t RkrGetPlayProgress() {
	uint16_t finished = 0;
	switch (rkr_play_status) {
		case RKR_STATE_STARTING:
			finished = (GetRkrPadding()-rkr_remains);
			break;
		case RKR_STATE_START_SYNC:
			finished = GetRkrPadding() + 1;
			break;
		case RKR_STATE_DATA:
			finished = GetRkrPadding() + 1 + (rkr_data_size - rkr_remains);
			break;			
		case RKR_STATE_STOP_NULL:			
		case RKR_STATE_COMPLETE:
		case RKR_STATE_FINISH:
			return 100;
	}
	uint16_t total = rkr_data_size + GetRkrPadding() + 2;
	uint32_t result = 100;
	result *= finished;
	result /= total;
	if (result > 100) {
		result = 100;
	}
	return result;
}



void RkrClose() {
	TIMSK &= ~_BV(OCIE1A); // отключаем прерывание по совпадению для таймера
	timer1compHandler = &EmptyVoidProc;
	beeper_off();
	clr_tape_out();
	
	FioCloseFile();
	rkr_play_status = RKR_STATE_FINISH;
	if (fio_error_mask) {
		UiShowError(ERROR_FIO_ERROR, SCREEN_BROWSE);
//	} else {
//		SetScreen(SCREEN_BROWSE);
	}
	Draw();
}

/*
#include <stdio.h>
void rkr_log(uint8_t val) {
	static FILE *f = 0;
	static int bitCnt = 0;

	if (!f) {
		f = fopen("out-rkr.dat", "wt");
	}
	if (val == 0xff) {
		fclose(f);
	} else if (val == 8) {
		fprintf(f, "\n");
		bitCnt = 0;
	} else {
		fprintf(f, "%i", val);
		if (++bitCnt == 2) {
			fprintf(f, "  ");
			bitCnt = 0;
		}
	}	
}
 */

//#define _out(level)	{if (level) {rkr_log(1);set_tape_out(); beeper_on();} else {rkr_log(0);clr_tape_out(); beeper_off();} }
#define _out(level)	{if (level) {set_tape_out(); beeper_on();} else {clr_tape_out(); beeper_off();} }


static void RkrPlayerTimerOverflowHandler_start_0() {
	TCNT1 = 0;
	// бит "0" кодируется перепадом "1" -> "0"
	_out(!(rkr_bit_counter & 1));
	if (--rkr_bit_counter == 0) {
//rkr_log(8);
		rkr_bit_counter = 16;
		if (--rkr_remains == 0) {
			rkr_data_byte = SYNC_BYTE;
			rkr_play_status = RKR_STATE_START_SYNC;
			timer1compHandler = &RkrPlayerTimerOverflowHandler_sendSync;
		}
	}
}


static void RkrPlayerTimerOverflowHandler_sendSync() {
	TCNT1 = 0;
//printf("%i\t\t bit=%i index=%i\n", rkr_bit_counter, rkr_data_byte & _BV(7) ? 1 : 0, (rkr_bit_counter & 1));
	if (!(rkr_bit_counter & 1)) {	// первая "половинка" бита
		_out(!(rkr_data_byte & _BV(7)));
	} else {								// вторая "половинка" бита
		_out(rkr_data_byte & _BV(7));
		rkr_data_byte <<= 1;
	}
	if (--rkr_bit_counter == 0) {
//rkr_log(8);		
		rkr_bit_counter = 16;
		rkr_remains = rkr_data_size;
		rkr_data_byte = FioReadByte();
		if (rkr_data_byte == SYNC_BYTE) {
			rkr_data_byte = FioReadByte();
			rkr_data_size--;
		}
		rkr_play_status = RKR_STATE_DATA;
		timer1compHandler = &RkrPlayerTimerOverflowHandler_data;
	}
}


static void RkrPlayerTimerOverflowHandler_data() {
	TCNT1 = 0;
	if (!(rkr_bit_counter & 1)) {	// первая "половинка" бита
		_out(!(rkr_data_byte & _BV(7)));
	} else {								// вторая "половинка" бита
		_out(rkr_data_byte & _BV(7));
		rkr_data_byte <<= 1;
	}
	if (--rkr_bit_counter == 0) {
//rkr_log(8);		
		rkr_bit_counter = 16;
		if (--rkr_remains != 0) {
			rkr_data_byte = FioReadByte();
		} else {
			rkr_play_status = RKR_STATE_STOP_NULL;
			timer1compHandler = &RkrPlayerTimerOverflowHandler_sendEnd;
		}
	}
}


static void RkrPlayerTimerOverflowHandler_sendEnd() {
	TCNT1 = 0;
	// бит "0" кодируется перепадом "1" -> "0"
	_out(!(rkr_bit_counter & 1));
	if (--rkr_bit_counter == 0) {
//rkr_log(8);		
		timer1compHandler = &EmptyVoidProc;
		rkr_play_status = RKR_STATE_COMPLETE;
		TIMSK &= ~_BV(OCIE1A); // отключаем прерывание по совпадению для таймера
		beeper_off();
		clr_tape_out();

//rkr_log(0xff);
	}
}

