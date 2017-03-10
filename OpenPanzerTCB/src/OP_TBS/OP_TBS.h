/* OP_TBS.h         Open Panzer TBS - a library for controlling the Benedini TBS Mini sound unit
 * Source:          openpanzer.org              
 * Authors:         Luke Middleton
 *
 * This library provides functions to the Open Panzer project for interfacing with the Benedini TBS Mini sound unit. 
 * If you wish to purchase one of these sound units, visit Thomas Benedini's webpage at: http://www.benedini.de/
 *
 * Connect three servo cables from Prop1, Prop2 and Prop3 on the Open Panzer TCB board to Prop1, Prop2 and Prop3 on the Mini. 
 *   
 * NOTE: This will only work with Benedini TBS Flash v3 or later! Benedini released this version in spring of 2017. If you installed
 * TBS Flash before that, please update to the latest version. 
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

#ifndef OP_TBS_H
#define OP_TBS_H

#include "../OP_Settings/OP_Settings.h"
#include "../OP_Servo/OP_Servo.h"
#include "../OP_Motors/OP_Motors.h"
#include "../OP_SimpleTimer/OP_SimpleTimer.h"
#include "../OP_Tank/OP_BattleTimes.h"

// To save typing
#define PROP1               SERVONUM_PROP1
#define PROP2               SERVONUM_PROP2
#define PROP3               SERVONUM_PROP3

// Prop1 - Throttle speed
#define PROP1_IDLE          1500                // Idle throttle
#define PROP1_JUST_MOVING   1550                // What throttle value do we change from idle to moving sound (above 1500 which is center). No longer used since TBS Flash v3.0 and later. 
#define PROP1_FULL_SPEED    2000                // Full speed

// Prop2 - 2 sounds in direct control mode, you must emulate a 3-position switch
#define PROP2_SWITCH_OFF    1500                // Off position for Prop2 Function 1/2 (default)
#define PROP2_SWITCH_1      2000                // Prop2, sound for 2nd Coder Position 1 (We use it for engine startup/shutdown)
#define PROP2_SWITCH_2      1000                // Prop2, sound for 2nd Coder Position 2 (We use it for repair sound)
// We've disabled the beep-functionality in favor of using this slot for an actual sound
//#define BEEP_LENGTH_mS        350                 // How long is the beep sound in milliseconds

// Prop3 - 12 sounds for Benedini Flash V1
#define SOUND_OFF            0          // Sound 0:  Prop3 default (off - no special sound)
#define SOUND_TURRET         1          // Sound 1:  Turret rotation
#define SOUND_CANNON         2          // Sound 2:  Cannon fire
#define SOUND_MG             3          // Sound 3:  Machine gun
#define SOUND_CANNON_HIT     4          // Sound 4:  Received cannon hit - damage
#define SOUND_MG_HIT         5          // Sound 5:  Received machine gun hit - damage
#define SOUND_BATTLE_DESTROY 6          // Sound 6:  Received hit - tank destroyed
#define SOUND_HEADLIGHTS     7          // Sound 7:  Headlights on/off
#define SOUND_SQUEAK_1       8          // Sound 8:  Squeak 1 (frequent)
#define SOUND_SQUEAK_2       9          // Sound 9:  Squeak 2 (medium frequency)
#define SOUND_SQUEAK_3       10         // Sound 10: Squeak 3 (less frequent)
#define SOUND_USER_1         11         // Sound 11: Custom User Sound 1
#define SOUND_USER_2         12         // Sound 12: Custom User Sound 2
#define PROP3_NUM_SOUNDS     13         // How many sounds are there in total in the Prop3 register, including the SOUND_OFF "sound"

// The Prop3 numbers above refer to positions in the array below.
// The array holds the actual PWM values for each of the sound slots, plus one more for SOUND_OFF (1500)
// Each sound also has a priority number associated with it. If a sound is playing, and second sound is triggered, the second
// sound will only interrupt the first one if it has a higher priority. However, note that most sounds will be considered over 
// after TBS_SIGNAL_mS long (see the define below, very short), even though the actual sound may play longer than that. So it is 
// still possible for a lower priority sound to interrupt a higher priority sound if it is triggered TBS_SIGNAL_mS after the first one
// was triggered, unless we specifically told the first sound to remain on indefinitely, ie repeat (by pass true to TriggerSpecialSound). 
// Anyway, this is all sort of complicated, but the main reason for all of it is so that when the machine gun is firing, other sounds
// won't stop it (like squeaks). But you can tweak the priorities to create other effects as well. The important thing to remember is
// make sure SOUND_OFF has priority = 0 and every other sound has at least priority = 1. 
typedef struct {
    int16_t Pulse;
    uint8_t Priority;
} Prop3Settings;

// Benedini uses an odd spread of pulses from 800uS to 2280uS. This means an even center is not 1500 but rather 1540, although in the case
// of Prop 3 it doesn't really matter. What does matter is that we spread out each position as much as possible to take full advantage of the 
// range. However recall the OP Servo class won't generate signals greater than 2250 uS (but we keep it here to 2200). 
// We spread out each step by approximately 80uS with the exception of the two steps around center which we extend to approximately 100 uS. 
// Values were tweaked to correspond as closely to whole integers from 0-255 as possible (since that is actually how the TBS Mini reads the pulses
// apparently). The small tweaks are probably unnecessary and irrelevant once you account for all the variability in creating and reading PWM signals. 
const Prop3Settings Prop3[PROP3_NUM_SOUNDS] PROGMEM_FAR = {
{1500, 0},  // No sound
{1000, 1},  // Turret rotation
{1083, 1},  // Cannon fire
{1165, 2},  // Machine gun fire - higher priority because it needs to repeat
{1248, 1},  // Cannon hit
{1331, 1},  // Machine gun hit
{1413, 1},  // Vehicle destroyed
{1587, 1},  // Headlights
{1669, 1},  // Squeak 1
{1752, 1},  // Squeak 2
{1835, 1},  // Squeak 3 
{1917, 3},  // User Sound 1 - even higher priority than MG
{2000, 3}   // User Sound 2 - even higher priority than MG
};
// We can't refer directly to array elements and struct members when using far addresses. For some reason, even using s*sizeof(Prop3) instead of (s*3) doesn't work. 
#define Prop3SoundPulse(s)      pgm_read_word_far(pgm_get_far_address(Prop3) + (s*3))       // Get address of Prop3 array, then skip ahead to the s-th element, since each struct is 3 bytes wide
#define Prop3SoundPriority(s)   pgm_read_byte_far(pgm_get_far_address(Prop3) + (s*3) + 2)   // Get address of Prop3 array, then skip ahead to the s-th element, then skip the next 2 bytes to get to the 3rd byte in the struct (Priority)

#define TBS_SIGNAL_mS        30         // How long to send a temporary signal for TBS to get it. 20ms didn't seem stable, so it needs to be greater than that, but as small as possible. 

#define DEFAULT_SQUEAK_MIN_mS 800       // Min time between squeaks defaults to 0.8 seconds
#define DEFAULT_SQUEAK_MAX_mS 3500      // Max time between squeaks defaults to 3.5 seconds
#define SQUEAK_DELAY_mS       3000      // We don't start squeaking until this amount of time has passed after we first start moving
                                        // I suppose this should probably be stuck in EEPROM and let the user adjust it... 

// Let's create descriptions of these sounds so we can print them out during the teaching routine (EDIT: teaching routine is no longer required
// since TBS Flash v3.0 but these have still be useful in testing. For production we comment them out). 
// These must match the order they were defined in (see above). If SOUNDNAME_CHARS is 31, that means you have 30 (not 31) chars for the name.
// You must leave one char for the null terminator. 
// As with the function names in OP_FunctionsTriggers.h, this construct is wasteful of program space since we are reserving 31 chars
// per name whether we need that many or not. It is harder to address these elements as well, but the tradeoff is that we get to 
// keep these in FAR progmem (see OP_Settings.h for the exact location where), rather than in near which causes all sorts of problems, 
// AND we didn't have to make custom modifications to the linker script.
#define SOUNDNAME_CHARS  31
const char _sound_descr_table_[PROP3_NUM_SOUNDS][SOUNDNAME_CHARS] PROGMEM_FAR = 
{   "Sound Off",                            // 0
    "Turret Rotation",                      // 1
    "Cannon Fire",                          // 2
    "Machine Gun",                          // 3
    "Cannon Hit Received - Damage",         // 4
    "MG Hit Received - Damage",             // 5
    "Hit Received - Destroyed",             // 6
    "Headlights on/off",                    // 7
    "Squeak 1 - frequent",                  // 8
    "Squeak 2 - medium frequency",          // 9
    "Squeak 3 - less frequent",             // 10
    "User Sound #1",                        // 11
    "User Sound #2"                         // 12
};

class OP_TBS
{
public:
    OP_TBS(OP_SimpleTimer * t);                             // Constructor
    void begin();                                           // Attach servo outputs and initialize
    void InitializeOutputs(void);                           // Initialize
                
    // PROP1: Engine speed sound                
    void SetEngineSpeed(int);                               // Send the engine speed to TBS
    void IdleEngine(void);                                  // Idle engine
    
    // PROP2: Engine startup/shutdown and second sound 
    void PROP2_OFF(void);                                   // Direct control of the Prop2 switch
    void ToggleEngineSound(void);                           // Toggle engine startup/shutdown
    void Repair(void);                                      // Tank repair sound
    void StopRepairSound(void);                             // Explicit call to quit repair sound

//  All Beeping stuff was removed to make space for another sound
//  static void Beep(void);                                 // Program beep
//  static void ForceBeep(void);                            // Blocking call to beep
//  static void ForceBeeps(int);                            // Beep number of times in a row (blocks code)

    // PROP3: Individual Sounds
    void Cannon(void);                                      // Play cannon fire sound
    void MachineGun(void);                                  // Play machine gun sound
    void StopMachineGun(void);                              // Explicit call to stop the machine gun
    void Turret(void);                                      // Play turret rotation sound
    void StopTurret(void);                                  // Stop playing turret rotation sound
    void Barrel(void);                                      // Play barrel elevation sound
    void StopBarrel(void);                                  // Stop playing barel elevation sound    
    void MGHit(void);                                       // Play machine gun hit sound
    void CannonHit(void);                                   // Play cannon hit sound
    void Destroyed(void);                                   // Play tank destroyed sound
    void HeadlightSound(void);                              // Play the headlight on/off sound
    void UserSound_Play(uint8_t);                           // Play user sound x once
    void UserSound_Repeat(uint8_t);                         // Repeat user sound x
    void UserSound_Stop(uint8_t);                           // Stop user sound x
    void IncreaseVolume(void);                              // Increase volume
    void DecreaseVolume(void);                              // Decrease volume
    void StopVolume(void);                                  // Stop changing volume
                
    // Set enabled status of certain sounds         
    void HeadlightSound_SetEnabled(boolean);                // Headlight sound enabled or not
    void TurretSound_SetEnabled(boolean);                   // Turret sound enabled or not
    void BarrelSound_SetEnabled(boolean);                   // Barrel sound enabled or not

    // Squeak functions
    void SetSqueak_Interval(uint8_t, unsigned int, unsigned int); // Set squeak interval for squeak x
    void Squeak_SetEnabled(uint8_t, boolean);               // Enabled or disable Squeak x
    static void StartSqueaks(void);                         // Starts all squeaks
    void StopSqueaks(void);                                 // Stops all squeaks
    boolean AreSqueaksActive(void);                         // Returns true or false if sqeaks are active
    
    // Utilities
    void TeachEncoder(void);
    //    void testProp3(void);                                   // Used for testing

 private:   
    static OP_Servos  * TBSProp;
    static OP_SimpleTimer * TBSTimer;                       // Pointer to the sketch's simple timer so we don't have to create a whole new one.

    // PROP 2
    static int          TBSProp2TimerID;
    static boolean      Prop2TimerComplete; 
    static void         StartProp2Timer(void);
    static void         StartProp2Timer(int);               // Overloaded, in case we want to keep it on for a set amount of time. 
    static void         ClearProp2Timer(void);
    
    // PROP 3
    static void         TriggerSpecialSound(int, bool);     // eg, TriggerSpecialSound(SOUND_CANNON); // will trigger cannon sound. The second parameter is an optional boolean 
                                                            // to keep the sound on indefinitely, rather than triggering it once (if nothing is passed, it will trigger once)
    static void         StopSpecialSounds(void);            // Most special sounds only run once, but for repeated sounds (like machine gun), we can use this to turn them off.
    
    static int          TBSProp3TimerID;
    static boolean      Prop3TimerComplete; 
    static void         StartProp3Timer(void);
    static void         ClearProp3Timer(void);
    static uint8_t      currentProp3SoundNum;               // Which sound number is currently playing
    boolean             HeadlightSound_Enabled;
    boolean             TurretSound_Enabled;
    boolean             BarrelSound_Enabled;
    
    static void         StartSqueaksForReal(void);
    static void         Squeak1_Activate(void);
    static void         Squeak1_Pause(void);
    static void         Squeak1(void);
    static void         Squeak2_Activate(void);
    static void         Squeak2_Pause(void);
    static void         Squeak2(void);
    static void         Squeak3_Activate(void);
    static void         Squeak3_Pause(void);    
    static void         Squeak3(void);
    static boolean      Squeak1_Enabled;                    // Enabled means, are we going to use this sqeak or not
    static boolean      Squeak2_Enabled;                    // The user has the option of disabling some or all of the sqeaks in settings
    static boolean      Squeak3_Enabled;            
    static boolean      AllSqueaks_Active;          
    static boolean      Squeak1_Active;                     // Active means, is this sqeak now sqeaking, or is it waiting to sqeak
    static boolean      Squeak2_Active;                     // Squeaks are only active while the tank is moving. 
    static boolean      Squeak3_Active;
    static int          Squeak1TimerID;
    static int          Squeak2TimerID;
    static int          Squeak3TimerID;
    static unsigned int SQUEAK1_MIN_mS;
    static unsigned int SQUEAK1_MAX_mS;
    static unsigned int SQUEAK2_MIN_mS;
    static unsigned int SQUEAK2_MAX_mS;
    static unsigned int SQUEAK3_MIN_mS;
    static unsigned int SQUEAK3_MAX_mS;
    
    // BENEDINI FLASH v1
    static void         PulseDelayProp3(int);
    
};


#endif

