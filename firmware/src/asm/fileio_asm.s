
/*
 * fileio_asm.s
 *
 * Created: 10.10.2015 18:38:32
 *  Author: Trol
 */ 

 #include <avr/io.h>
 

#define FIO_ERROR_READ_NOT_READY			1
#define FIO_ERROR_WRITE_NOT_READY		2

.extern readNext
.extern FioNextByte

.extern fio_buf_part_pos
.extern fio_get_next_byte_handler
.extern sdcard_buf_1
.extern sdcard_buf_2
.extern fio_readyFlags
.extern fio_error_cnt
.extern fio_error_mask
 
.global FioReadDword
.global FioReadWord
.global FioPutByte
.global FioPutWord
.global FioPutDword

.global fioNextFromBuf1Start
.global fioNextFromBuf1End
.global fioNextFromBuf2Start
.global fioNextFromBuf2End
.global fioNextFromBuf1Wait
.global fioNextFromBuf2Wait

.global fioNextToBuf1Start
.global fioNextToBuf1End
.global fioNextToBuf2Start
.global fioNextToBuf2End
.global fioNextToBuf1Wait
.global fioNextToBuf2Wait
/****************************************************************/
//
/****************************************************************/
FioReadWord:
	call		readNext

	call		FioNextByte
	push		r24
	call		FioNextByte
	mov		r25, r24
	pop		r24
	ret		// r25:r24

/****************************************************************/
//
/****************************************************************/
FioReadDword:
	call		readNext

	call		FioNextByte
	push		r24
	call		FioNextByte
	push		r24
	call		FioNextByte
	push		r24
	call		FioNextByte
	mov		r25, r24
	pop		r24
	pop		r23
	pop		r22
	ret		// r25:r24:r23:r22

/****************************************************************/
//
/****************************************************************/
// void FioPutWord(uint16_t w)
// input:  r25:r24
FioPutWord:
	push		r25
	call		FioPutByte
	pop		r24
	jmp		FioPutByte


/****************************************************************/
//
/****************************************************************/
// void FioPutDword(uint32_t dw)
// input: r25:r24:r23:r22
FioPutDword:
	push		r25
	push		r24
	mov		r24, r22
	call		FioPutByte
	mov		r24, r23
	call		FioPutByte
	pop		r24
	call		FioPutByte
	pop		r24
	jmp		FioPutByte
// r22 -> r24
// r23 -> r24
// r24 -> r24
// r25 -> r24


/****************************************************************/
//
/****************************************************************/
fioNextFromBuf1Start:
	ldi		ZH, hi8(sdcard_buf_1)
	ldi		ZL, lo8(sdcard_buf_1)
	lds		r25, fio_buf_part_pos	
	add		ZL, r25
	adc		ZH, r1

	ld			r24, Z
	inc		r25
	sts		fio_buf_part_pos, r25

	breq		next_part_start_1
	ret
next_part_start_1:
	ldi		ZH, pm_hi8(fioNextFromBuf1End)
	ldi		ZL, pm_lo8(fioNextFromBuf1End)
	sts		fio_get_next_byte_handler+1, ZH
	sts		fio_get_next_byte_handler, ZL
	ret


/****************************************************************/
//
/****************************************************************/
fioNextFromBuf2Start:
	ldi		ZH, hi8(sdcard_buf_2)
	ldi		ZL, lo8(sdcard_buf_2)
	lds		r25, fio_buf_part_pos	
	add		ZL, r25
	adc		ZH, r1

	ld			r24, Z
	inc		r25
	sts		fio_buf_part_pos, r25

	breq		next_part_start_2
	ret
next_part_start_2:
	ldi		ZH, pm_hi8(fioNextFromBuf2End)
	ldi		ZL, pm_lo8(fioNextFromBuf2End)
	sts		fio_get_next_byte_handler+1, ZH
	sts		fio_get_next_byte_handler, ZL
	ret


/****************************************************************/
//
/****************************************************************/
fioNextFromBuf1End:
	ldi		ZH, hi8(sdcard_buf_1 + 0x100)
	ldi		ZL, lo8(sdcard_buf_1 + 0x100)
	lds		r25, fio_buf_part_pos	
	add		ZL, r25
	adc		ZH, r1

	ld			r24, Z
	inc		r25
	sts		fio_buf_part_pos, r25

	breq		next_part_end_1
	ret
next_part_end_1:

   lds		r25, fio_readyFlags
   andi		r25, 0xFE
	sts		fio_readyFlags, r25

	ldi		ZH, pm_hi8(fioNextFromBuf2Wait)
	ldi		ZL, pm_lo8(fioNextFromBuf2Wait)
	sts		fio_get_next_byte_handler+1, ZH
	sts		fio_get_next_byte_handler, ZL
	ret

	
/****************************************************************/
//
/****************************************************************/
fioNextFromBuf2End:
	ldi		ZH, hi8(sdcard_buf_2 + 0x100)
	ldi		ZL, lo8(sdcard_buf_2 + 0x100)
	lds		r25, fio_buf_part_pos	
	add		ZL, r25
	adc		ZH, r1

	ld			r24, Z
	inc		r25
	sts		fio_buf_part_pos, r25

	breq		next_part_end_2
	ret
next_part_end_2:

   lds		r25, fio_readyFlags
   andi		r25, 0xFD
	sts		fio_readyFlags, r25

   ldi		ZH, pm_hi8(fioNextFromBuf1Wait)
   ldi		ZL, pm_lo8(fioNextFromBuf1Wait)
   sts		fio_get_next_byte_handler+1, ZH
   sts		fio_get_next_byte_handler, ZL
	ret


/****************************************************************/
//
/****************************************************************/
fioNextFromBuf1Wait:
	lds		r24, fio_readyFlags
	sbrs		r24, 0
	rjmp		next_wait_1_error

   ldi		ZH, pm_hi8(fioNextFromBuf1Start)
   ldi		ZL, pm_lo8(fioNextFromBuf1Start)
   sts		fio_get_next_byte_handler+1, ZH
   sts		fio_get_next_byte_handler, ZL

	lds		r24, sdcard_buf_1
	ldi		r25, 1

//	ldi		ZH, hi8(sdcard_buf_1)
//	ldi		ZL, lo8(sdcard_buf_1)
//	lds		r25, fio_buf_part_pos			// TODO !!!! тут же должен быть 0 !!!!!
//	add		ZL, r25								// !!!!
//	adc		ZH, r1								// !!!!

//	ld			r24, Z
//	inc		r25
	sts		fio_buf_part_pos, r25

	ret

next_wait_1_error:
    lds		r24, fio_error_mask
    ori		r24, FIO_ERROR_READ_NOT_READY
    sts		fio_error_mask, r24
    lds		r24, fio_error_cnt				; fio_error_cnt++;
    lds		r25, fio_error_cnt+1
    adiw		r24, 1
    sts		fio_error_cnt+1, r25
    sts		fio_error_cnt, r24
    ldi		r24, 0x80					; при ошибке возвращается 0x80
    ret


/****************************************************************/
//
/****************************************************************/
fioNextFromBuf2Wait:
	lds		r24, fio_readyFlags
	sbrs		r24, 1
	rjmp		next_wait_2_error

   ldi		ZH, pm_hi8(fioNextFromBuf2Start)
   ldi		ZL, pm_lo8(fioNextFromBuf2Start)
   sts		fio_get_next_byte_handler+1, ZH
   sts		fio_get_next_byte_handler, ZL

	lds		r24, sdcard_buf_2
	ldi		r25, 1

//	ldi		ZH, hi8(sdcard_buf_2)
//	ldi		ZL, lo8(sdcard_buf_2)
//	lds		r25, fio_buf_part_pos				// TODO !!!! тут же должен быть 0 !!!!!
//	add		ZL, r25									// !!!!
//	adc		ZH, r1									// !!!!

//	ld			r24, Z
//	inc		r25
	sts		fio_buf_part_pos, r25

	ret

next_wait_2_error:
    lds		r24, fio_error_mask
    ori		r24, FIO_ERROR_READ_NOT_READY
    sts		fio_error_mask, r24
    lds		r24, fio_error_cnt				; fio_error_cnt++;
    lds		r25, fio_error_cnt+1
    adiw		r24, 1
    sts		fio_error_cnt+1, r25
    sts		fio_error_cnt, r24
    ldi		r24, 0x80					; при ошибке возвращается 0x80
    ret


/****************************************************************/
//
/****************************************************************/
fioNextToBuf1Start:
	ldi		ZH, hi8(sdcard_buf_1)
	ldi		ZL, lo8(sdcard_buf_1)
	lds		r25, fio_buf_part_pos
	add		ZL, r25
	adc		ZH, r1

	st			Z, r24
	inc		r25
	sts		fio_buf_part_pos, r25

	breq		put_next_part_start_1
	ret
put_next_part_start_1:
	ldi		ZH, pm_hi8(fioNextToBuf1End)
	ldi		ZL, pm_lo8(fioNextToBuf1End)
	sts		fio_put_next_byte_handler+1, ZH
	sts		fio_put_next_byte_handler, ZL
	ret


/****************************************************************/
//
/****************************************************************/
fioNextToBuf2Start:
	ldi		ZH, hi8(sdcard_buf_2)
	ldi		ZL, lo8(sdcard_buf_2)
	lds		r25, fio_buf_part_pos	
	add		ZL, r25
	adc		ZH, r1

	st			Z, r24
	inc		r25
	sts		fio_buf_part_pos, r25

	breq		put_next_part_start_2
	ret
put_next_part_start_2:
	ldi		ZH, pm_hi8(fioNextToBuf2End)
	ldi		ZL, pm_lo8(fioNextToBuf2End)
	sts		fio_put_next_byte_handler+1, ZH
	sts		fio_put_next_byte_handler, ZL
	ret


/****************************************************************/
//
/****************************************************************/
fioNextToBuf1End:
	ldi		ZH, hi8(sdcard_buf_1 + 0x100)
	ldi		ZL, lo8(sdcard_buf_1 + 0x100)
	lds		r25, fio_buf_part_pos	
	add		ZL, r25
	adc		ZH, r1

	st			Z, r24
	inc		r25
	sts		fio_buf_part_pos, r25

	breq		put_next_part_end_1
	ret
put_next_part_end_1:
   lds		r25, fio_readyFlags
	ori		r25, 1
	sts		fio_readyFlags, r25

	ldi		ZH, pm_hi8(fioNextToBuf2Wait)
	ldi		ZL, pm_lo8(fioNextToBuf2Wait)
	sts		fio_put_next_byte_handler+1, ZH
	sts		fio_put_next_byte_handler, ZL
	ret


/****************************************************************/
//
/****************************************************************/
fioNextToBuf2End:
	ldi		ZH, hi8(sdcard_buf_2 + 0x100)
	ldi		ZL, lo8(sdcard_buf_2 + 0x100)
	lds		r25, fio_buf_part_pos	
	add		ZL, r25
	adc		ZH, r1

	st			Z, r24
	inc		r25
	sts		fio_buf_part_pos, r25

	breq		put_next_part_end_2
	ret
put_next_part_end_2:
   lds		r25, fio_readyFlags
	ori		r25, 2
	sts		fio_readyFlags, r25

	ldi		ZH, pm_hi8(fioNextToBuf1Wait)
	ldi		ZL, pm_lo8(fioNextToBuf1Wait)
	sts		fio_put_next_byte_handler+1, ZH
	sts		fio_put_next_byte_handler, ZL
	ret


/****************************************************************/
//
/****************************************************************/
fioNextToBuf1Wait:
	lds		r25, fio_readyFlags
	sbrs		r25, 0
	rjmp		next_write_wait_1_ok

	lds		r24, fio_error_mask
	ori		r24, FIO_ERROR_WRITE_NOT_READY
	sts		fio_error_mask, r24
	lds		r24, fio_error_cnt				; fio_error_cnt++;
	lds		r25, fio_error_cnt+1
	adiw		r24, 1
	sts		fio_error_cnt+1, r25
	sts		fio_error_cnt, r24
	ret
next_write_wait_1_ok:
   ldi		ZH, pm_hi8(fioNextToBuf1Start)
   ldi		ZL, pm_lo8(fioNextToBuf1Start)
   sts		fio_put_next_byte_handler+1, ZH
   sts		fio_put_next_byte_handler, ZL

	sts		sdcard_buf_1, r24
	ldi		r25, 1
	sts		fio_buf_part_pos, r25
	ret


/****************************************************************/
//
/****************************************************************/
fioNextToBuf2Wait:
	lds		r25, fio_readyFlags
	sbrs		r25, 1
	rjmp		next_write_wait_2_ok

	lds		r24, fio_error_mask
	ori		r24, FIO_ERROR_WRITE_NOT_READY
	sts		fio_error_mask, r24
	lds		r24, fio_error_cnt				; fio_error_cnt++;
	lds		r25, fio_error_cnt+1
	adiw		r24, 1
	sts		fio_error_cnt+1, r25
	sts		fio_error_cnt, r24
	ret
next_write_wait_2_ok:
   ldi		ZH, pm_hi8(fioNextToBuf2Start)
   ldi		ZL, pm_lo8(fioNextToBuf2Start)
   sts		fio_put_next_byte_handler+1, ZH
   sts		fio_put_next_byte_handler, ZL

	sts		sdcard_buf_2, r24
	ldi		r25, 1
	sts		fio_buf_part_pos, r25
	ret












#ifdef NEVER


// (X=R27:R26, Y=R29:R28 and Z=R31:R30)


// r18-r27, r30, r31		могут свободно использоваться в коде

#endif