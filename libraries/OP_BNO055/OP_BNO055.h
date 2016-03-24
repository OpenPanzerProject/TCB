/* OP_BNO055.h      Open Panzer Bosch BNO-055 IMU sensor library
 * Source:          openpanzer.org              
 * Authors:         Kevin Townsend, Luke Middleton
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


#ifndef __OP_BNO055_H__
#define __OP_BNO055_H__

#include <Arduino.h>
#include <alloca.h>
#include "OP_I2C.h"

#define BNO055_ADDRESS_A (0x28)     // Decimal 40
#define BNO055_ADDRESS_B (0x29)     // Decimal 41
#define BNO055_ID        (0xA0)     // Decimal 160

#define NUM_BNO055_OFFSET_REGISTERS (22)

#define MAX_OPERATIONS 10

typedef struct 
{   union 
    {   float v[3];
        struct 
        {
            float x;
            float y;
            float z;
        };
    };
} imu_orientation_t;

typedef struct 
{   union 
    {   uint8_t c[4];
        struct 
        {
            uint8_t system;
            uint8_t gyro;
            uint8_t accel;
            uint8_t mag;
        };
    };
} imu_calibration_t;

typedef struct 
{   union 
    {   uint8_t c[3];
        struct 
        {
            uint8_t systemStatus;
            uint8_t selfTest;
            uint8_t systemError;
        };
    };
} imu_status_t;

/*
typedef struct
{
    uint16_t accel_x;
    uint16_t accel_y;
    uint16_t accel_z;
    uint16_t gyro_x;
    uint16_t gyro_y;
    uint16_t gyro_z;
    uint16_t mag_x;
    uint16_t mag_y;
    uint16_t mag_z;
    uint16_t accel_radius;
    uint16_t mag_radius;
} bno055_offsets_t;
*/

class OP_BNO055 
{
  public:
    typedef enum
    {
      /* Page id register definition */
      BNO055_PAGE_ID_ADDR                                     = 0X07,

      /* PAGE0 REGISTER DEFINITION START*/
      BNO055_CHIP_ID_ADDR                                     = 0x00,
      BNO055_ACCEL_REV_ID_ADDR                                = 0x01,
      BNO055_MAG_REV_ID_ADDR                                  = 0x02,
      BNO055_GYRO_REV_ID_ADDR                                 = 0x03,
      BNO055_SW_REV_ID_LSB_ADDR                               = 0x04,
      BNO055_SW_REV_ID_MSB_ADDR                               = 0x05,
      BNO055_BL_REV_ID_ADDR                                   = 0X06,

      /* Accel data register */
      BNO055_ACCEL_DATA_X_LSB_ADDR                            = 0X08,
      BNO055_ACCEL_DATA_X_MSB_ADDR                            = 0X09,
      BNO055_ACCEL_DATA_Y_LSB_ADDR                            = 0X0A,
      BNO055_ACCEL_DATA_Y_MSB_ADDR                            = 0X0B,
      BNO055_ACCEL_DATA_Z_LSB_ADDR                            = 0X0C,
      BNO055_ACCEL_DATA_Z_MSB_ADDR                            = 0X0D,

      /* Mag data register */
      BNO055_MAG_DATA_X_LSB_ADDR                              = 0X0E,
      BNO055_MAG_DATA_X_MSB_ADDR                              = 0X0F,
      BNO055_MAG_DATA_Y_LSB_ADDR                              = 0X10,
      BNO055_MAG_DATA_Y_MSB_ADDR                              = 0X11,
      BNO055_MAG_DATA_Z_LSB_ADDR                              = 0X12,
      BNO055_MAG_DATA_Z_MSB_ADDR                              = 0X13,

      /* Gyro data registers */
      BNO055_GYRO_DATA_X_LSB_ADDR                             = 0X14,
      BNO055_GYRO_DATA_X_MSB_ADDR                             = 0X15,
      BNO055_GYRO_DATA_Y_LSB_ADDR                             = 0X16,
      BNO055_GYRO_DATA_Y_MSB_ADDR                             = 0X17,
      BNO055_GYRO_DATA_Z_LSB_ADDR                             = 0X18,
      BNO055_GYRO_DATA_Z_MSB_ADDR                             = 0X19,

      /* Euler data registers */
      BNO055_EULER_H_LSB_ADDR                                 = 0X1A,
      BNO055_EULER_H_MSB_ADDR                                 = 0X1B,
      BNO055_EULER_R_LSB_ADDR                                 = 0X1C,
      BNO055_EULER_R_MSB_ADDR                                 = 0X1D,
      BNO055_EULER_P_LSB_ADDR                                 = 0X1E,
      BNO055_EULER_P_MSB_ADDR                                 = 0X1F,

      /* Quaternion data registers */
      BNO055_QUATERNION_DATA_W_LSB_ADDR                       = 0X20,
      BNO055_QUATERNION_DATA_W_MSB_ADDR                       = 0X21,
      BNO055_QUATERNION_DATA_X_LSB_ADDR                       = 0X22,
      BNO055_QUATERNION_DATA_X_MSB_ADDR                       = 0X23,
      BNO055_QUATERNION_DATA_Y_LSB_ADDR                       = 0X24,
      BNO055_QUATERNION_DATA_Y_MSB_ADDR                       = 0X25,
      BNO055_QUATERNION_DATA_Z_LSB_ADDR                       = 0X26,
      BNO055_QUATERNION_DATA_Z_MSB_ADDR                       = 0X27,

      /* Linear acceleration data registers */
      BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR                     = 0X28,
      BNO055_LINEAR_ACCEL_DATA_X_MSB_ADDR                     = 0X29,
      BNO055_LINEAR_ACCEL_DATA_Y_LSB_ADDR                     = 0X2A,
      BNO055_LINEAR_ACCEL_DATA_Y_MSB_ADDR                     = 0X2B,
      BNO055_LINEAR_ACCEL_DATA_Z_LSB_ADDR                     = 0X2C,
      BNO055_LINEAR_ACCEL_DATA_Z_MSB_ADDR                     = 0X2D,

      /* Gravity data registers */
      BNO055_GRAVITY_DATA_X_LSB_ADDR                          = 0X2E,
      BNO055_GRAVITY_DATA_X_MSB_ADDR                          = 0X2F,
      BNO055_GRAVITY_DATA_Y_LSB_ADDR                          = 0X30,
      BNO055_GRAVITY_DATA_Y_MSB_ADDR                          = 0X31,
      BNO055_GRAVITY_DATA_Z_LSB_ADDR                          = 0X32,
      BNO055_GRAVITY_DATA_Z_MSB_ADDR                          = 0X33,

      /* Temperature data register */
      BNO055_TEMP_ADDR                                        = 0X34,

      /* Status registers */
      BNO055_CALIB_STAT_ADDR                                  = 0X35,
      BNO055_SELFTEST_RESULT_ADDR                             = 0X36,
      BNO055_INTR_STAT_ADDR                                   = 0X37,

      BNO055_SYS_CLK_STAT_ADDR                                = 0X38,
      BNO055_SYS_STAT_ADDR                                    = 0X39,
      BNO055_SYS_ERR_ADDR                                     = 0X3A,

      /* Unit selection register */
      BNO055_UNIT_SEL_ADDR                                    = 0X3B,
      BNO055_DATA_SELECT_ADDR                                 = 0X3C,

      /* Mode registers */
      BNO055_OPR_MODE_ADDR                                    = 0X3D,
      BNO055_PWR_MODE_ADDR                                    = 0X3E,

      BNO055_SYS_TRIGGER_ADDR                                 = 0X3F,
      BNO055_TEMP_SOURCE_ADDR                                 = 0X40,

      /* Axis remap registers */
      BNO055_AXIS_MAP_CONFIG_ADDR                             = 0X41,
      BNO055_AXIS_MAP_SIGN_ADDR                               = 0X42,

      /* SIC registers */
      BNO055_SIC_MATRIX_0_LSB_ADDR                            = 0X43,
      BNO055_SIC_MATRIX_0_MSB_ADDR                            = 0X44,
      BNO055_SIC_MATRIX_1_LSB_ADDR                            = 0X45,
      BNO055_SIC_MATRIX_1_MSB_ADDR                            = 0X46,
      BNO055_SIC_MATRIX_2_LSB_ADDR                            = 0X47,
      BNO055_SIC_MATRIX_2_MSB_ADDR                            = 0X48,
      BNO055_SIC_MATRIX_3_LSB_ADDR                            = 0X49,
      BNO055_SIC_MATRIX_3_MSB_ADDR                            = 0X4A,
      BNO055_SIC_MATRIX_4_LSB_ADDR                            = 0X4B,
      BNO055_SIC_MATRIX_4_MSB_ADDR                            = 0X4C,
      BNO055_SIC_MATRIX_5_LSB_ADDR                            = 0X4D,
      BNO055_SIC_MATRIX_5_MSB_ADDR                            = 0X4E,
      BNO055_SIC_MATRIX_6_LSB_ADDR                            = 0X4F,
      BNO055_SIC_MATRIX_6_MSB_ADDR                            = 0X50,
      BNO055_SIC_MATRIX_7_LSB_ADDR                            = 0X51,
      BNO055_SIC_MATRIX_7_MSB_ADDR                            = 0X52,
      BNO055_SIC_MATRIX_8_LSB_ADDR                            = 0X53,
      BNO055_SIC_MATRIX_8_MSB_ADDR                            = 0X54,

      /* Accelerometer Offset registers */
      ACCEL_OFFSET_X_LSB_ADDR                                 = 0X55,
      ACCEL_OFFSET_X_MSB_ADDR                                 = 0X56,
      ACCEL_OFFSET_Y_LSB_ADDR                                 = 0X57,
      ACCEL_OFFSET_Y_MSB_ADDR                                 = 0X58,
      ACCEL_OFFSET_Z_LSB_ADDR                                 = 0X59,
      ACCEL_OFFSET_Z_MSB_ADDR                                 = 0X5A,

      /* Magnetometer Offset registers */
      MAG_OFFSET_X_LSB_ADDR                                   = 0X5B,
      MAG_OFFSET_X_MSB_ADDR                                   = 0X5C,
      MAG_OFFSET_Y_LSB_ADDR                                   = 0X5D,
      MAG_OFFSET_Y_MSB_ADDR                                   = 0X5E,
      MAG_OFFSET_Z_LSB_ADDR                                   = 0X5F,
      MAG_OFFSET_Z_MSB_ADDR                                   = 0X60,

      /* Gyroscope Offset register s*/
      GYRO_OFFSET_X_LSB_ADDR                                  = 0X61,
      GYRO_OFFSET_X_MSB_ADDR                                  = 0X62,
      GYRO_OFFSET_Y_LSB_ADDR                                  = 0X63,
      GYRO_OFFSET_Y_MSB_ADDR                                  = 0X64,
      GYRO_OFFSET_Z_LSB_ADDR                                  = 0X65,
      GYRO_OFFSET_Z_MSB_ADDR                                  = 0X66,

      /* Radius registers */
      ACCEL_RADIUS_LSB_ADDR                                   = 0X67,
      ACCEL_RADIUS_MSB_ADDR                                   = 0X68,
      MAG_RADIUS_LSB_ADDR                                     = 0X69,
      MAG_RADIUS_MSB_ADDR                                     = 0X6A
    } bno055_reg_t;

    typedef enum
    {
      POWER_MODE_NORMAL                                       = 0X00,
      POWER_MODE_LOWPOWER                                     = 0X01,
      POWER_MODE_SUSPEND                                      = 0X02
    } bno055_powermode_t;

    typedef enum
    {
      /* Operation mode settings*/
      OPERATION_MODE_CONFIG                                   = 0X00,
      OPERATION_MODE_ACCONLY                                  = 0X01,
      OPERATION_MODE_MAGONLY                                  = 0X02,
      OPERATION_MODE_GYRONLY                                  = 0X03,
      OPERATION_MODE_ACCMAG                                   = 0X04,
      OPERATION_MODE_ACCGYRO                                  = 0X05,
      OPERATION_MODE_MAGGYRO                                  = 0X06,
      OPERATION_MODE_AMG                                      = 0X07,
      OPERATION_MODE_IMUPLUS                                  = 0X08,
      OPERATION_MODE_COMPASS                                  = 0X09,
      OPERATION_MODE_M4G                                      = 0X0A,
      OPERATION_MODE_NDOF_FMC_OFF                             = 0X0B,
      OPERATION_MODE_NDOF                                     = 0X0C
    } bno055_opmode_t;

    typedef enum
    {
      REMAP_CONFIG_P0                                         = 0x21,
      REMAP_CONFIG_P1                                         = 0x24, // default
      REMAP_CONFIG_P2                                         = 0x24,
      REMAP_CONFIG_P3                                         = 0x21,
      REMAP_CONFIG_P4                                         = 0x24,
      REMAP_CONFIG_P5                                         = 0x21,
      REMAP_CONFIG_P6                                         = 0x21,
      REMAP_CONFIG_P7                                         = 0x24
    } bno055_axis_remap_config_t;

    typedef enum
    {
      REMAP_SIGN_P0                                           = 0x04,
      REMAP_SIGN_P1                                           = 0x00, // default
      REMAP_SIGN_P2                                           = 0x06,
      REMAP_SIGN_P3                                           = 0x02,
      REMAP_SIGN_P4                                           = 0x03,
      REMAP_SIGN_P5                                           = 0x01,
      REMAP_SIGN_P6                                           = 0x07,
      REMAP_SIGN_P7                                           = 0x05
    } bno055_axis_remap_sign_t;

    typedef struct
    {
      uint8_t  accel_rev;
      uint8_t  mag_rev;
      uint8_t  gyro_rev;
      uint16_t sw_rev;
      uint8_t  bl_rev;
    } bno055_rev_info_t;

    typedef enum
    {
      VECTOR_ACCELEROMETER = BNO055_ACCEL_DATA_X_LSB_ADDR,
      VECTOR_MAGNETOMETER  = BNO055_MAG_DATA_X_LSB_ADDR,
      VECTOR_GYROSCOPE     = BNO055_GYRO_DATA_X_LSB_ADDR,
      VECTOR_EULER         = BNO055_EULER_H_LSB_ADDR,
      VECTOR_LINEARACCEL   = BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR,
      VECTOR_GRAVITY       = BNO055_GRAVITY_DATA_X_LSB_ADDR
    } vector_type_t;

    typedef enum
    {
        STATE_IDLE,
        STATE_GET_ID,
        STATE_SETUP,
        STATE_GET_TEMP,
        STATE_GET_CAL,
        STATE_GET_VECTOR,
        STATE_GET_STATUS,
        STATE_GET_OFFSETS,
        STATE_WRITE_OFFSETS
    } op_bno055_state_t;

// Constructor - default to abstract sensor ID -1, and BNO055 address A 
    OP_BNO055 ( int32_t sensorID = -1, uint8_t address = BNO055_ADDRESS_A );

// Public functions
    void  begin               ( void );
    void  checkIfPresent      ( void );
    bool  isPresent()         { return _deviceFound; }
    bool  setup               ( boolean usextal = true, bno055_opmode_t mode = OPERATION_MODE_NDOF );

    bool  process             ( void );
    bool  transactionDone     ( void );
    bool  transactionSuccessful( void );
    void  requestStatus       ( void ); 
    void  requestCalibration  ( void );
    void  requestVector       ( vector_type_t vector_type );
    void  requestEuler        ( void );
    void  requestTemp         ( void ); 

// Public variables
    op_bno055_state_t   state;
    op_bno055_state_t   lastState;
    imu_orientation_t   orientation;    // This is a struct of 3 floats: x, y, z
    imu_calibration_t   calibration;    // This is a struct of 4 unsigned ints: system, gyro, accel, mag (each with numbers 0-3)
    imu_status_t        status;         // This is a struct of 3 unsigned ints: systemStatus, selfTest, systemError
    int8_t              temperature;    // Temperature

// We aren't using the offset stuff for now
//  void  requestSensorOffsets( void );
//  void  setSensorOffsets_Accel ( void );
//  void  setSensorOffsets_Gyro ( void );
//  void  setSensorOffsets_Mag ( void );
//  uint8_t             offsetData[NUM_BNO055_OFFSET_REGISTERS];
//  bno055_offsets_t    offsets;

  private:
    void  getCalibration      ( void );
    void  getOffsets          ( void ); // We will already have offsets saved in the offsetData[] array, but this puts them into a convenient struct "offsets"
    void  getVector           ( void );
    void  getTemp             ( void ); 
    void  clearReg            ( void );
  
    void  read8   ( uint8_t *reg, uint8_t *value);
    void  readLen ( uint8_t *reg, uint8_t *buffer, uint8_t len);
   
// Private variables
    i2c_txn_t *t;               // Our I2C transaction

    uint8_t _address;           // Device address
    int32_t _sensorID;          // Abstract ID, not needed or used
    bno055_opmode_t _mode;      // What mode to set the sensor to
    
    uint8_t _chipID;            // Unique number that identifies a BNO055, as opposed to other devices on the bus
    boolean _deviceFound;
    uint8_t _calData; 
    
    vector_type_t _cur_vector_type; // What vector type did the user request
    uint8_t _vectorBuf[6];          // Vector data returned from the device is held in this array
    
    uint8_t _temp;              // Temp data returned from the device is held in this var
    
    uint8_t reg[MAX_OPERATIONS * 2];    // Yikes
};

#endif
