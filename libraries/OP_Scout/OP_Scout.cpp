/* OP_Scout.cpp     Library for communicating with the Open Panzer Scout dual motor serial ESC
 * Source:          openpanzer.org              
 * Authors:         Luke Middleton
 *   
 * This library is not part of the firmware for the Scout ESC - it is used to communicate WITH the Scout ESC from some other device, such 
 * as the Open Panzer TCB (Tank Control Board). The protocol is basically the same as the one used by the Dimension Engineering Sabertooth controllers. 
 * The Scout only has two possible addresses (191 and 192) and it doesn't do auto-baud detection, instead you must set the baud rate using a dipswitch. 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */ 
 
#include "OP_Scout.h"


OP_Scout::OP_Scout(byte address, HardwareSerial *port) : _address(address), _port(port) { }

void OP_Scout::command(byte command, byte value) const
{
  _port->write(address());
  _port->write(command);
  _port->write(value);
  _port->write((address() + command + value) & B01111111);
}

void OP_Scout::throttleCommand(byte command, int speed) const
{
  speed = constrain(speed, -127, 127);
  this->command(command, (byte)abs(speed));
}

void OP_Scout::motor(byte motor, int speed) const
{
  if (motor < 1 || motor > 2) { return; }
  throttleCommand((motor == 2 ? 4 : 0) + (speed < 0 ? 1 : 0), speed);
}

void OP_Scout::allStop() const
{
  motor(1, 0);
  motor(2, 0);
}

