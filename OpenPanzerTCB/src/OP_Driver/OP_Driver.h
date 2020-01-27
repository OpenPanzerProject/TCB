/* OP_Driver.h      Open Panzer Driver - a library of driving functions
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
 

#ifndef OP_DRIVER_H
#define OP_DRIVER_H

#include <Arduino.h>
#include "../OP_Settings/OP_Settings.h"
#include "../OP_Motors/OP_Motors.h"
                                                   

typedef char DRIVETYPE;
#define DT_UNKNOWN      0
#define DT_TANK         1
#define DT_HALFTRACK    2
#define DT_CAR          3
#define DT_DKLM         4           // For the unusual DKLM gearboxes that use a single motor for propulsion and a second motor for steering. Basically same as a car, but we give it a unique drive type.
#define DT_DMD          5           // For the Tamiya DMD integrated ESCs and mixer units
#define DT_DIRECT       6           // Direct drive of each tread using individual radio channels
#define LAST_DT         DT_DIRECT
const __FlashStringHelper *printDriveType(DRIVETYPE Type); //Returns a character string that is name of the drive type.

typedef char _driveModes; 
#define UNKNOWN      0
#define STOP         1              // Should match Neutral _ManualTransGear
#define FORWARD      2              // Make sure forward and reverse numbers match the _ManualTransGear defines
#define REVERSE      3              // Make sure forward and reverse numbers match the _ManualTransGear defines
#define NEUTRALTURN  4
#define TRACK_RECOIL 5
#define LAST_MODE    TRACK_RECOIL
const __FlashStringHelper *printMode(_driveModes Type);     //Returns a character string that is name of the drive mode.

typedef char _ManualTransGear;
#define GEAR_NA      0              // Not using manual transmission, or not detected yet
#define GEAR_NEUTRAL 1              // Should match Stop _driveModes
#define GEAR_FORWARD 2              // Make sure forward and reverse numbers match the _driveModes defines
#define GEAR_REVERSE 3              // Make sure forward and reverse numbers match the _driveModes defines

// Accel/decel presets 
typedef char ACCEL_DRIVE_PRESET;
#define ADP_NONE                0   // This preset does nothing. The constraint is set by the user-selected level (skip num)
#define ADP_PRESET_1            1   // Accel preset 1
#define ADP_PRESET_2            2   // Accel preset 2
#define ADP_PRESET_3            3   // Accel preset 3
#define NUM_ACCEL_PRESETS       4   // Remember to count none
typedef char DECEL_DRIVE_PRESET;
#define DDP_NONE                0   // This preset does nothing. The constraint is set by the user-selected level (skip num)
#define DDP_PRESET_1            1   // Decel preset 1
#define DDP_PRESET_2            2   // Decel preset 2
#define DDP_PRESET_3            3   // Decel preset 3
#define NUM_DECEL_PRESETS       4   // Remember to count none

#define NUM_TURN_MODES          3   // How many turn modes are configured

#define MAX_SKIP_NUM            14  // At our default ramp step of 2, and with 256 updates per second, if we only increment drive speed every 14th interrupt
                                    // it will take 7 seconds to go from a complete stop to full speed (or vice versa). That is quite slow enough. 

#define DRIVE_RAMP_STEP_DEFAULT 2   // These are defaults, but they may be modified dynamically by the code or driving-preset algorithms. 
#define ACCEL_RAMP_STEP_DEFAULT 2   // They are not user-configurable in the desktop program. Instead, the user sets the frequency 
#define DECEL_RAMP_STEP_DEFAULT 2   // that drive speed increments are made. 

#define COMPLETE_STOP_NEARLIMIT 45  // If we are commanding the opposite throttle from the direction we are going, that is counted as a brake.
                                    // But if command far enough in the opposite direction, we want it to indicate an immediate, complete stop.
                                    // COMPLETE_STOP_NEARLIMIT is how close to the extreme opposite end of the throttle limit do we need to get,
                                    // before we consider it a complete stop command. If throttle command can be up to 255, a NEARLIMIT value of 50
                                    // means any opposite throttle of 210 or greater will be a complete stop. 

// DRIVING FUNCTIONS - set drive and throttle speed according to any acceleration/deceleration constraints, mix throttle and steering commands to 
// arrive at individual tread speeds, etc... 
class OP_Driver
{
public:
    OP_Driver(void);

    // Also configures timer3
    static void begin(DRIVETYPE, uint8_t, boolean, uint8_t, uint8_t);  // Variables are: Drive type, Turn mode, Neutral turn allowed, track recoil kickback speed, track recoil deceleration factor
    static void setDrivingProfileSettings(boolean, boolean, ACCEL_DRIVE_PRESET, DECEL_DRIVE_PRESET, uint8_t, uint8_t); // Variables are: accel ramp enabled, decel ramp enabled, accel preset, decel preset, accel skip num, decel skip num
    
    // called by the timer interrupt service routine, see the cpp file for details.
    static void OCR3A_ISR(void);                            // The ISR simply adjusts RampedDriveSpeed/RampedThrottleSpeed by +/- Step
    
    static boolean GetBrakeFlag(_driveModes, _driveModes);  // Are we braking? Pass previous drive mode and current drive mode command
    static _driveModes GetDriveMode(int, int);              // Return the drive mode from throttle and turn commands
    

    // We may want to change these on the fly
    static boolean isAccelRampEnabled(void) { return AccelRampEnabled; }
    static boolean isDecelRampEnabled(void) { return DecelRampEnabled; }
    static void setAccelRampFrequency(uint8_t);             // Modify the acceleration ramp frequency. 
    static uint8_t getAccelRampFrequency(void);             // Return
    static void setDecelRampFrequency(uint8_t);             // Modify the deceleration ramp frequency. 
    static uint8_t getDecelRampFrequency(void);             // Return
    static void setTurnMode(uint8_t);
    static uint8_t getTurnMode();
    static void setNeutralTurnAllowed(boolean);
    static boolean getNeutralTurnAllowed(void);
    int GetDriveSpeed(int, int, _driveModes, boolean);      // What is the *drive* speed. This is used to control the tank movement. 
    int GetThrottleSpeed(int, int, int, _driveModes, boolean); // What is the *engine* speed (different from drive speed) - this is used for the sound and smoker outputs
    int ScaleTurnCommand(int, int);                         // This scales a turn command to some lesser amount, used for neutral turns (tank mode) and turn command applied to rear treads in halftrack mode.
    void MixSteering(int, int, int*, int*);                 // This mixes throttle and turn commands into speeds for the left and right treads
                                                            // (int DriveSpeed, int TurnAmount, int *RightSpeed, int *LeftSpeed)

protected:  
    // Interrupts
    static void SetRampInterrupt(void);         // This determines if the interrupt can be turned off or needs to be left on
    static void DisableRampInterrupt(void);     // Disables interrupt
    static void EnableRampInterrupt(void);      // Enables interrupt

    // Driving 
    static uint8_t DriveType;                   // Tank, halftrack, car
    
    // Track recoil
    static uint8_t KickbackSpeed;               // Track recoil initial kick-back speed
    static float DecelerationFactor;            // Track recoil deceleration factor applied to kick-back speed
	static uint8_t TrackRecoilDuration;			// Duration of the simple track recoil in mS (values from 1 to 255)
    static uint32_t TrackRecoilStartTime;		// Time when simple track recoil begins, so we know when to stop it. 
	
    // Drive speed ramping
    static boolean DriveRampEnabled;            // Is ramping enabled for drive speed
    static volatile uint16_t RampedDriveSpeed;  // DRIVE speed constrained to ramping
    static volatile int8_t DriveRampStep;       // How much is drive speed changing each interrupt (step)
    static volatile int8_t DriveSkipNum;        // How many interrupts to skip before incrementing speed
    static volatile int8_t DriveSkipCount;      // Running count of interrupts skipped
    
    // Throttle speed ramping
    static boolean ThrottleRampEnabled;         // Is ramping enabled for throttle speed
    static volatile int16_t RampedThrottleSpeed;// THROTTLE speed constrained to ramping
    static volatile int8_t ThrottleRampStep;    // How much is throttle speed changing each interrupt (step)
    static volatile int8_t ThrottleSkipNum;     // How many interrupts to skip before incrementing speed
    static volatile int8_t ThrottleSkipCount;   // Running count of interrupts skipped      

    // User settings
    static boolean AccelRampEnabled;            // Should we ramp drive speed on acceleration? (Enforce inertial constraints)
    static boolean DecelRampEnabled;            // Should we ramp drive speed on deceleration? (Enforce momentum constraints)
    static uint8_t AccelSkipNum;                // What "level" should the constraint be - but this actually translates literally into the number of interrupts to skip
    static uint8_t DecelSkipNum;                // " "
    static ACCEL_DRIVE_PRESET AccelPreset;      // Has the user selected an acceleration preset algorithm
    static DECEL_DRIVE_PRESET DecelPreset;      // " "

    // Braking
    static int16_t Reverse_FullStopCmd;         // What forward command beyond which, when moving in reverse, a complete stop will be executed (rather than brake)
    static int16_t Forward_FullStopCmd;         // What reverse command beyond which, when moving forward, a complete stop will be executed (rather than braking)

    // Turns and Neutral Turn
    static uint8_t TurnMode;                    // Present turn mode
    static boolean NeutralTurnAllowed;          // Are neutral turns permitted
};


#endif

