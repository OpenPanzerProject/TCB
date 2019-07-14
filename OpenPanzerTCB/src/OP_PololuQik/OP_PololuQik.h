/* OP_PololuQik.h   Open Panzer PololuQik library - functions for communicating with Pololu's Qik dual-serial motor controllers, 
 *                                                  such as the 2s12v10 and the 2s9v1 running the full Pololu Protocol in 7-bit mode.
 * Source:          http://www.Pololu.com
 * Authors:         Pololu
 *   
 * This library is a modification of the Pololu "qik-arduino" library, and should only be used with the Open Panzer project. 
 * For the original library, see Pololu's github page: https://github.com/pololu/qik-arduino
 *
 * The original Pololu copyright is listed below: 
 *
 * Copyright (c) 2012 Pololu Corporation.  For more information, see
 * http://www.pololu.com/
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 */ 

#ifndef OP_PololuQik_h
#define OP_PololuQik_h

#include <Arduino.h>
#include "../OP_Settings/OP_Settings.h"


// We are using the full Pololu Protocol (not the compact protocol)

// Commands
#define QIK_INIT_COMMAND                 0xAA
#define QIK_GET_ERROR_BYTE               0x82
#define QIK_GET_CONFIGURATION_PARAMETER  0x83
#define QIK_SET_CONFIGURATION_PARAMETER  0x84

#define QIK_PARAM_NUM_DEVICE_ID          0
#define QIK_PARAM_NUM_PWM                1
#define QIK_PARAM_NUM_SHTDN_ERR          2
#define QIK_PARAM_NUM_SER_TIMEOUT        3
#define QIK_PARAM_NUM_M0_ACCEL           4
#define QIK_PARAM_NUM_M1_ACCEL           5
#define QIK_PARAM_NUM_M0_BRAKE           6
#define QIK_PARAM_NUM_M1_BRAKE           7
#define QIK_PARAM_NUM_M0_CURLIMIT        8
#define QIK_PARAM_NUM_M1_CURLIMIT        9
#define QIK_PARAM_NUM_M0_CURRESPONSE     10
#define QIK_PARAM_NUM_M1_CURRESPONSE     11


// 7-bit speed commands, we are not using the 8 bit option in Open Panzer
#define QIK_MOTOR_M0_FORWARD             0x08
#define QIK_MOTOR_M0_REVERSE             0x0A
#define QIK_MOTOR_M1_FORWARD             0x0C
#define QIK_MOTOR_M1_REVERSE             0x0E
#define QIK_MOTOR_M0_BRAKE               0x06
#define QIK_MOTOR_M1_BRAKE               0x07
#define QIK_MOTOR_BRAKE_LEVEL            0x7F   // Maximum = 127

// CRC7 lookup table, takes up 256 bytes, stick it in PROGMEM
const unsigned char CRC7Table[256] PROGMEM_FAR = 
{
    0x00,
    0x41,0x13,0x52,0x26,0x67,0x35,0x74,0x4C,0x0D,0x5F,0x1E,0x6A,0x2B,0x79,0x38,
    0x09,0x48,0x1A,0x5B,0x2F,0x6E,0x3C,0x7D,0x45,0x04,0x56,0x17,0x63,0x22,0x70,
    0x31,0x12,0x53,0x01,0x40,0x34,0x75,0x27,0x66,0x5E,0x1F,0x4D,0x0C,0x78,0x39,
    0x6B,0x2A,0x1B,0x5A,0x08,0x49,0x3D,0x7C,0x2E,0x6F,0x57,0x16,0x44,0x05,0x71,
    0x30,0x62,0x23,0x24,0x65,0x37,0x76,0x02,0x43,0x11,0x50,0x68,0x29,0x7B,0x3A,
    0x4E,0x0F,0x5D,0x1C,0x2D,0x6C,0x3E,0x7F,0x0B,0x4A,0x18,0x59,0x61,0x20,0x72,
    0x33,0x47,0x06,0x54,0x15,0x36,0x77,0x25,0x64,0x10,0x51,0x03,0x42,0x7A,0x3B,
    0x69,0x28,0x5C,0x1D,0x4F,0x0E,0x3F,0x7E,0x2C,0x6D,0x19,0x58,0x0A,0x4B,0x73,
    0x32,0x60,0x21,0x55,0x14,0x46,0x07,0x48,0x09,0x5B,0x1A,0x6E,0x2F,0x7D,0x3C,
    0x04,0x45,0x17,0x56,0x22,0x63,0x31,0x70,0x41,0x00,0x52,0x13,0x67,0x26,0x74,
    0x35,0x0D,0x4C,0x1E,0x5F,0x2B,0x6A,0x38,0x79,0x5A,0x1B,0x49,0x08,0x7C,0x3D,
    0x6F,0x2E,0x16,0x57,0x05,0x44,0x30,0x71,0x23,0x62,0x53,0x12,0x40,0x01,0x75,
    0x34,0x66,0x27,0x1F,0x5E,0x0C,0x4D,0x39,0x78,0x2A,0x6B,0x6C,0x2D,0x7F,0x3E,
    0x4A,0x0B,0x59,0x18,0x20,0x61,0x33,0x72,0x06,0x47,0x15,0x54,0x65,0x24,0x76,
    0x37,0x43,0x02,0x50,0x11,0x29,0x68,0x3A,0x7B,0x0F,0x4E,0x1C,0x5D,0x7E,0x3F,
    0x6D,0x2C,0x58,0x19,0x4B,0x0A,0x32,0x73,0x21,0x60,0x14,0x55,0x07,0x46,0x77,
    0x36,0x64,0x25,0x51,0x10,0x42,0x03,0x3B,0x7A,0x28,0x69,0x1D,0x5C,0x0E,0x4F
};
#define GetCRC(i) pgm_read_byte_far(pgm_get_far_address(CRC7Table) + i)

class OP_PololuQik
{
  public:
    /*!
    Initializes a new instance of the PololuQik class.
    The device ID is set to the value given, and the specified serial port is used.
    \param deviceID The device ID to use.
    \param port     The port to use.
    */
    OP_PololuQik(byte deviceID, HardwareSerial *port);

  public:
    /*!
    Gets the device number.
    \return The device number.
    */
    inline byte deviceID() const { return _deviceID; }

    /*!
    Gets the serial port.
    \return The serial port.
    */
    inline HardwareSerial* port() const { return _port; }

    /*!
    Sends the autobaud character.
    \param dontWait If false, a delay is added to give the driver time to start up.
    */
    void autobaud(boolean dontWait = false) const;

    /*!
    Sends the autobaud character.
    \param port     The port to use.
    \param dontWait If false, a delay is added to give the driver time to start up.
    */
    void autobaud(HardwareSerial *port, boolean dontWait = false);

    /*!
    Sends a packet serial command to the motor driver.
    \param command The number of the command.
    \param value   The command's value.
    */
    void command(byte command, byte value) const;

    public:
    /*!
    Sets the speed of the specified motor.
    \param motor The motor number, 1 or 2.
    \param speed The speed, between -127 and 127.
    */
    void motor(byte motor, int speed) const;

    /*!
    Stops.
    */
    void allStop() const;
    
    /*!
    Configures the device for use with the TCB.
    \param SetDeviceID = what device ID to set the unit to (0-127)
    \returns true if configuration was successful, false if not. 
    \routine can take 
    */
    boolean configurePololu(byte SetDeviceID);


  private:
    void sendMessage(unsigned char message[], unsigned int length); // Sends a message of any length, including CRC

    void motorCommand(byte command, int speed) const;

    void clearError(void);

    //byte getConfigurationParameter(byte parameter);       // Presently we are only transmitting, not receiving. 
    byte setConfigurationParameter(byte parameter, byte value);

  private:
    const byte      _deviceID;  
    HardwareSerial *_port;  
};


#endif