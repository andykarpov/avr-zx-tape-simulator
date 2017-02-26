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

#ifndef _I2C_H_
#define _I2C_H_

#include <stdbool.h>
#include <util/twi.h>
#include <avr/io.h>



// defines the data direction (reading from I2C device) for i2c_sendAddress()
#define I2C_READ    1

// defines the data direction (writing to I2C device) for i2c_sendAddress()
#define I2C_WRITE   0

// I2C clock in Hz
#define SCL_CLOCK  100000L



void i2c_init();
bool i2c_sendAddress(uint8_t address);
bool i2c_start();
bool i2c_stop();

bool i2c_sendByte(uint8_t byte);

bool i2c_receiveByteACK(uint8_t *byte);
bool i2c_receiveByteNACK(uint8_t *byte);



#endif // _I2C_H_
