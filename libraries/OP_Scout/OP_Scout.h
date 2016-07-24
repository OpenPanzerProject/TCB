/* OP_Scout.h       Library for communicating with the Open Panzer Scout dual motor serial ESC
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

#ifndef OP_Scout_h
#define OP_Scout_h   

#include <Arduino.h>


// Commands
#define SCOUT_CMD_SET_FAN_SPEED             0x14    // 20
#define SCOUT_CMD_SET_AUTO_FAN_CONTROL      0x15    // 21
#define SCOUT_CMD_SET_MAX_CURRENT           0x16    // 22
#define SCOUT_CMD_ENABLE_SERIAL_WATCHDOG    0x17    // 23
#define SCOUT_CMD_DISABLE_SERIAL_WATCHDOG   0x18    // 24


class OP_Scout
{
public:
  
    // Initializes a new instance of the Sabertooth class.
    // The driver address is set to the value given, and the specified serial port is used.
    OP_Scout(byte address, HardwareSerial *port);

public:
    // Gets the driver address.
    inline byte address() const { return _address; }
  
    // Gets the serial port.
    inline HardwareSerial* port() const { return _port; }

    // Sends a packet serial command to the motor driver.
    // command The number of the command.
    // value   The command's value.
  void command(byte command, byte value) const;
  
    // Sets the speed of the specified motor.
    // motor The motor number, 1 or 2.
    // speed The speed, between -127 and 127.
    void motor(byte motor, int speed) const;
   
    // Stops everything
    void allStop() const;
  

private:
    void throttleCommand(byte command, int speed) const;
    
    const byte      _address;
    HardwareSerial *_port;

};

#endif
