
/*
 * zx_tape_loader.s
 *
 * Created: 19.10.2015 22:17:44
 *  Author: Trol
 */ 

 #include <avr/io.h>

 .extern timer1compHandler


 .global TIMER1_COMPA_vect
 TIMER1_COMPA_vect:
	push	r1											// 2 clk			2
	push	r0											// 2 clk			4
	in		r0, _SFR_IO_ADDR(SREG)				// 1 clk			5
	push	r0											// 2 clk			7
	eor		r1, r1								// 1 clk			8
	in		r0, _SFR_IO_ADDR(RAMPZ)				//	1 clk			9
	push	r0											// 2 clk			11

	push	r24										// 2 clk			13
	push	r25										// 2 clk			15
	push	r30		// ZL							// 2 clk			17
	push	r31		// ZH							// 2 clk			19
	
	lds		ZL, timer1compHandler			// 2 clk			21
	lds		ZH, timer1compHandler+1			// 2 clk			23
	icall												// 4 clk			27

	pop		r31
	pop		r30
	pop		r25
	pop		r24

	pop		r0
	out		_SFR_IO_ADDR(RAMPZ), r0
	pop		r0
	out		_SFR_IO_ADDR(SREG), r0
	pop		r0
	pop		r1

	reti	
