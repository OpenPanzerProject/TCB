/* OP_Sabertooth.cpp Open Panzer Sabertooth library - 
 *                   functions for communicating with Dimension Engineering's Sabertooth seriers of dual-motor controllers, 
 *                                                   such as the 2X5, 2X12, 2X25, 2X32, 2X60
 * Source:           http://www.dimensionengineering.com/
 * Authors:          Dimension Engineering
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

#include "OP_Sabertooth.h"


OP_Sabertooth::OP_Sabertooth(byte address, HardwareSerial *port) : _address(address), _port(port)
{

}

void OP_Sabertooth::autobaud(boolean dontWait) const
{
  autobaud(port(), dontWait);
}

void OP_Sabertooth::autobaud(HardwareSerial *thisport, boolean dontWait)
{
  if (!dontWait) { delay(1500); }
  
  // Sabertooth V1 Devices (2x5, 2x10, 2x25V1)
  // ------------------------------------------------------------------------------------------------------------------------------>>
  // For these devices the baud rate is set automatically by the appearance of 170/0xAA at the desired baud rate  
  thisport->write(0xAA);
  thisport->flush();
  if (!dontWait) { delay(500); }
  
  // Sabertooth V2 Devices (2x12, 2x25V2, 2x60)
  // ------------------------------------------------------------------------------------------------------------------------------>>
  // These devices do NOT auto-detect the baud rate. The baud rate must be set by sending a Baud Rate command at whatever baud rate the device is already set at. 
  // They come from the factory with a default rate of 9600 (strange choice) but if it was changed before you will have no idea what the current rate is. In that case
  // the only way to issue a successful change baud rate command is to issue it once at every possible baud rate. After each command the Sabertooth will reboot which
  // takes time, and there are 5 possible rates. For that reason we do not bother trying to set the baud rate within this routine. If the user needs to change the 
  // baud rate on their V2 device they can use the utility provided in OP Config. 
  
}

void OP_Sabertooth::command(byte command, byte value) const
{
  _port->write(address());
  _port->write(command);
  _port->write(value);
  _port->write((address() + command + value) & B01111111);
}

void OP_Sabertooth::throttleCommand(byte command, int speed) const
{
  speed = constrain(speed, -127, 127);
  this->command(command, (byte)abs(speed));
}

void OP_Sabertooth::motor(byte motor, int speed) const
{
  if (motor < 1 || motor > 2) { return; }
  throttleCommand((motor == 2 ? 4 : 0) + (speed < 0 ? 1 : 0), speed);
}

void OP_Sabertooth::allStop() const
{
  motor(1, 0);
  motor(2, 0);
}

// For the 2x5, valid baud rates for packetized serial are 2400, 9600, 19200, and 38400. Some of the other controllers (2x60 for example)
// can also work at 115200. 
// 9600 is the default baud rate for Sabertooth packet serial for all their products, however, it does not work well with Arduino! 
// 38400 has been tested to work very well. 19200 may also work but faster is better. 
// NOTE: This routine is only useful if you already know what baud rate the Sabertooth is presently running at. Otherwise you won't know 
// what baud rate to send the command at; you will have to send the command once at every possible rate and wait after each transmission 
// to allow the Sabertooth to reboot. In our opinion this is a really poor setup. We prefer the auto-baud detection capabilities of the 2x5, 
// or else at least a predictable boot-up baud rate ala the Scout. 
void OP_Sabertooth::setBaudRate(long baudRate) const
{
  _port->flush();


  byte value;
  switch (baudRate)
  {
  case 2400:           value = 1; break;
  case 9600: default:  value = 2; break;
  case 19200:          value = 3; break;
  case 38400:          value = 4; break;
  case 115200:         value = 5; break;
  }
  command(15, value);

  _port->flush();
  
  // (1) flush() does not seem to wait until transmission is complete.
  //     As a result, a Serial.end() directly after this appears to
  //     not always transmit completely. So, we manually add a delay.
  // (2) Sabertooth takes about 200 ms after setting the baud rate to
  //     respond to commands again (it restarts).
  // So, this 500 ms delay should deal with this.
  delay(500);
}



