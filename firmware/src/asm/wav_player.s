
/*
 * audio_io.s
 *
 * Created: 22.10.2015 0:13:46
 *  Author: Trol
 */ 

#include <avr/io.h>

.extern beeper_volume
.extern closeWav

.global WavPlayerNextSample


/****************************************************************/
//
/****************************************************************/
WavPlayerNextSample:
	out		_SFR_IO_ADDR(TCNT1H), r1
	out		_SFR_IO_ADDR(TCNT1L), r1
    
	
//	call	FioNextByte
	lds		ZL, fio_get_next_byte_handler
	lds		ZH, fio_get_next_byte_handler+1
	icall									// ZH, ZL, r24, r25

	sbrs		r24, 7
	rjmp		wav_player_set
	cbi		_SFR_IO_ADDR(PORTE), 3
	sts		OCR3BL, r1
	ret

wav_player_set:
	sbi		_SFR_IO_ADDR(PORTE), 3
	lds		r24, beeper_volume
	sts		OCR3BL, r24
	ret


