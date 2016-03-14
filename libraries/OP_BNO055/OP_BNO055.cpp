/* OP_BNO055.cpp 	Open Panzer Bosch BNO-055 IMU sensor library
 * Source: 			openpanzer.org				
 * Authors:    		Kevin Townsend, Luke Middleton
 *
 * This library is a modification of Kevin Townsend's (KTOWN) Adafruit_BNO055 library.
 * GitHub page: https://github.com/adafruit/Adafruit_BNO055
 *
 * We've removed a bunch of stuff, including all compatibility with the Adafruit standard sensor. 
 * The biggest change is the incorporation of our non-blocking I2C library (see OP_I2C). Whereas
 * the Adafruit library sends an I2C request, waits (blocks all code), then spits out the response,
 * in our case we must submit a request then come back later to see if it was successful. This 
 * basically required changing everything, but all the definitions in the h file were still quite
 * useful. 
 * 
 * The original code was copyrighted under the MIT License, which this modification maintains.
 */

// ORIGINAL LICENSE:
/***************************************************************************
  This is a library for the BNO055 orientation sensor

  Designed specifically to work with the Adafruit BNO055 Breakout.

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products

  These sensors use I2C to communicate, 2 pins are required to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by KTOWN for Adafruit Industries.

  The MIT License (MIT)
 
  Copyright (c) 2015 Adafruit Industries
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.  
 ***************************************************************************/


#include "OP_BNO055.h"

/***************************************************************************
 CONSTRUCTOR
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief  Instantiates a new OP_BNO055 class
*/
/**************************************************************************/
OP_BNO055::OP_BNO055(int32_t sensorID, uint8_t address)
{
	// Sensor ID is an internal ID saved by the class, it has nothing to do with
	// the physical device. If nothing is passed, the ID will be -1. 
	_sensorID = sensorID;
	// Address is the special BNO055 address. It can have one of two addresses. 
	// Default is 0x28, alternate is 0x29 set by putting the ADR pin to 3v. 
	// If we pass nothing, default will be 0x28 (BNO055_ADDRESS_A).
	_address = address;
	
	// There is also an actual sensor ID that is specific to the BNO055, to 
	// differentiate it on the bus from other Bosch devices. This ID is 
	// 0xA0 (BNO055_ID). If we poll the device and it doesn't return that ID, 
	// there may be something on the I2C line, but it's not a Bosch BNO055
}

/***************************************************************************
 PUBLIC FUNCTIONS
 ***************************************************************************/

void OP_BNO055::begin()
{
	// Initialize I2C comms using our special non-blocking library
	OP_I2C_Init();

	// Initialize some variables
	state = STATE_IDLE;
	_deviceFound = false;
	_chipID = 0;

	// Set our transaction struct to point to a location in memory large enough to hold up to MAX_OPS number of operations. 
	// t is 5 bytes, and each operation is 5 bytes (we think - assuming 2-byte RAM pointers). So total RAM = 5 + 25*5 = 130 bytes
	t = (i2c_txn_t*)alloca(sizeof(*t) + MAX_OPERATIONS * sizeof(t->ops[0]));

	// Clear the register array
	clearReg();
}

bool OP_BNO055::transactionDone()
{
	uint8_t flag = t->flags;
	return (flag & I2C_TXN_DONE);
}

bool OP_BNO055::transactionSuccessful()
{
	uint8_t flag = t->flags;
	if (flag & I2C_TXN_ERR) return false;
	else return true;
}

boolean OP_BNO055::process()
{	// This routine needs to be polled at regular intervals, or actually, whenever transactionDone is true.
	if (transactionDone())
	{
		// If transaction was a success, see if we need to process anything
		if (transactionSuccessful())
		{
			switch (state)
			{
				case STATE_GET_ID:			// The device returned an ID. See if it's the one we were looking for
					if (_chipID == BNO055_ID) _deviceFound = true;
					else 					  _deviceFound = false;
					break;
				
				case STATE_GET_CAL:					
					getCalibration();		// Splits the single byte result into the 4 calibration vaiables
					break; 
				
				case STATE_GET_OFFSETS:		
					//getOffsets();			// We will already have offsets saved in the offsetData[] array, but this puts them into a convenient struct "offsets"
					break;					// DISABLED FOR NOW. 
				
				case STATE_GET_VECTOR:
					getCalibration();		// Whenever a vector is requested, we also get the calibration to go with it
					getVector();			// In this case, we fill the orientation variable with our vector readings
					break;
				
				case STATE_GET_TEMP:				
					getTemp();				// This converts the variable to a signed int8 and saves it to "temperature"
					break; 
				
				case STATE_GET_STATUS:
					// Don't need to do anything
					break;
			}
		}
		// Whether it succeeded or failed, if the transaction is done we are now in the idle state
		state = STATE_IDLE;	
		// Clear the register variables for next time
		clearReg();	
		// Return true to indicate that processing is done
		return true;
	}
	else
	{
		// Process is still ongoing
		return false;
	}
}

void OP_BNO055::checkIfPresent(void)
{
	reg[0] = BNO055_CHIP_ID_ADDR;
	read8(&reg[0], &_chipID);		// Read CHIP_ID_ADDR, save it to variable _chipID
	state = STATE_GET_ID;			// Let the polling handler know what to check next
}

void OP_BNO055::clearReg(void)
{
	memset(reg, 0, (MAX_OPERATIONS*2));
}

boolean OP_BNO055::setup(boolean usextal, bno055_opmode_t mode)
{
	// Save the desired mode internally 
	_mode = mode; 

	// Reset the device, it will power on in configuration mode
	// Transaction with count of operations
		OP_I2C_Init_Transaction(t, 1);
	// 1. Reset
		reg[0] = BNO055_SYS_TRIGGER_ADDR;
		reg[1] = 0x20;
		OP_I2C_Init_WriteRegAndByte(&t->ops[0], _address, &reg[0]);
	// Now post it
		OP_I2C_Post(t);
	// Wait for transaction to complete
		while (!process()) { delay(1); }

	// According to Section 1.2 (Table 0-2) of the datasheet, power off reset typically takes 650mS, so we wait a bit longer than that. 
	// Yes, this is a blocking delay, but we allow it because this is the setup routine
        delay(800);

	// Check if the device is back online
		checkIfPresent();                       // Now see if the IMU can be detected on the bus
        while (!process()) { delay(1); }        // Wait for transaction to complete (yes, we block but this is the setup routine so we don't care)
        if (!isPresent()) 
		{
			return false; // Setup failed
		}
	
	// Now complete setup
	
	// Transaction with count of operations
		OP_I2C_Init_Transaction(t, 6);
		
	// 1. Set to normal power mode 
		reg[0] = BNO055_PWR_MODE_ADDR;
		reg[1] = POWER_MODE_NORMAL;
		OP_I2C_Init_WriteRegAndByte(&t->ops[0], _address, &reg[0]);
	
	// 2. Delay 10 mS
		OP_I2C_Init_Pause(&t->ops[1], 10);
	
	// 3. Go to Page 0
		reg[2] = BNO055_PAGE_ID_ADDR;
		reg[3] = 0x00;
		OP_I2C_Init_WriteRegAndByte(&t->ops[2], _address, &reg[2]);
	
	// 4. Crystal
		reg[4] = BNO055_SYS_TRIGGER_ADDR;
		usextal ? reg[5] = 0x80 : reg[5] = 0x00;
		OP_I2C_Init_WriteRegAndByte(&t->ops[3], _address, &reg[4]);
		
	// 5. Delay 10 mS
		OP_I2C_Init_Pause(&t->ops[4], 10);
	
	// X. Output units
		/* Defaults are fine, we don't need to change them. 
		uint8_t unitsel = (0 << 7) | // Orientation = Android
						  (0 << 4) | // Temperature = 1=Fahrenheit, 0=Celsius
						  (0 << 2) | // Euler = Degrees
						  (1 << 1) | // Gyro = Rads
						  (0 << 0);  // Accelerometer = m/s^2
		reg[8] = BNO055_UNIT_SEL_ADDR;
		reg[9] = unitsel;
		OP_I2C_Init_WriteRegAndByte(&t->ops[4], _address, &reg[8]);
		*/
	
	// X. Temp source
		//reg[10] = BNO055_TEMP_SOURCE_ADDR;
		//reg[11] = 0x00;	// 0x00 for Accel source, 0x01 for Gyro source - they both seem to return about the same thing
		//OP_I2C_Init_WriteRegAndByte(&t->ops[5], _address, &reg[10]);	
	
	// X. Axis mapping
		// You can mess with this later. The default works well if you use the z-axis as pitch...
		//  write8(BNO055_AXIS_MAP_CONFIG_ADDR, REMAP_CONFIG_P2); // P0-P7, Default is P1
		//  write8(BNO055_AXIS_MAP_SIGN_ADDR, REMAP_SIGN_P2); // P0-P7, Default is P1

	// 6. Set the operating mode
		reg[6] = BNO055_OPR_MODE_ADDR;
		reg[7] = _mode;
		OP_I2C_Init_WriteRegAndByte(&t->ops[5], _address, &reg[6]);
	
	// Now post it
		OP_I2C_Post(t);

	// Let the polling handler know what to check next
		state = STATE_SETUP;	

	// Now wait for this to complete (yes, we block but this is the setup routine so we don't care)
        while (!process()) { delay(1); }        

	// Return the status of the transaction. If true, setup worked. 
		return transactionSuccessful();
}


void OP_BNO055::requestStatus(void)
{
	// Transaction with 7 operations
	OP_I2C_Init_Transaction(t, 7);	

	// Operations
	// 1. Page 0
		reg[0] = BNO055_PAGE_ID_ADDR;
		reg[1] = 0x00;
		OP_I2C_Init_WriteRegAndByte(&t->ops[0], _address, &reg[0]);
	// 2-3. Read system status
		reg[2] = BNO055_SYS_STAT_ADDR;
		OP_I2C_Init_WriteReg(&t->ops[1], _address, &reg[2]);		
		OP_I2C_Init_ReadByte(&t->ops[2], _address, &status.systemStatus);				
	// 4-5. Read Test Results
		reg[3] = BNO055_SELFTEST_RESULT_ADDR;	
		OP_I2C_Init_WriteReg(&t->ops[3], _address, &reg[1]);		
		OP_I2C_Init_ReadByte(&t->ops[4], _address, &status.selfTest);				
	// 6-7. Read system status
		reg[4] = BNO055_SYS_ERR_ADDR;
		OP_I2C_Init_WriteReg(&t->ops[5], _address, &reg[2]);		
		OP_I2C_Init_ReadByte(&t->ops[6], _address, &status.systemError);	
	// Now post it
		OP_I2C_Post(t);
	// Let the polling handler know what to check next
		state = STATE_GET_STATUS;	

  /* System Status (see section 4.3.58)
     ---------------------------------
     0 = Idle
     1 = System Error
     2 = Initializing Peripherals
     3 = System Iniitalization
     4 = Executing Self-Test
     5 = Sensor fusio algorithm running
     6 = System running without fusion algorithms */

  /* Self Test Results (see section )
     --------------------------------
     1 = test passed, 0 = test failed

     Bit 0 = Accelerometer self test
     Bit 1 = Magnetometer self test
     Bit 2 = Gyroscope self test
     Bit 3 = MCU self test

     0x0F = all good! */

  /* System Error (see section 4.3.59)
     ---------------------------------
     0 = No error
     1 = Peripheral initialization error
     2 = System initialization error
     3 = Self test result failed
     4 = Register map value out of range
     5 = Register map address out of range
     6 = Register map write error
     7 = BNO low power mode not available for selected operat ion mode
     8 = Accelerometer power mode not available
     9 = Fusion algorithm configuration error
     A = Sensor configuration error */

}


void OP_BNO055::requestCalibration() 
{
	// Clear calibration data
	_calData = 0;	
	calibration.system = calibration.gyro = calibration.accel = calibration.mag = 0;
	clearReg();
	reg[0] = BNO055_CALIB_STAT_ADDR;
	read8(&reg[0], &_calData);
	state = STATE_GET_CAL;
}

void OP_BNO055::getCalibration()
{	// Split the returned byte into four parts and save to our calibration struct
	calibration.system = (_calData >> 6) & 0x03;
    calibration.gyro = (_calData >> 4) & 0x03;
    calibration.accel = (_calData >> 2) & 0x03;
    calibration.mag = _calData & 0x03;
}


void OP_BNO055::requestTemp(void)
{
	reg[0] = BNO055_TEMP_ADDR;
	read8(&reg[0], &_temp);			// Read TEMP_ADDR, save it to variable "_temp"
	state = STATE_GET_TEMP;			// Let the polling handler know what to check next	
}

void OP_BNO055::getTemp(void)
{	
	temperature = (int8_t)_temp;	// Convert to signed and save to "temperature"
}


void OP_BNO055::requestEuler(void)
{
	requestVector(VECTOR_EULER);
}

void OP_BNO055::requestVector(vector_type_t vector_type)
{	// Whenever we request a vector, we also automatically request calibration data as well

	// Transaction with 5 operations
		OP_I2C_Init_Transaction(t, 5);	
	// Save vector type so we know how to convert it when the results come back
		_cur_vector_type = vector_type;
	// Clear the vector buffer
		memset (_vectorBuf, 0, 6);
	// Operations
	// 1-2. Get vector
		reg[1] = (uint8_t)vector_type;
		OP_I2C_Init_WriteReg(&t->ops[0], _address, &reg[1]);		
		OP_I2C_Init_ReadLen(&t->ops[1], _address, _vectorBuf, 6);	// _vectorBuf is already an array, so we don't need to pass the address of it
	// 3. Delay 10 mS
		OP_I2C_Init_Pause(&t->ops[2], 5);		
	// 4-5. Get calibration
		reg[0] = BNO055_CALIB_STAT_ADDR;
		OP_I2C_Init_WriteReg(&t->ops[3], _address, &reg[0]);		
		OP_I2C_Init_ReadByte(&t->ops[4], _address, &_calData);	
	// Now post it
		OP_I2C_Post(t);
	// Let the polling handler know what to check next
		state = STATE_GET_VECTOR;
}

void OP_BNO055::getVector()
{
	// This will assign whatever vector type was originally requested to the orientation variable
	int16_t x, y, z;
	x = y = z = 0;
	x = ((int16_t)_vectorBuf[0]) | (((int16_t)_vectorBuf[1]) << 8);
	y = ((int16_t)_vectorBuf[2]) | (((int16_t)_vectorBuf[3]) << 8);
	z = ((int16_t)_vectorBuf[4]) | (((int16_t)_vectorBuf[5]) << 8);

	// Convert the value to an appropriate range (section 3.6.4) 
	// and assign the value to the orientation struct 
	switch(_cur_vector_type)
	{
		case VECTOR_MAGNETOMETER:
			// 1uT = 16 LSB
			orientation.x = ((double)x)/16.0;
			orientation.y = ((double)y)/16.0;
			orientation.z = ((double)z)/16.0;
			break;
		case VECTOR_GYROSCOPE:
			// 1rps = 900 LSB 
			orientation.x = ((double)x)/900.0;
			orientation.y = ((double)y)/900.0;
			orientation.z = ((double)z)/900.0;
			break;
		case VECTOR_EULER:
			// 1 degree = 16 LSB 
			orientation.x = ((double)x)/16.0;
			orientation.y = ((double)y)/16.0;
			orientation.z = ((double)z)/16.0;
			break;
		case VECTOR_ACCELEROMETER:
		case VECTOR_LINEARACCEL:
		case VECTOR_GRAVITY:
			// 1m/s^2 = 100 LSB 
			orientation.x = ((double)x)/100.0;
			orientation.y = ((double)y)/100.0;
			orientation.z = ((double)z)/100.0;
			break;
	}
}


/***************************************************************************
 PRIVATE FUNCTIONS
 ***************************************************************************/
// You can only use these if you are submitting a single transaction and this is
// the only operation. Otherwise you must setup a fuller transaction manually, 
// which is what is most often done in the code above. 
void OP_BNO055::read8(uint8_t *reg, uint8_t *value)
{
	// Read 1 byte (8 bits)
	// This involves a transaction with two operations:
	// Operation 1: Send a command ("write") to tell the device what we want to know
	// Operation 2: Read the byte the device will respond with
    OP_I2C_Init_Transaction(t, 2);
	// First operation, is to request the register
	OP_I2C_Init_WriteReg(&t->ops[0], _address, reg);		// Send one byte register 
    // Second operation, will be to store this ID in value
	OP_I2C_Init_ReadByte(&t->ops[1], _address, value);		// Read 1 byte back and store it in variable pointed to by value

	// Now post it
	OP_I2C_Post(t);
}

void OP_BNO055::readLen(uint8_t *reg, uint8_t *buffer, uint8_t len)
{
	// Read multiple bytes
	// This involves a transaction with two operations:
	// Operation 1: Send a command ("write") to tell the device what we want to know
	// Operation 2: Read the bytes the device will respond with
    OP_I2C_Init_Transaction(t, 2);
	// First operation, is to request the register
	OP_I2C_Init_WriteReg(&t->ops[0], _address, reg);		// Send one byte register 
    // Second operation, will be to store the return value in the buffer
	OP_I2C_Init_ReadLen(&t->ops[1], _address, buffer, len);	// Read len bytes back and store it in buffer

	// Now post it
	OP_I2C_Post(t);
}


//---------------------------------------------------------------------------------------------------------------------------->>
// OFFSETS STUFF - Theoretically this all works, but we aren't using it, so it's commented out to save space. 
//---------------------------------------------------------------------------------------------------------------------------->>
/*
void OP_BNO055::requestSensorOffsets()
{
	// Sensor offsets are only useful if the device is fully calibrated (sys, gyro, accel, mag all equal 3)
	// But we will assume that has already been checked before these were requested

	// Transaction with 4 operations
		OP_I2C_Init_Transaction(t, 4);
	// Operations
	// 1. Set to configuration mode
		reg[0] = BNO055_OPR_MODE_ADDR;
		reg[1] = OPERATION_MODE_CONFIG;
		OP_I2C_Init_WriteRegAndByte(&t->ops[0], _address, &reg[0]);
	// 2-3. Read offset data
		reg[2] = ACCEL_OFFSET_X_LSB_ADDR;	// Start at this address, and read 22 bytes
		OP_I2C_Init_WriteReg(&t->ops[1], _address, &reg[2]);		
		OP_I2C_Init_ReadLen(&t->ops[2], _address, offsetData, NUM_BNO055_OFFSET_REGISTERS);	// offsetData is already an array, so we don't need the "&"
	// 4. Set the mode back to what it was
		reg[3] = BNO055_OPR_MODE_ADDR;
		reg[4] = _mode;
		OP_I2C_Init_WriteRegAndByte(&t->ops[4], _address, &reg[3]);
	// Now post it
		OP_I2C_Post(t);
	// Let the polling handler know what to check next
		state = STATE_GET_OFFSETS;		
}

void OP_BNO055::getOffsets(void)
{
	// Save offset bytes from the offsetData[] array into the struct for convenience
	offsets.accel_x = (offsetData[1]  << 8 | offsetData[0]  );
	offsets.accel_y = (offsetData[3]  << 8 | offsetData[2]  );
	offsets.accel_z = (offsetData[5]  << 8 | offsetData[4]  );

	offsets.mag_x   = (offsetData[7]  << 8 | offsetData[6]  ); 
	offsets.mag_y   = (offsetData[9]  << 8 | offsetData[8]  );
	offsets.mag_z   = (offsetData[11] << 8 | offsetData[10] );

	offsets.gyro_x  = (offsetData[13] << 8 | offsetData[12] );
	offsets.gyro_y  = (offsetData[15] << 8 | offsetData[14] );
	offsets.gyro_z  = (offsetData[17] << 8 | offsetData[16] );

	offsets.accel_radius = (offsetData[19] << 8 | offsetData[18]);
	offsets.mag_radius = (offsetData[21] << 8 | offsetData[20]);
}

void OP_BNO055::setSensorOffsets_Accel()
{	// This writes the accelerometer offsets saved in offsetData[]

	// Transaction with 10 operations
		OP_I2C_Init_Transaction(t, 10);
	// Operations
	// 1. Set to configuration mode
		reg[0] = BNO055_OPR_MODE_ADDR;
		reg[1] = OPERATION_MODE_CONFIG;
		OP_I2C_Init_WriteRegAndByte(&t->ops[0], _address, &reg[0]);
	// 2-9. Write offset data, this involves a lot of bytes because we need to send the register for each first. 
	//	  Unfortunately I2C doesn't have a "writeLen" so this gets to be ridiculous, that is why we split up
	//    writing the offsets into three functions so we limit the number of operations needed at any one time.
		reg[2]  = ACCEL_OFFSET_X_LSB_ADDR;	
		reg[3]  = offsetData[0];
		OP_I2C_Init_WriteRegAndByte(&t->ops[1], _address, &reg[2]);
		reg[4]  = ACCEL_OFFSET_X_MSB_ADDR;	
		reg[5]  = offsetData[1];		
		OP_I2C_Init_WriteRegAndByte(&t->ops[2], _address, &reg[4]);
		reg[6]  = ACCEL_OFFSET_Y_LSB_ADDR;	
		reg[7]  = offsetData[2];		
		OP_I2C_Init_WriteRegAndByte(&t->ops[3], _address, &reg[6]);
		reg[8]  = ACCEL_OFFSET_Y_MSB_ADDR;	
		reg[9]  = offsetData[3];	
		OP_I2C_Init_WriteRegAndByte(&t->ops[4], _address, &reg[8]);
		reg[10] = ACCEL_OFFSET_Z_LSB_ADDR;	
		reg[11] = offsetData[4];
		OP_I2C_Init_WriteRegAndByte(&t->ops[5], _address, &reg[10]);
		reg[12] = ACCEL_OFFSET_Z_MSB_ADDR;	
		reg[13] = offsetData[5];			
		OP_I2C_Init_WriteRegAndByte(&t->ops[6], _address, &reg[12]);
		reg[14] = ACCEL_RADIUS_LSB_ADDR;	
		reg[15] = offsetData[18];	// Make sure you have the right elements in the array!
		OP_I2C_Init_WriteRegAndByte(&t->ops[7], _address, &reg[14]);
		reg[16] = ACCEL_RADIUS_MSB_ADDR;	
		reg[17] = offsetData[19];		
		OP_I2C_Init_WriteRegAndByte(&t->ops[8], _address, &reg[16]);
	// 10. Set the mode back to what it was
		reg[18] = BNO055_OPR_MODE_ADDR;
		reg[19] = _mode;
		OP_I2C_Init_WriteRegAndByte(&t->ops[9], _address, &reg[18]);
	// Now post it
		OP_I2C_Post(t);
	// Let the polling handler know what to check next
		state = STATE_WRITE_OFFSETS;
}

void OP_BNO055::setSensorOffsets_Gyro()
{	// This writes the gyro offsets saved in offsetData[]

	// Transaction with 8 operations
		OP_I2C_Init_Transaction(t, 8);
	// Operations
	// 1. Set to configuration mode
		reg[0] = BNO055_OPR_MODE_ADDR;
		reg[1] = OPERATION_MODE_CONFIG;
		OP_I2C_Init_WriteRegAndByte(&t->ops[0], _address, &reg[0]);
	// 2-7. Write offset data, this involves a lot of bytes because we need to send the register for each first. 
	//	  Unfortunately I2C doesn't have a "writeLen" so this gets to be ridiculous, that is why we split up
	//    writing the offsets into three functions so we limit the number of operations needed at any one time.
		reg[2] = GYRO_OFFSET_X_LSB_ADDR;	
		reg[3] = offsetData[12];
		OP_I2C_Init_WriteRegAndByte(&t->ops[1], _address, &reg[2]);
		reg[4] = GYRO_OFFSET_X_MSB_ADDR;	
		reg[5] = offsetData[13];		
		OP_I2C_Init_WriteRegAndByte(&t->ops[2], _address, &reg[4]);
		reg[6] = GYRO_OFFSET_Y_LSB_ADDR;	
		reg[7] = offsetData[14];		
		OP_I2C_Init_WriteRegAndByte(&t->ops[3], _address, &reg[6]);
		reg[8] = GYRO_OFFSET_Y_MSB_ADDR;	
		reg[9] = offsetData[15];	
		OP_I2C_Init_WriteRegAndByte(&t->ops[4], _address, &reg[8]);
		reg[10] = GYRO_OFFSET_Z_LSB_ADDR;	
		reg[11] = offsetData[16];
		OP_I2C_Init_WriteRegAndByte(&t->ops[5], _address, &reg[10]);
		reg[12] = GYRO_OFFSET_Z_MSB_ADDR;	
		reg[13] = offsetData[17];
		OP_I2C_Init_WriteRegAndByte(&t->ops[6], _address, &reg[12]);
	// 8. Set the mode back to what it was
		reg[14] = BNO055_OPR_MODE_ADDR;
		reg[15] = _mode;
		OP_I2C_Init_WriteRegAndByte(&t->ops[7], _address, &reg[14]);
	// Now post it
		OP_I2C_Post(t);
	// Let the polling handler know what to check next
		state = STATE_WRITE_OFFSETS;
}


void OP_BNO055::setSensorOffsets_Mag()
{	// This writes the magnetometer offsets saved in offsetData[]

	// Transaction with 10 operations
		OP_I2C_Init_Transaction(t, 10);
	// Operations
	// 1. Set to configuration mode
		reg[0] = BNO055_OPR_MODE_ADDR;
		reg[1] = OPERATION_MODE_CONFIG;
		OP_I2C_Init_WriteRegAndByte(&t->ops[0], _address, &reg[0]);
	// 2-9. Write offset data, this involves a lot of bytes because we need to send the register for each first. 
	//	  Unfortunately I2C doesn't have a "writeLen" so this gets to be ridiculous, that is why we split up
	//    writing the offsets into three functions so we limit the number of operations needed at any one time.
		reg[2] = MAG_OFFSET_X_LSB_ADDR;	
		reg[3] = offsetData[6];
		OP_I2C_Init_WriteRegAndByte(&t->ops[1], _address, &reg[2]);
		reg[4] = MAG_OFFSET_X_MSB_ADDR;	
		reg[5] = offsetData[7];		
		OP_I2C_Init_WriteRegAndByte(&t->ops[2], _address, &reg[4]);
		reg[6] = MAG_OFFSET_Y_LSB_ADDR;	
		reg[7] = offsetData[8];		
		OP_I2C_Init_WriteRegAndByte(&t->ops[3], _address, &reg[6]);
		reg[8] = MAG_OFFSET_Y_MSB_ADDR;	
		reg[9] = offsetData[9];	
		OP_I2C_Init_WriteRegAndByte(&t->ops[4], _address, &reg[8]);
		reg[10] = MAG_OFFSET_Z_LSB_ADDR;	
		reg[11] = offsetData[10];
		OP_I2C_Init_WriteRegAndByte(&t->ops[5], _address, &reg[10]);
		reg[12] = MAG_OFFSET_Z_MSB_ADDR;	
		reg[13] = offsetData[11];
		OP_I2C_Init_WriteRegAndByte(&t->ops[6], _address, &reg[12]);
		reg[14] = MAG_RADIUS_LSB_ADDR;	
		reg[15] = offsetData[20];		// Make sure you have the right elements in the array!
		OP_I2C_Init_WriteRegAndByte(&t->ops[7], _address, &reg[14]);
		reg[16] = MAG_RADIUS_MSB_ADDR;	
		reg[17] = offsetData[21];	
		OP_I2C_Init_WriteRegAndByte(&t->ops[8], _address, &reg[16]);
	// 10. Set the mode back to what it was
		reg[18] = BNO055_OPR_MODE_ADDR;
		reg[19] = _mode;
		OP_I2C_Init_WriteRegAndByte(&t->ops[9], _address, &reg[18]);
	// Now post it
		OP_I2C_Post(t);
	// Let the polling handler know what to check next
		state = STATE_WRITE_OFFSETS;
}
*/