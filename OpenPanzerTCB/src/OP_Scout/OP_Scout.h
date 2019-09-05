/* OP_Scout.cpp     Library for communicating with the Open Panzer Scout dual brushed motor serial ESC
 * Source:          openpanzer.org      
 * Authors:         Luke Middleton
 *   
 * DESCRIPTION
 * ===============================================================================================================
 * This library is not part of the firmware for the Scout ESC - this library is used to communicate WITH the Scout ESC from some other microcontroller, such as an Arduino. 
 * 
 * The Scout responds to a serial protocol using the same data packet structure as the Dimension Engineering Sabertooth controllers. 
 * Some commands are shared with the Sabertooth line, but the Sabertooth has some commands the Scout does not, and the Scout has others the Sabertooth do not. 
 *
 * The serial packet is 4 bytes long: Address byte, command byte, data byte, checksum. 
 *
 * Unlike the Sabertooth line which have 8 device addresses (128-135), the Scout only has two possible addresses: 131 (Address A) and 132 (Address B). The device address is selected by the 
 * physical switch on the Scout. 
 * 
 *
 * FURTHER RESOURCES
 * ===============================================================================================================
 * Scout Firmware & Manual                      https://github.com/OpenPanzerProject/Scout-ESC
 * Scout Schematics, Board Files, and BOM:      http://openpanzer.org/downloads#scout
 *
 *
 * LICENSE
 * ===============================================================================================================
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


// Addresses
#define SCOUT_ADDRESS_A                     0x83    // 131
#define SCOUT_ADDRESS_B                     0x84    // 132

// Defaults
#define SCOUT_DEFAULT_CURRENT_LIMIT         12      // 12 amps by default per motor, this is the amount it should be able to maintain continuously without extra cooling
#define SCOUT_MAXIMUM_CURRENT_LIMIT         30      // 30 amps maximum current per motor

// Commands                                         // 0    Motor 1 forward
                                                    // 1    Motor 1 reverse
#define SCOUT_CMD_MIN_VOLTAGE               0x02    // 2    Set minimum voltage
#define SCOUT_CMD_MAX_VOLTAGE               0x03    // 3    Set maximum voltage
                                                    // 4    Motor 2 forward
                                                    // 5    Motor 2 reverse
                                                    // 6        Reserved for Motor 1 7-bit commands - not presently implemented
                                                    // 7        Reserved for Motor 2 7-bit commands - not presently implemented
                                                    // 8-13     Reserved - could add compatibility with Sabertooth mixed-mode commands, not presently implemented
#define SCOUT_CMD_SERIAL_WATCHDOG           0x0E    // 14
#define SCOUT_CMD_BAUD_RATE                 0x0F    // 15
                                                    // 16       Reserved for future compatibility with Sabertooth ramping features
                                                    // 17       Reserved for future compatibility with Sabertooth deadband feature
                                                    // 19   Unused
#define SCOUT_CMD_SET_FAN_SPEED             0x14    // 20   Direct fan speed control (or use it as a third, uni-directional ESC)
#define SCOUT_CMD_SET_AUTO_FAN_CONTROL      0x15    // 21   Revert fan control to Scout auto control (based on temperature)
#define SCOUT_CMD_SET_MAX_CURRENT           0x16    // 22   Set maximum current
#define SCOUT_CMD_BRAKE_AT_STOP             0x17    // 23   Command used to enable/disable braking motors when stopped (by shorting motor leads). Pass true to enable, false to disable. 
#define SCOUT_CMD_DRAG_INNER_TRACK          0x18    // 24   Command used to pulse brake commands to the slower of the two motors, can be used in tanks withe freewheeling gearboxes to assist in turning. See online documentation for more. 

// Codes
#define SCOUT_BAUD_CODE_2400                   1    // Codes for changing baud rates
#define SCOUT_BAUD_CODE_9600                   2    // These are the same codes used by certain Dimension Engineering Sabertooth controllers
#define SCOUT_BAUD_CODE_19200                  3    //
#define SCOUT_BAUD_CODE_38400                  4    //    
#define SCOUT_BAUD_CODE_115200                 5    //
#define SCOUT_BAUD_CODE_57600                  6    // The preceding codes are numbered identically to the codes used for Sabertooth controllers, which do not include 57600. That is why 57600 is number 6 and not number 5. 

class OP_Scout
{
public:
  
    // Initializes a new instance of the Scout class.
    // The driver address is set to the value given, and the specified serial port is used.
    OP_Scout(byte address, HardwareSerial *port);

public:
    // Return the driver address.
    inline byte address() const { return _address; }
  
    // Return the serial port.
    inline HardwareSerial* port() const { return _port; }

    // Sends a packet serial command to the Scout
    // command  The number of the command.
    // value    The command's value.
    void command(byte command, byte value) const;
  
    // Sets the speed of the specified motor
    // motor    The motor number, 1 or 2.
    // speed    The speed, between -127 and 127.
    void motor(byte motor, int speed) const;

    // Stops both motors
    void allStop() const;
    
    // Convenience functions
    
    // Manually set fan speed (can be used as a third, uni-directional, low current ESC)
    inline void SetFanSpeed(byte speed) { command(SCOUT_CMD_SET_FAN_SPEED, speed); }
    
    // Revert to automatic fan control based on temperature
    inline void AutoFanControl() { command(SCOUT_CMD_SET_AUTO_FAN_CONTROL, 0); }
  
    // Enable serial watchdog
    inline void EnableWatchdog(byte timeout) { command(SCOUT_CMD_SERIAL_WATCHDOG, timeout); }
    
    // Disable serial watchdog (send timeout value of 0)
    inline void DisableWatchdog() { command(SCOUT_CMD_SERIAL_WATCHDOG, 0); }
    
    // Brake at stop
    inline void BrakeAtStop(boolean enable) { command(SCOUT_CMD_BRAKE_AT_STOP, enable); }
  
    // Drag inner track
    inline void DragInnerTrack(boolean enable) { command(SCOUT_CMD_DRAG_INNER_TRACK, enable); }
    
    // Set maximum current
    inline void SetMaxCurrent(uint8_t cl) 
    { 
        if (cl > 0 && cl <= SCOUT_MAXIMUM_CURRENT_LIMIT) command(SCOUT_CMD_SET_MAX_CURRENT, cl); 
        else                                             command(SCOUT_CMD_SET_MAX_CURRENT, SCOUT_DEFAULT_CURRENT_LIMIT);
    }

    // Set minimum voltage
    // Used to set a custom minimum voltage for the battery supplying power (essentially LVC level). If battery voltage drops below this value, 
    // motors will be turned off. This value is cleared at startup, so must be set each run (Scout reverts to default min voltage of 6 volts at boot). 
    // The value is sent in 0.2 volt increments with a command of zero corresponding to 6v, which is the minimum. Valid data is from 0 to 50 (6-16 volts). 
    // The function for converting volts to command data is Value = (desired volts-6) x 5    
    inline void SetMinVoltage(uint8_t v)
    {
        if (v >= 0 && v <= 50) command(SCOUT_CMD_MIN_VOLTAGE, v);
    }
    
    // Set maximum voltage
    // Used to set a custom maximum voltage. If voltage exceeds this level, the motors will be turned off. The Scout reverts to a default maximum
    // voltage of 16 volts at boot, so this command must be sent at each startup if you want something different. 
    // The value is sent in 0.2 volt increments within the allowed range of 6-28 volts that equates to values of 30 to 140. 
    // The function for converting volts to command data is Value = (desired volts * 5)
    inline void SetMaxVoltage(uint8_t v)
    {
        if (v >= 30 && v <= 140) command(SCOUT_CMD_MAX_VOLTAGE, v);
    }    

private:
    void throttleCommand(byte command, int speed) const;
    
    const byte      _address;
    HardwareSerial *_port;

};

#endif
