/* OP_SoundCard.h       Library for communicating via serial with Open Panzer sound cards
 * Source:              openpanzer.org      
 * Authors:             Luke Middleton
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


#ifndef OP_SOUNDCARD_H
#define OP_SOUNDCARD_H

#include "OP_Sound.h"

// Commands                                         // 0    Motor 1 forward
                                                    // 1    Motor 1 reverse
#define OPSC_CMD_SERIAL_WATCHDOG             0x0E   // 14
#define OPSC_CMD_BAUD_RATE                   0x0F   // 15
                                                    // 16       Reserved for future compatibility with Sabertooth ramping features
                                                    // 17       Reserved for future compatibility with Sabertooth deadband feature
                                                    // 19   Unused
#define OPSC_CMD_SET_MAX_CURRENT           0x16     // 22   Set maximum current

// Codes
#define OPSC_BAUD_CODE_2400                   1     // Codes for changing baud rates
#define OPSC_BAUD_CODE_9600                   2     // These are the same codes used by certain Dimension Engineering Sabertooth controllers
#define OPSC_BAUD_CODE_19200                  3     //
#define OPSC_BAUD_CODE_38400                  4     //    
#define OPSC_BAUD_CODE_115200                 5     //
#define OPSC_BAUD_CODE_57600                  6     // The preceding codes are numbered identically to the codes used for Sabertooth controllers, which does not include 57600. That is why 57600 is number 6 and not number 5. 

class OP_SoundCard: public OP_Sound {
  
public:
    OP_SoundCard(HardwareSerial *p) : OP_Sound(), _port(p) {} 

public:
    // Required by OP_Sound
    void begin(void);
    // Engine sound functions   
    void StartEngine(void);
    void StopEngine(void);
    void SetEngineSpeed(int);
    void IdleEngine(void);
    // Repair sounds    
    void Repair(void);
    void StopRepairSound(void);
    // Cannon   
    void Cannon(void);
    void CannonHit(void);
    void Destroyed(void);
    // Machine Gun  
    void MachineGun(void);
    void StopMachineGun(void);
    void MGHit(void);
    // Turret/Barrel
    void Turret(void);
    void StopTurret(void);
    void TurretSound_SetEnabled(boolean);
    void Barrel(void);
    void StopBarrel(void);
    void BarrelSound_SetEnabled(boolean);
    // Headlight
    void HeadlightSound(void);
    void HeadlightSound_SetEnabled(boolean);
    // User sounds
    void UserSound1(void);
    void UserSound1_Repeat(void);
    void UserSound1_Stop(void);
    void UserSound2(void);
    void UserSound2_Repeat(void);
    void UserSound2_Stop(void);
    // Squeaks  
    void StartSqueaks(void);
    void StopSqueaks(void);
    boolean AreSqueaksActive(void);
    void SetSqueak1_Interval(unsigned int, unsigned int);
    void SetSqueak2_Interval(unsigned int, unsigned int);
    void SetSqueak3_Interval(unsigned int, unsigned int);
    void Squeak1_SetEnabled(boolean);
    void Squeak2_SetEnabled(boolean);
    void Squeak3_SetEnabled(boolean);
    // Beeps
    void Beep(void);
    void Beeps(int);


    // OP_SoundCard-specific
  
    // Return the serial port.
    inline HardwareSerial* port() const { return _port; }

    // Enable serial watchdog
    inline void EnableWatchdog(byte timeout) { command(OPSC_CMD_SERIAL_WATCHDOG, timeout); }
    
    // Disable serial watchdog (send timeout value of 0)
    inline void DisableWatchdog() { command(OPSC_CMD_SERIAL_WATCHDOG, 0); }
    
    
  

private:
    // Sends a packet serial command to the Scout
    // command  The number of the command.
    // value    The command's value.
//    void command(byte command, byte value) const;
      
    HardwareSerial *_port;
  
};


#endif // OP_SOUNDCARD_H

