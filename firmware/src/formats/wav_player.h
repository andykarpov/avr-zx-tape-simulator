/*
 * wav_player.h
 *
 * Created: 03.10.2015 22:12:36
 *  Author: Trol
 */ 


#ifndef WAV_PLAYER_H_
#define WAV_PLAYER_H_


#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


#define PLAY_WAV_OK					0
#define PLAY_WAV_IO_ERROR			1
#define PLAY_WAV_WRONG_FORMAT		2

/*
																						5 917 430			6 425 609

	0: 'RIFF'				[4]		(0x52494646)	
	4: chunk_size			[4]*	(file size - 8)	 									5 917 412 (!)		6 425 601
	8: 'WAVE'				[4]
	
	12: 'fmt '				[4]
	16: fmt_chunk_size		[4]		= 0x10000000
	20: audio_format		[2]		= 1 (PCM, no compression)
	22: num_channels		[2]		= 1 (mono)
	24: sample_rate			[4]
	28: byte_rate			[4]		= sample_rate   (== SampleRate * NumChannels * BitsPerSample/8)
	32: block_align			[2]*	= 1	(== NumChannels * BitsPerSample/8)
	34: bits_per_sample		[2]		= 8
	
	36: 'data'
	40: data_cunk_size		[4]															5 917 377		6 425 565		(file_size - 44)
	44: data								01-FF (1,-1) / 7F-8F (127,-113)
											-128  .. 127
	
chunk_size:	36 + SubChunk2Size, or more precisely: 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
This is the size of the rest of the chunk following this number.  This is the size of the entire file in bytes minus 8 bytes for the 
two fields not included in this count: ChunkID and ChunkSize.	


data_chunk_size == NumSamples * NumChannels * BitsPerSample/8
This is the number of bytes in the data. You can also think of this as the size of the read of the subchunk following this number.

*/



/* Начинает проигрывание файла и возвращает код PLAY_WAV_xxx            */
uint8_t playWav(const char* fileName);

void closeWav();
void WavPlayerNextSample();


uint32_t WavGetSize();

#endif // WAV_PLAYER_H_ 