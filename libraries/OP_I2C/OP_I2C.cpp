/* OP_I2C.cpp 		Open Panzer I2C communication library
 * Source: 			openpanzer.org				
 * Authors:    		Pieter Noordhuis, Luke Middleton
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

#include "OP_I2C.h"


/* Pointers to current txn and op. */
static volatile i2c_txn_t *txn;
static volatile i2c_op_t *op;

/*
 * By default, the control register is set to:
 *  - TWEA: Automatically send acknowledge bit in receive mode.
 *  - TWEN: Enable the I2C system.
 *  - TWIE: Enable interrupt requests when TWINT is set.
 */
#define TWCR_DEFAULT (_BV(TWEA) | _BV(TWEN) | _BV(TWIE))
#define TWCR_NOT_ACK (_BV(TWINT) | _BV(TWEN) | _BV(TWIE))
#define TWCR_ACK (TWCR_NOT_ACK | _BV(TWEA))

void OP_I2C_Init(void) 
{
	uint8_t sreg;

	// Store the status register and disable interrupts. 
	sreg = SREG;
	cli();

	// 
	// From Atmega2560 datasheet (pg 248)
	//   SCL freq = (CPU Clock freq) / 16 + (2*TWBR * (4^PrescalerValue))

	// Which means:
	//   TWBR = ((CPU Clock freq / SCL freq) - 16) / (2 * (4^PrescalerValue))

	// We find that setting prescaler to 1 (none) gives us the higher frequencies
	TWSR &= ~(_BV(TWPS1) | _BV(TWPS0));			// Set prescaler to 1 / none
	TWBR = ((F_CPU / I2C_FREQ) - 16) / (2 * 4); // Set bit rate based on CPU freq and desired I2C frequency
	//Serial.println(TWBR);						// TWBR should equal 18 if we want 100kHz frequency

	// Activate internal pull-up resistors, even though we also have external ones attached.
	// Atmega2560 ports are PD0 for SCL and PD1 for SDA.
	PORTD |= _BV(PD0) | _BV(PD1);

	// Enable I2C interrupts via the control register
	TWCR = TWCR_DEFAULT;

	// Disable slave mode
	TWAR = 0;

	// These are used for the interrupt-based pauses that we can include in our transactions. We use output compare C of Timer 1.
	// Timer 1 is assumed already to be set up by the various other classes that use it (Servo, PPM, IR).
	TIFR1 |= (1 << OCF1C);			// Clear any pending interrupts by setting OCF1C bit of TIFR1 - output compare match flag for Timer 1, C.
									// Oddly (see page 168), the flag is manually cleared by writing a logic one to it.
									// Otherwise in use it is set when the compare matches, and automatically cleared when exiting the Timer 1 Compare Match C ISR
	TIMSK1 &= ~(1 << OCIE1C);		// OCIE1C bit of TIMSK1 = Output Compare Interrupt Enable 1 C - disable for now.

	// Restore the status register. 
	SREG = sreg;
}

void OP_I2C_Post(i2c_txn_t *t) 
{
	uint8_t sreg;

	// Reset transaction attributes. 
	t->flags = 0;
	t->opspos = 0;
	t->next = NULL;

	sreg = SREG;
	cli();

	// Append transaction to linked list. 
	if (txn == NULL) 
	{
		txn = t;
		op = &txn->ops[0];

		// Transmit START to kickstart things. 
		TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
	} 
	else 
	{
		volatile i2c_txn_t *txn_ = txn;
		while (txn_ != NULL) 
		{
			if (txn_->next != NULL) 
			{
				txn_ = txn_->next;
			} 
			else 
			{
				txn_->next = t;
				break;
			}
		}
	}

	SREG = sreg;
}


// The original author used 5 assert statements in this interrupt. Although I am not entirely certain what the definition 
// of this statement is in the current environment, it seems likely to halt all operations in an eternal loop. 

ISR(TWI_vect, ISR_BLOCK) 
{
	uint8_t status = TW_STATUS;

	// This interrupt should only fire if there is something to do 
	//assert(op != NULL);
	
	// That is true, but we are not going to use assert to handle it. Instead just exit. 
	if (op == NULL) return;

	if ((op->address & _BV(0)) == TW_READ) 
	{	// Master Receiver mode. 
		switch (status) 
		{
			// A START condition has been transmitted. 
			case TW_START:
			// A repeated START condition has been transmitted. 
			case TW_REP_START:
				// The idea behind this assert is that we should not be starting transmissions if we don't 
				// have another operation to follow. That is true, but we need some sort of better
				// error handling than an assert statement.
				//assert(op->buflen > 0);
				op->bufpos = 0;
				TWDR = op->address;
				TWCR = TWCR_DEFAULT | _BV(TWINT);
				break;

			// Arbitration lost in SLA+R or NOT ACK bit. 
			case TW_MR_ARB_LOST:
				// A START condition will be transmitted when the bus becomes free. 
				TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
				break;

			// SLA+R has been transmitted; ACK has been received. 
			case TW_MR_SLA_ACK:
				if (op->buflen == 1) 
				{
					TWCR = TWCR_NOT_ACK;
				}
				else 
				{
					TWCR = TWCR_ACK;
				}
				break;

			// SLA+R has been transmitted; NOT ACK has been received. 
			case TW_MR_SLA_NACK:
				txn->flags = I2C_TXN_DONE | I2C_TXN_ERR;
				goto next_txn;

			// Data byte has been received; ACK has been returned. 
			case TW_MR_DATA_ACK:
				op->buf[op->bufpos++] = TWDR;
				if (op->bufpos+1 == op->buflen) 
				{
					TWCR = TWCR_NOT_ACK;
				} 
				else 
				{
					TWCR = TWCR_ACK;
				}
				break;

			// Data byte has been received; NOT ACK has been returned. 
			case TW_MR_DATA_NACK:
				op->buf[op->bufpos++] = TWDR;
				goto next_op;

			//default:
				// This case should not happen, but again, if it does, we need
				// something better than assert. Instead we do nothing and just
				// let the ISR exit.
				//assert(0 && "unknown status in master receiver mode");
		}
	}
	else 
	{
		// Master Transmitter mode. 
		switch (status) 
		{

			// A START condition has been transmitted. 
			case TW_START:
			// A repeated START condition has been transmitted. 
			case TW_REP_START:
				// The idea behind this assert is that we should not be starting transmissions if we don't 
				// have another operation to follow. That is true, but we need some sort of better
				// error handling than an assert statement.
				//assert(op->buflen > 0);
				op->bufpos = 0;
				TWDR = op->address;
				TWCR = TWCR_DEFAULT | _BV(TWINT);
				break;

			// Arbitration lost in SLA+W or data bytes. 
			case TW_MT_ARB_LOST:
				// A START condition will be transmitted when the bus becomes free. 
				TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
				break;

			// SLA+W has been transmitted; ACK has been received. 
			case TW_MT_SLA_ACK:
				TWDR = op->buf[op->bufpos++];
				TWCR = TWCR_DEFAULT | _BV(TWINT);
				break;

			// SLA+W has been transmitted; NOT ACK has been received. 
			case TW_MT_SLA_NACK:
				txn->flags = I2C_TXN_DONE | I2C_TXN_ERR;
				goto next_txn;

			// Data byte has been transmitted; ACK has been received. 
			case TW_MT_DATA_ACK:
				if (op->bufpos < op->buflen) 
				{
					TWDR = op->buf[op->bufpos++];
					TWCR = TWCR_DEFAULT | _BV(TWINT);
					break;
				}
				// No more bytes left to transmit... 
				goto next_op;

			// Data byte has been transmitted; NOT ACK has been received. 
			case TW_MT_DATA_NACK:
				if (op->bufpos < op->buflen) 
				{
					// There were more bytes left to transmit! 
					txn->flags = I2C_TXN_DONE | I2C_TXN_ERR;
					goto next_txn;
				}
				goto next_op;

			//default:
				// This case should not happen, but again, if it does, we need
				// something better than assert. Instead we do nothing and just
				// let the ISR exit.			
				//assert(0 && "unknown status in master transmitter mode");
		}
	}

	return;

	// Advance to next operation in transaction, if possible.
	next_op:	
		OP_I2C_IncrementOp();
	return;

	// Advance to next transaction, if possible.
	next_txn:	
		OP_I2C_IncrementTxn();
}


// Timer1 Output Compare C interrupt service routine
// This triggers when TCNT1 = OCR1C
ISR(TIMER1_COMPC_vect)
{
	OP_I2C_IncrementOp();
}

void OP_I2C_IncrementOp(void)
{
	if (++(txn->opspos) < txn->opslen) 
	{
		op = &txn->ops[txn->opspos];

		if (op->isPause)
		{
			OCR1C = TCNT1 + op->pauseLen;	// Set the length of time to wait
			TIMSK1 |= (1 << OCIE1C);		// Enable the interrupt (OCIE1C bit of TIMSK1 = Output Compare Interrupt Enable 1 C)
		}
		else
		{
			OP_I2C_DisableCompareIntr();
			
			// Repeated start 
			TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
		}
		return;
	}
	OP_I2C_DisableCompareIntr();

	// No more operations, mark transaction as done
	txn->flags = I2C_TXN_DONE;
	
	// Go to the next transaction
	OP_I2C_IncrementTxn();
}

void OP_I2C_IncrementTxn(void)
{
	if (txn->next != NULL) 
	{
		txn = txn->next;
		op = &txn->ops[0];

		// Repeated start
		TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
		return;
	}
	txn = NULL;
	op = NULL;
	
	// Make sure this is really off 
	OP_I2C_DisableCompareIntr();

	// No more transaction, transmit STOP
	TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTO);
}

void OP_I2C_DisableCompareIntr(void)
{
	// Disable the Timer 1 Compare C interrupt
	TIMSK1 &= ~(1 << OCIE1C);	
}
