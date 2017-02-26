/*
 * wav_recorder.c
 *
 * Created: 11.10.2015 11:10:37
 *  Author: Trol
 */ 

#include "wav_recorder.h"
#include "wav_player.h"
#include "tap_recorder.h"
#include "../fileio.h"
#include "../debug.h"
#include "../beeper.h"
#include "../general.h"

#define get_tape_in_level()		(PIND & _BV(4))

uint32_t volatile samplesCount;
uint16_t sampleRate = 44100/2;

extern volatile uint16_t wavOcr;



bool WavSave(const char* fileName) {
	TapSaveCd();
	if (!FioCreateFile(fileName)) {
		return false;
	}
	
	// размер заголовка wav-файла - 44 байта, заполняем нулями
	for (uint8_t i = 0; i < 44; i++) {
		FioPutByte(0);
	}
	
	FioFlush();
	
	samplesCount = 0;
	
	// настраиваем TC1, канал A
	TCCR1A = 0;				// нормальный режим для таймера 3
	
	wavOcr = (uint16_t)((uint32_t)F_CPU / sampleRate);
	
	TCNT1 = 0;		// начальное значение таймера
	OCR1A = wavOcr;
	
	timer1compHandler = &WavRecorderNextSample;

	TIMSK |= _BV(OCIE1A);	// прерывание по совпадению для таймера
	TCCR1B = _BV(CS10);		// пуск без делителя частоты
	
	return true;
}


static void writeWavHeader() {
	FioFlush();
	FioSeekFromStart(0);
	
	FioPutDword(0x46464952);						// 0: 'RIFF'			[4]
	FioPutDword(samplesCount + 44 - 8);				// 4: chunk_size		[4]		= file_size - 8
	FioPutDword(0x45564157);						// 8: 'WAVE'			[4]
	FioPutDword(0x20746d66);						// 12: 'fmt '			[4]
	FioPutDword(0x10);								// 16: fmt_chunk_size	[4]		= 0x10
	FioPutWord(1);									// 20: audio_format		[2]		= 1 (PCM, no compression)
	FioPutWord(1);									// 22: num_channels		[2]		= 1 (mono)
	FioPutDword(sampleRate);						// 24: sample_rate		[4]
	FioPutDword(sampleRate);						// 28: byte_rate		[4]		= sample_rate   (== sample_rate * num_channels * bits_per_sample/8)
	FioPutWord(1);									// 32: block_align		[2]*	= 1	(== num_channels * bits_per_sample/8)
	FioPutWord(8);									// 34: bits_per_sample	[2]		= 8
	FioPutDword(0x61746164);						// 36: 'data'			[4]
	FioPutDword(samplesCount);						// 40: data_cunk_size	[4]		= file_size - 44

	FioFlush();
}


void WavRecorderNextSample() {
	OCR1A += wavOcr;
	//FioPutByte(get_tape_in_level() ? 0x7F : 0x8F);
	FioPutByte(get_tape_in_level() ? 0x01 : 0xFF);
	samplesCount++;
}


bool WavSaveStop() {
	TIMSK &= ~_BV(OCIE1A); // отключаем прерывание по совпадению для таймера
	BeeperInit();
	
	writeWavHeader();
	FioCloseFile();
	return true;
}


bool WavSaveIsBegan() {
	return true;
}
