/* OP_I2C.h         Open Panzer I2C communication library
 * Source:          openpanzer.org              
 * Authors:         Pieter Noordhuis, Luke Middleton
 *
 * This library is a modification of Peter Noordhuis' "AVR-I2C" library. 
 * GitHub page: https://github.com/pietern/avr-i2c
 *
 * In addition to changing some settings to make it compatible with the Atmega2560 (the original was written for the 328), 
 * an interrupt-based pause system was added to allow for user-defined pauses between subsequent operations. 
 * 
 * The original code was copyrighted under the MIT License, which this modification maintains.    
 */
 
// ORIGINAL LICENSE: 
/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Pieter Noordhuis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _OP_I2C_H
#define _OP_I2C_H

//#include <assert.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <util/twi.h>
#include <Arduino.h>


// BNO055 sensor can operate at frequencies up to 400kHz 
// But we'll use 100kHz instead
#define I2C_FREQ 100000

#define I2C_TXN_DONE _BV(0) // First bit to 1
#define I2C_TXN_ERR  _BV(1) // Second bit to 1

typedef struct i2c_op i2c_op_t;
typedef struct i2c_txn i2c_txn_t;

struct i2c_op {
  uint8_t address;
  uint8_t buflen;
  uint8_t bufpos;
  uint8_t *buf;
  boolean isPause;
  uint32_t pauseLen;
};

struct i2c_txn {
  struct i2c_txn *next;
  volatile uint8_t flags;
  uint8_t opslen;
  uint8_t opspos;
  struct i2c_op ops[];
};

// Create a regular operation
static inline void i2c_init_operation(i2c_op_t *o, uint8_t address, uint8_t *buf, uint8_t buflen) {
  o->address = address;
  o->buflen = buflen;
  o->bufpos = 0;
  o->buf = buf;
  o->isPause = false;
  o->pauseLen = 0;
}

// Create a pause operation
static inline void OP_I2C_Init_Pause(i2c_op_t *o, uint32_t len) {
  o->address = 0;
  o->buflen = 1;
  o->bufpos = 0;
  o->buf = NULL;
  o->isPause = true;
  o->pauseLen = len*2000;   // Multiply by 2 because there are 2 ticks per uS, and 1000 because ther are 1000 uS per mS (assumes)
}

// Read multiple bytes
static inline void OP_I2C_Init_ReadLen(i2c_op_t *o, uint8_t address, uint8_t *buf, uint8_t buflen) {
  i2c_init_operation(o, (address << 1) | TW_READ, buf, buflen);
}

// Read a single byte
static inline void OP_I2C_Init_ReadByte(i2c_op_t *o, uint8_t address, uint8_t *buf) {
  i2c_init_operation(o, (address << 1) | TW_READ, buf, 1);
}

// I2C doesn't have a "writeLen". You can either write 1 byte or 2 bytes but that's it (at a time)

// Write two bytes - register, and data
static inline void OP_I2C_Init_WriteRegAndByte(i2c_op_t *o, uint8_t address, uint8_t *buf) {
  i2c_init_operation(o, (address << 1) | TW_WRITE, buf, 2);
}

// Write 1 byte (the register byte)
static inline void OP_I2C_Init_WriteReg(i2c_op_t *o, uint8_t address, uint8_t *reg) {
  i2c_init_operation(o, (address << 1) | TW_WRITE, reg, 1);
}


static inline void OP_I2C_Init_Transaction(i2c_txn_t *t, uint8_t opslen) {
  t->flags = 0;
  t->opslen = opslen;
  t->opspos = 0;
  t->next = NULL;
}


void OP_I2C_Init(void);
void OP_I2C_Post(i2c_txn_t *t);
void OP_I2C_IncrementOp(void);
void OP_I2C_IncrementTxn(void);
void OP_I2C_DisableCompareIntr(void);

#endif
