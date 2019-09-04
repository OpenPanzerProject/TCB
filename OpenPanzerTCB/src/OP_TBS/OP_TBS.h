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

// Prop3 - 16 sounds in direct control mode (Benedini Flash v3.x only)
#define SOUND_OFF            0          // Sound 0:  Prop3 default (off - no special sound)
#define SOUND_TURRET         1          // Sound 1:  Turret rotation
#define SOUND_BARREL         2          // Sound 2:  Barrel elevation
#define SOUND_CANNON         3          // Sound 3:  Cannon fire
#define SOUND_MG             4          // Sound 4:  Machine gun
#define SOUND_CANNON_HIT     5          // Sound 5:  Received cannon hit - damage
#define SOUND_MG_HIT         6          // Sound 6:  Received machine gun hit - damage
#define SOUND_BATTLE_DESTROY 7          // Sound 7:  Received hit - vehicle destroyed
#define SOUND_HEADLIGHTS     8          // Sound 8:  Headlights on/off
#define SOUND_USER_1         9          // Sound 9:  Custom User Sound 1
#define SOUND_USER_2         10         // Sound 10: Custom User Sound 2 / Preheat sound
#define SOUND_USER_3         11         // Sound 11: Custom User Sound 3 / Second MG           
#define SOUND_USER_4         12         // Sound 12: Custom User Sound 4 / Squeak 1
#define SOUND_USER_5         13         // Sound 13: Custom User Sound 5 / Squeak 2
#define SOUND_USER_6         14         // Sound 14: Custom User Sound 6 / Squeak 3
#define SOUND_SQUEAK_1       12         // Sound 12: Squeak 1 (frequent)            - SAME AS USER SOUND 4
#define SOUND_SQUEAK_2       13         // Sound 13: Squeak 2 (medium frequency)    - SAME AS USER SOUND 5
#define SOUND_SQUEAK_3       14         // Sound 14: Squeak 3 (less frequent)       - SAME AS USER SOUND 6
#define SOUND_VOLUME_UP      15         // Sound 15: Increase volume
#define SOUND_VOLUME_DN      16         // Sound 16: Decrease volume
#define PROP3_NUM_SOUNDS     17         // How many sounds are there in total in the Prop3 register, including the SOUND_OFF "sound"
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
{1531, 0},  // Sound 0:  Prop3 default (off - no sound) - this is the odd Benedini center value
{864,  1},  // Sound 1:  Turret rotation
{945,  1},  // Sound 2:  Barrel elevation
{1026, 1},  // Sound 3:  Cannon fire
{1108, 2},  // Sound 4:  Machine gun fire - higher priority because it needs to repeat
{1189, 1},  // Sound 5:  Received cannon hit - damage
{1270, 1},  // Sound 6:  Received machine gun hit - damage
{1351, 4},  // Sound 7:  Received hit - vehicle destroyed - set to high priority though it shouldn't matter, nothing else will be playing at this time anyway
{1433, 1},  // Sound 8:  Headlights on/off
{1630, 3},  // Sound 9:  Custom User Sound 1 - even higher priority than MG 
{1711, 3},  // Sound 10: Custom User Sound 2 - even higher priority than MG (also used for pre-heat sound)
{1793, 3},  // Sound 11: Custom User Sound 3 - even higher priority than MG (also used for Second MG)
{1874, 3},  // Sound 12: Squeak 1 / Custom User Sound 4
{1955, 3},  // Sound 13: Squeak 2 / Custom User Sound 5
{2036, 3},  // Sound 14: Squeak 3 / Custom User Sound 6
{2117, 9},  // Sound 15: Increase volume - highest priority
{2199, 9}   // Sound 16: Decrease volume - highest priority
};
// We can't refer directly to array elements and struct members when using far addresses. For some reason, even using s*sizeof(Prop3) instead of (s*3) doesn't work. 
#define Prop3SoundPulse(s)      pgm_read_word_far(pgm_get_far_address(Prop3) + (s*3))       // Get address of Prop3 array, then skip ahead to the s-th element, since each struct is 3 bytes wide
#define Prop3SoundPriority(s)   pgm_read_byte_far(pgm_get_far_address(Prop3) + (s*3) + 2)   // Get address of Prop3 array, then skip ahead to the s-th element, then skip the next 2 bytes to get to the 3rd byte in the struct (Priority)

#define TBS_SIGNAL_mS         50        // How long to send a temporary signal for TBS to get it. 20ms didn't seem stable, so it needs to be greater than that, but as small as possible. 
#define TBS_SIGNAL_PROP2_mS   500       // Prop2 is used to toggle the engine. We find it works better with a longer signal, and because it doesn't interfere with other sounds it's fine to set it long. 

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
/*
#define SOUNDNAME_CHARS  31
const char _sound_descr_table_[PROP3_NUM_SOUNDS][SOUNDNAME_CHARS] PROGMEM_FAR = 
{   "Sound Off",                                // 0
    "Turret Rotation",                          // 1
    "Barrel Elevation",                         // 2
    "Cannon Fire",                              // 3
    "Machine Gun",                              // 4
    "Cannon Hit Received - Damage",             // 5
    "MG Hit Received - Damage",                 // 6
    "Hit Received - Destroyed",                 // 7
    "Headlights on/off",                        // 8
    "User Sound #1",                            // 9
    "User Sound #2",                            // 10
    "User Sound #3",                            // 11
    "Squeak 1 / User Sound #4",                 // 12
    "Squeak 2 / User Sound #5",                 // 13
    "Squeak 3 / User Sound #6",                 // 14
    "Increase volume",                          // 15
    "Decrease volume"                           // 16
};
*/

class OP_TBS
{
public:
    OP_TBS(OP_SimpleTimer * t, boolean Micro);              // Constructor
    void begin();                                           // Attach servo outputs and initialize
    void InitializeOutputs(void);                           // Initialize
                
    // PROP1: Engine speed sound                
    void SetEngineSpeed(int);                               // Send the engine speed to TBS
    void IdleEngine(void);                                  // Idle engine
    
    // PROP2: Engine startup/shutdown and second sound 
    void PROP2_OFF(void);                                   // Direct control of the Prop2 switch
    void ToggleEngineSound(void);                           // Toggle engine startup/shutdown
    void StartEngine(void);                                 // The Benedini device doesn't actually have an on/off (just toggle), but we need independent functions anyway 
    void StopEngine(void);                                  //     to account for different handling between Mini and Micro
    void Repair(void);                                      // Tank repair sound
    void StopRepairSound(void);                             // Explicit call to quit repair sound

    // PROP3: Individual Sounds
    void Cannon(void);                                      // Play cannon fire sound
    void MachineGun(void);                                  // Play machine gun sound
    void StopMachineGun(void);                              // Explicit call to stop the machine gun
    void SecondMachineGun(void);                            // Play second machine gun sound
    void StopSecondMachineGun(void);                        // Explicit call to stop the second machine gun    
    void Turret(void);                                      // Play turret rotation sound
    void StopTurret(void);                                  // Stop playing turret rotation sound
    void Barrel(void);                                      // Play barrel elevation sound
    void StopBarrel(void);                                  // Stop playing barel elevation sound    
    void MGHit(void);                                       // Play machine gun hit sound
    void CannonHit(void);                                   // Play cannon hit sound
    void Destroyed(void);                                   // Play tank destroyed sound
    void HeadlightSound(void);                              // Play the headlight on/off sound
    void PreHeatSound(void);                                // Play a sound to let the user know the engine start has been delayed in order to pre-heat the heating element of the smoker
    void UserSound_Play(uint8_t);                           // Play user sound x once
    void UserSound_Repeat(uint8_t);                         // Repeat user sound x
    void UserSound_Stop(uint8_t);                           // Stop user sound x
    void UserSound_StopAll(void);                           // Stop all user sounds
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
    void StartSqueaks(void);                                // Starts all squeaks
    void StopSqueaks(void);                                 // Stops all squeaks
    
//    void testProp3(void);                                   // Used for testing

 private:   
    static OP_Servos  * TBSProp;
    static OP_SimpleTimer * TBSTimer;                       // Pointer to the sketch's simple timer so we don't have to create a whole new one.
    static boolean      Micro;                              // Is this the Micro? If not, it's the Mini

    // PROP 1
    static void         ClearThrottleBlip(void);            // We will send a brief throttle signal to the Mini when the engine is started so it knows to wake up

    // PROP 2
    static int          TBSProp2TimerID;
    static boolean      Prop2TimerComplete; 
    static void         StartProp2Timer(void);
    static void         StartProp2Timer(int);               // Overloaded, in case we want to keep it on for a set amount of time. 
    static void         ClearProp2Timer(void);
    static boolean      EngineRunning;
    
    // PROP 3
    static void         TriggerSpecialSound(int, bool = true);     // eg, TriggerSpecialSound(SOUND_CANNON); // will trigger cannon sound. The second parameter is an optional boolean 
                                                            // to keep the sound on indefinitely, rather than triggering it once (if nothing is passed, it will trigger once)
    static void         StopSpecialSounds(int);             // Most special sounds only run once, but for repeated sounds (like machine gun), we can use this to turn them off.
    
    static int          TBSProp3TimerID;
    static boolean      Prop3TimerComplete; 
    static void         StartProp3Timer(void);
    static void         ClearProp3Timer(void);
    static uint8_t      currentProp3SoundNum;               // Which sound number is currently playing
    static uint8_t      previousProp3SoundNum;              // Which sound was playing before
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
    
};


#endif

