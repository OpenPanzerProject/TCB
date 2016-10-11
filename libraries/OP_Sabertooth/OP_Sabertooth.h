/* OP_Sabertooth.h  Open Panzer Sabertooth library - 
 *                  functions for communicating with Dimension Engineering's Sabertooth seriers of dual-motor controllers, 
 *                                                   such as the 2X5, 2X12, 2X25, 2X32, 2X60
 * Source:          http://www.dimensionengineering.com/
 * Authors:         Dimension Engineering
 *   
 * This library is a stripped-down version of DE's original for use in the Open Panzer project.
 * Please use the full version of the library for any other application. 
 * The original copyright is reprinted below: 
 * 
 * Arduino Libraries for SyRen/Sabertooth
 * Copyright (c) 2012-2013 Dimension Engineering LLC
 * http://www.dimensionengineering.com/arduino
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef OP_Sabertooth_h
#define OP_Sabertooth_h   

#include <Arduino.h>

#define SABERTOOTH_CMD_SERIALTIMEOUT    0x0E    // Decimal 14
#define SABERTOOTH_CMD_BAUDRATE         0x0F    // Decimal 15
    #define SABERTOOTH_BAUD_2400        1       //      Baud value byte for 2400
    #define SABERTOOTH_BAUD_9600        2       //      Baud value byte for 9600
    #define SABERTOOTH_BAUD_19200       3       //      Baud value byte for 19200
    #define SABERTOOTH_BAUD_38400       4       //      Baud value byte for 38400
    #define SABERTOOTH_BAUD_115200      5       //      Baud value byte for 115200
#define SABERTOOTH_CMD_RAMPING          0x10    // Decimal 16   NOT USED IN OP
#define SABERTOOTH_CMD_DEADBAND         0x11    // Decimal 17   NOT USED IN OP


/*!
\class OP_Sabertooth
\brief Controls a %Sabertooth or %SyRen motor driver running in Packet Serial mode.
*/
class OP_Sabertooth
{
public:
  
  /*!
  Initializes a new instance of the Sabertooth class.
  The driver address is set to the value given, and the specified serial port is used.
  \param address The driver address.
  \param port    The port to use.
  */
  OP_Sabertooth(byte address, HardwareSerial *port);

public:
  /*!
  Gets the driver address.
  \return The driver address.
  */
  inline byte address() const { return _address; }
  
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
  
public:
  /*!
  Sets the baud rate.
  Baud rate is stored in EEPROM, so changes persist between power cycles.
  \param baudRate The baud rate. This can be 2400, 9600, 19200, 38400, or on some drivers 115200.
  */
  void setBaudRate(long baudRate) const;
 
private:
  void throttleCommand(byte command, int speed) const;
  
private:
  const byte      _address;
  HardwareSerial *_port;
};

#endif
