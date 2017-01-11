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
#define SD_OP_SOUND_CARD        1
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
    virtual void UserSound3(void) =0;                   // Play user sound 3 once
    virtual void UserSound3_Repeat(void) =0;            // Repeat user sound 3
    virtual void UserSound3_Stop(void) =0;              // Stop user sound 3
    virtual void UserSound4(void) =0;                   // Play user sound 4 once
    virtual void UserSound4_Repeat(void) =0;            // Repeat user sound 4
    virtual void UserSound4_Stop(void) =0;              // Stop user sound 4    
    // Squeaks  
    virtual void StartSqueaks(void) =0;                 // Starts all squeaks
    virtual void StopSqueaks(void) =0;                  // Stops all squeaks
    virtual boolean AreSqueaksActive(void) =0;          // Returns true or false if sqeaks are active
    virtual void SetSqueak1_Interval(unsigned int, unsigned int) =0;
    virtual void SetSqueak2_Interval(unsigned int, unsigned int) =0;
    virtual void SetSqueak3_Interval(unsigned int, unsigned int) =0;
    virtual void SetSqueak4_Interval(unsigned int, unsigned int) =0;
    virtual void SetSqueak5_Interval(unsigned int, unsigned int) =0;
    virtual void SetSqueak6_Interval(unsigned int, unsigned int) =0;    
    virtual void Squeak1_SetEnabled(boolean) =0;        // Enabled or disable Squeak1 
    virtual void Squeak2_SetEnabled(boolean) =0;        // Enabled or disable Squeak2
    virtual void Squeak3_SetEnabled(boolean) =0;        // Enabled or disable Squeak3 
    virtual void Squeak4_SetEnabled(boolean) =0;        // Enabled or disable Squeak4 
    virtual void Squeak5_SetEnabled(boolean) =0;        // Enabled or disable Squeak5
    virtual void Squeak6_SetEnabled(boolean) =0;        // Enabled or disable Squeak6    
    // Beeps
    virtual void Beep(void) =0;                         // Beep once
    virtual void Beeps(uint8_t) =0;                     // Beep number of times in a row
    // Volume
    virtual void SetVolume(uint8_t) =0;                 // Set the volume from off (0) to full on (100)

};




// ------------------------------------------------------------------------------------------------------------------>>
// BENEDINI TBS WRAPPER CLASS   See OP_TBS class for specific implementation of functions. 
//                              The TBS is controlled via three RC signals. 
// ------------------------------------------------------------------------------------------------------------------>>

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
    void Barrel(void)                                           { return;                              } // TBS doesn't have barrel sound for now
    void StopBarrel(void)                                       { return;                              } // TBS doesn't have barrel sound for now
    void BarrelSound_SetEnabled(boolean)                        { return;                              } // TBS doesn't have barrel sound for now
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
    void UserSound3(void)                                       { return;                              } // TBS doesn't have user sound 3 for now
    void UserSound3_Repeat(void)                                { return;                              } // TBS doesn't have user sound 3 for now
    void UserSound3_Stop(void)                                  { return;                              } // TBS doesn't have user sound 3 for now
    void UserSound4(void)                                       { return;                              } // TBS doesn't have user sound 4 for now
    void UserSound4_Repeat(void)                                { return;                              } // TBS doesn't have user sound 4 for now
    void UserSound4_Stop(void)                                  { return;                              } // TBS doesn't have user sound 4 for now
  // Squeaks
    void StartSqueaks(void)                                     { OP_TBS::StartSqueaks();              }
    void StopSqueaks(void)                                      { OP_TBS::StopSqueaks();               }
    boolean AreSqueaksActive(void)                              { OP_TBS::AreSqueaksActive();          }
    void SetSqueak1_Interval(unsigned int a, unsigned int b)    { OP_TBS::SetSqueak1_Interval(a,b);    }
    void SetSqueak2_Interval(unsigned int a, unsigned int b)    { OP_TBS::SetSqueak2_Interval(a,b);    }
    void SetSqueak3_Interval(unsigned int a, unsigned int b)    { OP_TBS::SetSqueak3_Interval(a,b);    }
    void SetSqueak4_Interval(unsigned int, unsigned int)        { return;                              } // TBS doesn't have squeak 4 for now
    void SetSqueak5_Interval(unsigned int, unsigned int)        { return;                              } // TBS doesn't have squeak 5 for now
    void SetSqueak6_Interval(unsigned int, unsigned int)        { return;                              } // TBS doesn't have squeak 6 for now
    void Squeak1_SetEnabled(boolean b)                          { OP_TBS::Squeak1_SetEnabled(b);       }
    void Squeak2_SetEnabled(boolean b)                          { OP_TBS::Squeak2_SetEnabled(b);       }
    void Squeak3_SetEnabled(boolean b)                          { OP_TBS::Squeak3_SetEnabled(b);       }
    void Squeak4_SetEnabled(boolean)                            { return;                              } // TBS doesn't have squeak 4 for now
    void Squeak5_SetEnabled(boolean)                            { return;                              } // TBS doesn't have squeak 5 for now
    void Squeak6_SetEnabled(boolean)                            { return;                              } // TBS doesn't have squeak 6 for now
  // Beeps
    void Beep(void)                                             { return; }  // All TBS beeping stuff was removed to make space for another sound
    void Beeps(uint8_t)                                         { return; }  // It may be added back later if the TBS gets an update for use with the TCB
  // Volume
    void SetVolume(uint8_t)                                     { return; }  // It is possible to adjust TBS volume remotely but not through this function, see the wiki for instructions
    // These are other functions used internally to the TBS class. If beeps get re-enabled you would probably map the above two to these two. 
    // void ForceBeep(void);                                    // Blocking call to beep
    // void ForceBeeps(int);                                    // Beep number of times in a row (blocks code)
};




// ------------------------------------------------------------------------------------------------------------------>>
// OPEN PANZER SOUND CARDS      Open Panzer sound cards are controlled via serial in a manner similar to the Scout ESC
// ------------------------------------------------------------------------------------------------------------------>>

// Open Panzer Sound Card address byte
#define OPSC_ADDRESS                        0xDA    // 218  Unique identifier for Open Panzer sound cards

// Commands                                         
#define OPSC_CMD_SERIAL_WATCHDOG             0x0E   // 14   Same as used for Scout
#define OPSC_CMD_BAUD_RATE                   0x0F   // 15   Same as used for Scout

#define OPSC_CMD_ENGINE_START                0x2C   // 44   Other Sound Card commands begin at 44
#define OPSC_CMD_ENGINE_STOP                 0x2D   // 45   
#define OPSC_CMD_ENGINE_SET_SPEED            0x2E   // 46
#define OPSC_CMD_ENGINE_SET_IDLE             0x2F   // 47
#define OPSC_CMD_REPAIR_START                0x30   // 48
#define OPSC_CMD_REPAIR_STOP                 0x31   // 49
#define OPSC_CMD_CANNON                      0x32   // 50
#define OPSC_CMD_CANNON_HIT                  0x33   // 51
#define OPSC_CMD_TANK_DESTROYED              0x34   // 52
#define OPSC_CMD_MG_START                    0x35   // 53
#define OPSC_CMD_MG_STOP                     0x36   // 54
#define OPSC_CMD_MG_HIT                      0x37   // 55
#define OPSC_CMD_TURRET_START                0x38   // 56
#define OPSC_CMD_TURRET_STOP                 0x39   // 57
#define OPSC_CMD_TURRET_ENABLE               0x3A   // 58
#define OPSC_CMD_BARREL_START                0x3B   // 59
#define OPSC_CMD_BARREL_STOP                 0x3C   // 60
#define OPSC_CMD_BARREL_ENABLE               0x3D   // 61
#define OPSC_CMD_HEADLIGHT                   0x3E   // 62
#define OPSC_CMD_HEADLIGHT_ENABLE            0x3F   // 63
#define OPSC_CMD_USER_SOUND_1                0x40   // 64
#define OPSC_CMD_USER_SOUND_1_REPEAT         0x41   // 65
#define OPSC_CMD_USER_SOUND_1_STOP           0x42   // 66
#define OPSC_CMD_USER_SOUND_2                0x43   // 67
#define OPSC_CMD_USER_SOUND_2_REPEAT         0x44   // 68
#define OPSC_CMD_USER_SOUND_2_STOP           0x45   // 69
#define OPSC_CMD_USER_SOUND_3                0x46   // 70
#define OPSC_CMD_USER_SOUND_3_REPEAT         0x47   // 71
#define OPSC_CMD_USER_SOUND_3_STOP           0x48   // 72
#define OPSC_CMD_USER_SOUND_4                0x49   // 73
#define OPSC_CMD_USER_SOUND_4_REPEAT         0x4A   // 74
#define OPSC_CMD_USER_SOUND_4_STOP           0x4B   // 75
#define OPSC_CMD_SQUEAKS_START               0x4C   // 76
#define OPSC_CMD_SQUEAKS_STOP                0x4D   // 77
#define OPSC_CMD_SQUEAK1_MIN                 0x4E   // 78
#define OPSC_CMD_SQUEAK1_MAX                 0x4F   // 79
#define OPSC_CMD_SQUEAK2_MIN                 0x50   // 80
#define OPSC_CMD_SQUEAK2_MAX                 0x51   // 81
#define OPSC_CMD_SQUEAK3_MIN                 0x52   // 82
#define OPSC_CMD_SQUEAK3_MAX                 0x53   // 83
#define OPSC_CMD_SQUEAK4_MIN                 0x54   // 84
#define OPSC_CMD_SQUEAK4_MAX                 0x55   // 85
#define OPSC_CMD_SQUEAK5_MIN                 0x56   // 86
#define OPSC_CMD_SQUEAK5_MAX                 0x57   // 87
#define OPSC_CMD_SQUEAK6_MIN                 0x58   // 88
#define OPSC_CMD_SQUEAK6_MAX                 0x59   // 89
#define OPSC_CMD_SQUEAK1_ENABLE              0x5A   // 90
#define OPSC_CMD_SQUEAK2_ENABLE              0X5B   // 91
#define OPSC_CMD_SQUEAK3_ENABLE              0x5C   // 92
#define OPSC_CMD_SQUEAK4_ENABLE              0x5D   // 93
#define OPSC_CMD_SQUEAK5_ENABLE              0x5E   // 94
#define OPSC_CMD_SQUEAK6_ENABLE              0x5F   // 95
#define OPSC_CMD_BEEP_ONCE                   0x60   // 96
#define OPSC_CMD_BEEP_X                      0x61   // 97
#define OPSC_CMD_SET_VOLUME                  0x62   // 98

// Codes
#define OPSC_BAUD_CODE_2400                   1     // Codes for changing baud rates, same numbers as used for Scout
#define OPSC_BAUD_CODE_9600                   2     // These are the same codes used by certain Dimension Engineering Sabertooth controllers
#define OPSC_BAUD_CODE_19200                  3     //
#define OPSC_BAUD_CODE_38400                  4     //    
#define OPSC_BAUD_CODE_115200                 5     //
#define OPSC_BAUD_CODE_57600                  6     // The preceding codes are numbered identically to the codes used for Sabertooth controllers, which does not include 57600. That is why 57600 is number 6 and not number 5. 

class OP_SoundCard: public OP_Sound {
  public:
    OP_SoundCard(HardwareSerial *p) : OP_Sound(), _port(p) {} 
    void begin(void);

  // Engine sound functions   
    void StartEngine(void)                                      { command(OPSC_CMD_ENGINE_START);               }   
    void StopEngine(void)                                       { command(OPSC_CMD_ENGINE_STOP);                }   
    // Speed should be some value between MOTOR_MAX_REVSPEED (-255) to MOTOR_MAX_FWDSPEED (255). 
    // The engine speed is always positive, so we use abs() to convert reverse values. If anything outside -255/255 is passed, 
    // behavior will be unpredictable! Keep in mind also, in the case of engine sound, a speed of 0 should correspond to engine idle, not engine stopped. 
    void SetEngineSpeed(int s)                                  { command(OPSC_CMD_ENGINE_SET_SPEED, abs(s));   }
    void IdleEngine(void)                                       { command(OPSC_CMD_ENGINE_SET_IDLE);            }   
  // Repair sounds    
    void Repair(void)                                           { command(OPSC_CMD_REPAIR_START);               }
    void StopRepairSound(void)                                  { command(OPSC_CMD_REPAIR_STOP);                }
  // Cannon   
    void Cannon(void)                                           { command(OPSC_CMD_CANNON);                     }
    void CannonHit(void)                                        { command(OPSC_CMD_CANNON_HIT);                 }
    void Destroyed(void)                                        { command(OPSC_CMD_TANK_DESTROYED);             }
  // Machine Gun  
    void MachineGun(void)                                       { command(OPSC_CMD_MG_START);                   }
    void StopMachineGun(void)                                   { command(OPSC_CMD_MG_STOP);                    }
    void MGHit(void)                                            { command(OPSC_CMD_MG_HIT);                     }
  // Turret/Barrel
    void Turret(void)                                           { command(OPSC_CMD_TURRET_START);               }
    void StopTurret(void)                                       { command(OPSC_CMD_TURRET_STOP);                }
    void TurretSound_SetEnabled(boolean b)                      { command(OPSC_CMD_TURRET_ENABLE, b);           }
    void Barrel(void)                                           { command(OPSC_CMD_BARREL_START);               }
    void StopBarrel(void)                                       { command(OPSC_CMD_BARREL_STOP);                }
    void BarrelSound_SetEnabled(boolean b)                      { command(OPSC_CMD_BARREL_ENABLE, b);           }
  // Headlight
    void HeadlightSound(void)                                   { command(OPSC_CMD_HEADLIGHT);                  }
    void HeadlightSound_SetEnabled(boolean b)                   { command(OPSC_CMD_HEADLIGHT_ENABLE, b);        }
  // User sounds                                  
    void UserSound1(void)                                       { command(OPSC_CMD_USER_SOUND_1);               }
    void UserSound1_Repeat(void)                                { command(OPSC_CMD_USER_SOUND_1_REPEAT);        }
    void UserSound1_Stop(void)                                  { command(OPSC_CMD_USER_SOUND_1_STOP);          }
    void UserSound2(void)                                       { command(OPSC_CMD_USER_SOUND_2);               }
    void UserSound2_Repeat(void)                                { command(OPSC_CMD_USER_SOUND_2_REPEAT);        }
    void UserSound2_Stop(void)                                  { command(OPSC_CMD_USER_SOUND_2_STOP);          }
    void UserSound3(void)                                       { command(OPSC_CMD_USER_SOUND_3);               }
    void UserSound3_Repeat(void)                                { command(OPSC_CMD_USER_SOUND_3_REPEAT);        }
    void UserSound3_Stop(void)                                  { command(OPSC_CMD_USER_SOUND_3_STOP);          }
    void UserSound4(void)                                       { command(OPSC_CMD_USER_SOUND_4);               }
    void UserSound4_Repeat(void)                                { command(OPSC_CMD_USER_SOUND_4_REPEAT);        }
    void UserSound4_Stop(void)                                  { command(OPSC_CMD_USER_SOUND_4_STOP);          }
  // Squeaks                
    void StartSqueaks(void)                                     { command(OPSC_CMD_SQUEAKS_START); _squeaksActive = true;  } 
    void StopSqueaks(void)                                      { command(OPSC_CMD_SQUEAKS_STOP);  _squeaksActive = false; }
    boolean AreSqueaksActive(void)                              { return _squeaksActive;                        }
    void SetSqueak1_Interval(unsigned int i, unsigned int a)    { command(OPSC_CMD_SQUEAK1_MIN, i); command(OPSC_CMD_SQUEAK1_MAX, a);   }
    void SetSqueak2_Interval(unsigned int i, unsigned int a)    { command(OPSC_CMD_SQUEAK2_MIN, i); command(OPSC_CMD_SQUEAK2_MAX, a);   }
    void SetSqueak3_Interval(unsigned int i, unsigned int a)    { command(OPSC_CMD_SQUEAK3_MIN, i); command(OPSC_CMD_SQUEAK3_MAX, a);   }
    void SetSqueak4_Interval(unsigned int i, unsigned int a)    { command(OPSC_CMD_SQUEAK4_MIN, i); command(OPSC_CMD_SQUEAK4_MAX, a);   }
    void SetSqueak5_Interval(unsigned int i, unsigned int a)    { command(OPSC_CMD_SQUEAK5_MIN, i); command(OPSC_CMD_SQUEAK5_MAX, a);   }
    void SetSqueak6_Interval(unsigned int i, unsigned int a)    { command(OPSC_CMD_SQUEAK6_MIN, i); command(OPSC_CMD_SQUEAK6_MAX, a);   }
    void Squeak1_SetEnabled(boolean b)                          { command(OPSC_CMD_SQUEAK1_ENABLE, b);          }
    void Squeak2_SetEnabled(boolean b)                          { command(OPSC_CMD_SQUEAK2_ENABLE, b);          }
    void Squeak3_SetEnabled(boolean b)                          { command(OPSC_CMD_SQUEAK3_ENABLE, b);          }
    void Squeak4_SetEnabled(boolean b)                          { command(OPSC_CMD_SQUEAK4_ENABLE, b);          }
    void Squeak5_SetEnabled(boolean b)                          { command(OPSC_CMD_SQUEAK5_ENABLE, b);          }
    void Squeak6_SetEnabled(boolean b)                          { command(OPSC_CMD_SQUEAK6_ENABLE, b);          }
  // Beeps
    void Beep(void)                                             { command(OPSC_CMD_BEEP_ONCE);                  }
    void Beeps(uint8_t x)                                       { command(OPSC_CMD_BEEP_X, x);                  }
  // Volume
    void SetVolume(uint8_t v)                                   { command(OPSC_CMD_SET_VOLUME, v);              }   // Expects a value from 0-100 

  // Functions specific to the OP_SoundCard sub-class
    inline HardwareSerial* port() const                         { return _port;                                 }   // Return the serial port.
    inline void EnableWatchdog(byte timeout)                    { command(OPSC_CMD_SERIAL_WATCHDOG, timeout);   }   // Enable serial watchdog
    inline void DisableWatchdog()                               { command(OPSC_CMD_SERIAL_WATCHDOG, 0);         }   // Disable serial watchdog (send timeout value of 0)
  
  private:
    // Sends a packet serial command to the sound card
    // command:  The number of the command, see defines above
    // value:    The command's value
    void command(byte command, byte value) const;
    void command(byte command) const;               // This version is used in cases where only a command is needed, no value. 
      
    // Class variables
    HardwareSerial *_port;
    boolean _squeaksActive;
  
};



#endif // OP_SOUND_H

