
/*
 * audio_io.s
 *
 * Created: 22.10.2015 0:13:46
 *  Author: Trol
 */ 

#include <avr/io.h>

.extern beeper_volume
.extern bawLength

.global BawPlayerNextSample



/****************************************************************/
//
/****************************************************************/
BawPlayerNextSample:
	out		_SFR_IO_ADDR(TCNT1H), r1		// 1 clk
	out		_SFR_IO_ADDR(TCNT1L), r1		// 1 clk
	lds		r24, bawLength
	cpse		r24, r1
	rjmp		baw_player_exit

//	call	FioNextByte
	lds		ZL, fio_get_next_byte_handler
	lds		ZH, fio_get_next_byte_handler+1
	icall									// ZH, ZL, r24, r25
   
	cpse		r24, r1
	rjmp		baw_player_not_end
   jmp		closeBaw
baw_player_not_end:
	mov		r25, r24
	andi		r25, 0x7F
	sts		bawLength, r25
	sbrs		r24, 7
	rjmp		baw_player_clear_output
	sbi		_SFR_IO_ADDR(PORTE), 3
	lds		r24, beeper_volume
	sts		OCR3BL, r24
	rjmp		baw_player_exit
baw_player_clear_output:
	cbi		_SFR_IO_ADDR(PORTE), 3
	sts		OCR3BL, r1
baw_player_exit:
	lds		r24, bawLength
	subi		r24, 1
	sts		bawLength, r24
	ret

