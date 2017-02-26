/*
 * wav_player.c
 *
 * Created: 10.10.2015 15:05:04
 *  Author: Trol
 */ 
#include "wav_player.h"

#include "../fileio.h"
#include "../debug.h"
#include "../beeper.h"
#include "../general.h"

#include "../ui.h"




volatile uint16_t wavOcr;
volatile uint32_t bytesRemains;


//static 
void closeWav();



uint8_t playWav(const char* fileName) {
MSG("playWav");	
	if (!FioOpenFile(fileName)) {
		return PLAY_WAV_IO_ERROR;
	}
	FioEnableFastReadonlyMode();
	// 0: 'RIFF' - сигнатура
	if (FioReadDword() != 0x46464952) {
		MSG("ERROR WAV 'RIFF'");
		goto wrong_format;
	}
	// 4: chunk_size (file size - 8)
	//uint32_t chunkSize = 
	FioReadDword();	// chunk size
	// 8: 'WAVE' - сигнатура
	if (FioReadDword() != 0x45564157) {
		MSG("ERROR WAV 'WAVE'");
		goto wrong_format;
	}
	
	// 12: 'fmt ' - блок
	if (FioReadDword() != 0x20746d66) {
		MSG("ERROR WAV 'fmt '");
		goto wrong_format;
	}	
	// 16: fmt_chunk_size		[4]		= 0x10
	uint32_t fmtChunkSize = FioReadDword();
	if (fmtChunkSize != 0x10) {
		MSG_HEX("ERROR WAV 'fmt_chunk_size' -> ", fmtChunkSize, 4);
		goto wrong_format;
	}
	// 20: audio_format		[2]		= 1 (PCM, no compression)
	uint16_t audioFormat = FioReadWord();
	if (audioFormat != 1) {
		MSG_DEC("ERROR WAV 'audio_format' -> ", audioFormat);
		goto wrong_format;
	}
	// 22: num_channels		[2]		= 1 (mono)
	if (FioReadWord() != 1) {
		MSG("ERROR WAV 'num_channels'");
		goto wrong_format;
	}
	// 24: sample_rate			[4]
	uint32_t sampleRate = FioReadDword();
	// 28: byte_rate			[4]		= sample_rate   (== sample_rate * num_channels * bits_per_sample/8)
	uint32_t byteRate = FioReadDword();
	if (sampleRate != byteRate) {
		MSG_DEC("ERROR WAV 'byte_rate' ", byteRate);
		goto wrong_format;
	}
	// 32: block_align			[2]*	= 1	(== NumChannels * BitsPerSample/8)
	//uint8_t blockAlign = 
	FioReadWord();	// block align
	// 34: bits_per_sample		[2]		= 8
	uint16_t bitPerSample = FioReadWord();
	if (bitPerSample != 8) {
		MSG_DEC("ERROR WAV 'bits_per_sample' ", bitPerSample);
		goto wrong_format;
	}
	// 36: 'data'
	if (FioReadDword() != 0x61746164) {
		MSG("ERROR WAV 'data'");
		goto wrong_format;
	}	
	// 40: data_cunk_size		[4]	(file_size - 44)
	uint32_t dataChunkSize = FioReadDword();

	MSG_DEC("bps: ", sampleRate);
	MSG_DEC("cnt: ", dataChunkSize);
	
	
	// настраиваем TC3, канал A
	TCCR1A = 0;				// нормальный режим для таймера 3	
	
	uint32_t ocr32 = F_CPU / sampleRate;
	wavOcr = (uint16_t) ocr32;
	
	bytesRemains = dataChunkSize;
	
	// 44100 -> 22.675 uS		OCR3A = 0x016a
	// 44199    22.625 
	TCNT1 = 0;		// начальное значение таймера
	OCR1A = wavOcr;
	
	timer1compHandler = &WavPlayerNextSample;
	fio_read_eof_handler = &closeWav;

	TIMSK |= _BV(OCIE1A); // прерывание по совпадению для таймера
	TCCR1B = _BV(CS10);		// пуск без делителя частоты

	return PLAY_WAV_OK;
	
wrong_format:
	closeWav();
	return PLAY_WAV_WRONG_FORMAT;
}


void closeWav() {
	TIMSK &= ~_BV(OCIE1A); // отключаем прерывание по совпадению для таймера
	BeeperInit();
	FioCloseFile();
	beeper_off();
	clr_tape_out();
	
	if (fio_error_mask) {
		UiShowError(ERROR_FIO_ERROR, SCREEN_BROWSE);
	} else {
		SetScreen(SCREEN_BROWSE);
	}
	Draw();
}

/*
extern bool fio_eof;

inline void WavPlayerNextSample() {
	TCNT1H = 0;
	TCNT1 = 0;
	//OCR3A += wavOcr;
	uint8_t b = FioNextByte();
	if (b & _BV(7)) {
		clr_tape_out();
		beeper_off();
	} else {
		set_tape_out();
		beeper_on();
	}
	//TCNT1 = 0;
	// если данные закончились
	if (fio_eof) {
//	if (--bytesRemains == 0) {
		closeWav();
	}
//	TCNT1 = 0;
}
*/

uint32_t WavGetSize() {
	return bytesRemains;
}