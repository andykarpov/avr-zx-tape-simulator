/*
 * Copyright (c) 2010-2013 by Oleg Trifonov <otrifonow@gmail.com>
 *
 * http://trolsoft.ru
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "i2c.h"
#include "debug.h"

void i2c_init() {
	TWSR = 0;							// no prescaler
	TWBR = ((F_CPU/SCL_CLOCK)-16)/2;	// must be > 10 for stable operation
}

static void i2c_wait() {
	// wait until transmission completed
	uint16_t cnt = 0;
	while ( !(TWCR & _BV(TWINT)) ) {
		if ( --cnt == 0 ) {
			MSG("i2c_wait TIMEOUT!");
			break;
		}
	}
}


bool i2c_start() {
	TWCR = _BV(TWINT)|_BV(TWEN)|_BV(TWSTA);	// send START condition
	i2c_wait();
	return TW_STATUS == TW_START;
}


bool i2c_sendAddress(uint8_t address) {
	uint8_t status;

	if ( (address & 0x01) == 0 ) {
		status = TW_MT_SLA_ACK;
	} else {
		status = TW_MR_SLA_ACK;
	}

	TWDR = address;
	TWCR = _BV(TWINT)|_BV(TWEN);
	i2c_wait();

	return (TWSR & 0xF8) == status;
}


bool i2c_sendByte(uint8_t byte) {
	TWDR = byte;
	TWCR = _BV(TWINT)| _BV(TWEN);
	i2c_wait();

	return (TWSR & 0xF8) == TW_MT_DATA_ACK;
}


bool i2c_receiveByteACK(uint8_t *byte) {
	TWCR = _BV(TWEA)|_BV(TWINT)|_BV(TWEN);
	i2c_wait();
	bool result = (TWSR & 0xF8) == TW_MR_DATA_ACK;
	*byte = TWDR;
	return result;
}


bool i2c_receiveByteNACK(uint8_t *byte) {
	TWCR =_BV(TWINT)|_BV(TWEN);
	i2c_wait();
	bool result = (TWSR & 0xF8) == TW_MR_DATA_NACK;
	*byte = TWDR;
	return result;
}


bool i2c_stop() {
	TWCR = _BV(TWINT)|_BV(TWEN)|_BV(TWSTO); // send STOP condition
	uint16_t cnt = 0;
	while ( TWCR & _BV(TWSTO) ) {
		if ( --cnt == 0 ) {
			MSG("i2c_stop TIMEOUT!");
			return false;
		}
	}
	return true;
}

