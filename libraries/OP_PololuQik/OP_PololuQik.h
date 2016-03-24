/* OP_PololuQik.h   Open Panzer PololuQik library - functions for communicating with Pololu's Qik dual-serial motor controllers, 
 *                                                  such as the 2s12v10 and the 2s9v1 running the full Pololu Protocol in 7-bit mode.
 * Source:          http://www.Pololu.com
 * Authors:         Pololu
 *   
 * This library is a minor modification of the Pololu "qik-arduino" library.
 * For the original library, see their github page: https://github.com/pololu/qik-arduino
 *
 * All credit goes to Pololu. Their original copyright is listed below: 
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


// Commands
#define QIK_INIT_COMMAND                 0xAA

// 7-bit speed commands, we are not using the 8 bit option in Open Panzer
// We are however using the full Pololu Protocol (not the compact protocol)
#define QIK_MOTOR_M0_FORWARD             0x08
#define QIK_MOTOR_M0_REVERSE             0x0A
#define QIK_MOTOR_M1_FORWARD             0x0C
#define QIK_MOTOR_M1_REVERSE             0x0E

// Compact protocol commands:
//#define QIK_MOTOR_M0_FORWARD           0x88
//#define QIK_MOTOR_M0_REVERSE           0x8A
//#define QIK_MOTOR_M1_FORWARD           0x8C
//#define QIK_MOTOR_M1_REVERSE           0x8E

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
    static void autobaud(HardwareSerial *port, boolean dontWait = false);

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

  private:
    void throttleCommand(byte command, int speed) const;

  private:
    const byte      _deviceID;  
    HardwareSerial *_port;  
};


#endif