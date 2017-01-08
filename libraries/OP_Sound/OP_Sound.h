/* OP_Sound.h       Open Panzer Sound - container class for various supported sound devices
 * Source:          openpanzer.org              
 * Authors:         Luke Middleton
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

#ifndef OP_SOUND_H
#define OP_SOUND_H

#include <Arduino.h>
#include "OP_TBS.h"


typedef char SOUND_DEVICE;
#define SD_BENEDINI_TBSMINI     0
#define SD_OP_TEENSY            1
#define SD_BEIER_USMRC2         2
#define SD_FIRST_SD             SD_BENEDINI_TBSMINI
#define SD_LAST_SD              SD_BEIER_USMRC2
const __FlashStringHelper *printSoundDevice(SOUND_DEVICE Device); //Returns a character string that is name of the sound device


class OP_Sound {
  public:
    OP_Sound() {}                                       // Constructor
    virtual void begin() =0;                            // Setups
    
    // Engine sound functions   
    virtual void StartEngine(void) =0;  
    virtual void StopEngine(void) =0;   
    virtual void SetEngineSpeed(int) =0;                // Send the engine speed to TBS
    virtual void IdleEngine(void) =0;                   // Idle engine
    // Repair sounds    
    virtual void Repair(void) =0;                       // Tank repair sound
    virtual void StopRepairSound(void) =0;              // Explicit call to quit repair sound
    // Cannon   
    virtual void Cannon(void) =0;                       // Play cannon fire sound
    virtual void CannonHit(void) =0;                    // Play cannon hit sound
    virtual void Destroyed(void) =0;                    // Play tank destroyed sound
    // Machine Gun  
    virtual void MachineGun(void) =0;                   // Play machine gun sound
    virtual void StopMachineGun(void) =0;               // Explicit call to stop the machine gun
    virtual void MGHit(void) =0;                        // Play machine gun hit sound
    // Turret/Barrel
    virtual void Turret(void) =0;                       // Play turret rotation sound
    virtual void StopTurret(void) =0;                   // Stop turret rotation sound
    virtual void TurretSound_SetEnabled(boolean) =0;    // Turret sound enabled or not
    virtual void Barrel(void) =0;                       // Play barrel elevation sound
    virtual void StopBarrel(void) =0;                   // Stop barrel elevation sound
    virtual void BarrelSound_SetEnabled(boolean) =0;    // Barrel sound enabled or not
    // Headlight
    virtual void HeadlightSound(void) =0;               // Play the headlight on/off sound
    virtual void HeadlightSound_SetEnabled(boolean) =0; // Headlight sound enabled or not
    // User sounds
    virtual void UserSound1(void) =0;                   // Play user sound 1 once
    virtual void UserSound1_Repeat(void) =0;            // Repeat user sound 1
    virtual void UserSound1_Stop(void) =0;              // Stop user sound 1
    virtual void UserSound2(void) =0;                   // Play user sound 2 once
    virtual void UserSound2_Repeat(void) =0;            // Repeat user sound 2
    virtual void UserSound2_Stop(void) =0;              // Stop user sound 2
    // Squeaks  
    virtual void StartSqueaks(void) =0;                 // Starts all squeaks
    virtual void StopSqueaks(void) =0;                  // Stops all squeaks
    virtual boolean AreSqueaksActive(void) =0;          // Returns true or false if sqeaks are active
    virtual void SetSqueak1_Interval(unsigned int, unsigned int) =0;
    virtual void SetSqueak2_Interval(unsigned int, unsigned int) =0;
    virtual void SetSqueak3_Interval(unsigned int, unsigned int) =0;
    virtual void Squeak1_SetEnabled(boolean) =0;        // Enabled or disable Squeak1 
    virtual void Squeak2_SetEnabled(boolean) =0;        // Enabled or disable Squeak2
    virtual void Squeak3_SetEnabled(boolean) =0;        // Enabled or disable Squeak3 
    // Beeps
    virtual void Beep(void) =0;                         // Beep once
    virtual void Beeps(int) =0;                         // Beep number of times in a row

};



class BenediniTBS: public OP_Sound, public OP_TBS {
  public:
    BenediniTBS(OP_SimpleTimer * t) : OP_Sound(), OP_TBS(t)    {} // TBS requires pointer to SimpleTimer object
    void begin()                                                { OP_TBS::begin();                     }

    // Engine sound functions
    void StartEngine(void)                                      { OP_TBS::ToggleEngineSound();         }   // TBS doesn't have start/stop, just toggle
    void StopEngine(void)                                       { OP_TBS::ToggleEngineSound();         }   // TBS doesn't have start/stop, just toggle
    void SetEngineSpeed(int s)                                  { OP_TBS::SetEngineSpeed(s);           }
    void IdleEngine(void)                                       { OP_TBS::IdleEngine();                }
    // Repair sounds
    void Repair(void)                                           { OP_TBS::Repair();                    }
    void StopRepairSound(void)                                  { OP_TBS::StopRepairSound();           }
    // Cannon
    void Cannon(void)                                           { OP_TBS::Cannon();                    }
    void CannonHit(void)                                        { OP_TBS::CannonHit();                 }
    void Destroyed(void)                                        { OP_TBS::Destroyed();                 }
    // Machine Gun
    void MachineGun(void)                                       { OP_TBS::MachineGun();                }
    void StopMachineGun(void)                                   { OP_TBS::StopMachineGun();            }
    void MGHit(void)                                            { OP_TBS::MGHit();                     }
    // Turret/Barrel
    void Turret(void)                                           { OP_TBS::Turret();                    }
    void StopTurret(void)                                       { OP_TBS::StopTurret();                }
    void TurretSound_SetEnabled(boolean b)                      { OP_TBS::TurretSound_SetEnabled(b);   }
    void Barrel(void)                                           { return;                               } // TBS doesn't have barrel sound for now
    void StopBarrel(void)                                       { return;                               } // TBS doesn't have barrel sound for now
    void BarrelSound_SetEnabled(boolean)                        { return;                               } // TBS doesn't have barrel sound for now
    // Headlight
    void HeadlightSound(void)                                   { OP_TBS::HeadlightSound();            }
    void HeadlightSound_SetEnabled(boolean b)                   { OP_TBS::HeadlightSound_SetEnabled(b);}
    // User Sounds
    void UserSound1(void)                                       { OP_TBS::UserSound1();                }
    void UserSound1_Repeat(void)                                { OP_TBS::UserSound1_Repeat();         }
    void UserSound1_Stop(void)                                  { OP_TBS::UserSound1_Stop();           }
    void UserSound2(void)                                       { OP_TBS::UserSound2();                }
    void UserSound2_Repeat(void)                                { OP_TBS::UserSound2_Repeat();         }
    void UserSound2_Stop(void)                                  { OP_TBS::UserSound2_Stop();           }
    // Squeaks
    void StartSqueaks(void)                                     { OP_TBS::StartSqueaks();              }
    void StopSqueaks(void)                                      { OP_TBS::StopSqueaks();               }
    boolean AreSqueaksActive(void)                              { OP_TBS::AreSqueaksActive();          }
    void SetSqueak1_Interval(unsigned int a, unsigned int b)    { OP_TBS::SetSqueak1_Interval(a,b);    }
    void SetSqueak2_Interval(unsigned int a, unsigned int b)    { OP_TBS::SetSqueak2_Interval(a,b);    }
    void SetSqueak3_Interval(unsigned int a, unsigned int b)    { OP_TBS::SetSqueak3_Interval(a,b);    }
    void Squeak1_SetEnabled(boolean b)                          { OP_TBS::Squeak1_SetEnabled(b);       }
    void Squeak2_SetEnabled(boolean b)                          { OP_TBS::Squeak2_SetEnabled(b);       }
    void Squeak3_SetEnabled(boolean b)                          { OP_TBS::Squeak3_SetEnabled(b);       }
    // Beeps
    void Beep(void)                                             { return; }  // All TBS beeping stuff was removed to make space for another sound
    void Beeps(int)                                             { return; }  // It may be added back later if the TBS gets an update for use with the TCB
    // These are other functions used internally to the TBS class. If beeps get re-enabled you would probably map the above two to these two. 
    // void ForceBeep(void);                                    // Blocking call to beep
    // void ForceBeeps(int);                                    // Beep number of times in a row (blocks code)
};

class OP_TeensySound: public OP_Sound {
  public:
    OP_TeensySound(void) : OP_Sound() {} 
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

};


#endif // OP_SOUND_H

