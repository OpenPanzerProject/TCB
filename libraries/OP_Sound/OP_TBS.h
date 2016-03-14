/* OP_TBS.h			Open Panzer TBS - a library for controlling the Benedini TBS Mini sound unit
 * Source: 			openpanzer.org				
 * Authors:    		Luke Middleton
 *
 * This library provides functions to the Open Panzer project for interfacing with the Benedini TBS Mini sound unit. 
 * If you wish to purchase one of these sound units, visit Thomas Benedini's webpage at: http://www.benedini.de/
 *
 * Connect three servo cables from Prop1, Prop2 and Prop3 on the Open Panzer TCB board to Prop1, Prop2 and Prop3 on the Mini. 
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

#include <Arduino.h>
#include <avr/pgmspace.h>
#include "OP_Settings.h"
#include "OP_Servo.h"
#include "OP_Motors.h"
#include "OP_SimpleTimer.h"
#include "OP_BattleTimes.h"

// To save typing
#define PROP1 				SERVONUM_PROP1
#define PROP2 				SERVONUM_PROP2
#define PROP3 				SERVONUM_PROP3

// Prop1 - Throttle speed
#define PROP1_IDLE			1500				// Idle throttle
#define PROP1_JUST_MOVING 	1550				// What throttle value do we change from idle to moving sound (above 1500 which is center) 
#define PROP1_FULL_SPEED    2000				// Full speed

// Prop2 - 2 sounds in direct control mode, you must emulate a 3-position switch
#define PROP2_SWITCH_OFF	1500				// Off position for Prop2 Function 1/2 (default)
#define PROP2_SWITCH_1		2000				// Prop2, sound for 2nd Coder Position 1 (We use it for engine startup/shutdown)
#define PROP2_SWITCH_2		1000				// Prop2, sound for 2nd Coder Position 2 (We use it for repair sound)
// We've disabled the beep-functionality in favor of using this slot for an actual sound
//#define BEEP_LENGTH_mS		350					// How long is the beep sound in milliseconds

// Prop3 - 12 sounds in direct control mode, you must emulate a 12-position switch
#define SOUND_OFF 			 0			// Sound 0:  Prop3 default (off - no special sound)
#define SOUND_TURRET		 1			// Sound 1:  Turret rotation
#define SOUND_CANNON		 2			// Sound 2:  Cannon fire
#define SOUND_MG			 3			// Sound 3:  Machine gun
#define SOUND_CANNON_HIT	 4			// Sound 4:  Received cannon hit - damage
#define SOUND_MG_HIT		 5			// Sound 5:  Received machine gun hit - damage
#define SOUND_BATTLE_DESTROY 6			// Sound 6:  Received hit - tank destroyed
#define SOUND_HEADLIGHTS	 7			// Sound 7:  Headlights on/off
#define SOUND_SQUEAK_1		 8			// Sound 8:  Squeak 1 (frequent)
#define SOUND_SQUEAK_2		 9			// Sound 9:  Squeak 2 (medium frequency)
#define SOUND_SQUEAK_3		 10			// Sound 10: Squeak 3 (less frequent)
#define SOUND_USER_1		 11			// Sound 11: Custom User Sound 1
#define SOUND_USER_2		 12 		// Sound 12: Custom User Sound 2
// The Prop3 numbers above refer to positions in the array below.
// The array holds the actual PWM values for each of the 12 sound slots, plus one more for SOUND_OFF (1500) (so 13 total)
// Each sound also has a priority number associated with it. If a sound is playing, and second sound is triggered, the second
// sound will only interrupt the first one if it has a higher priority. However, note that most sounds will be considered over 
// after TBS_SIGNAL_mS long (see the define below, very short), even though the actual sound may play longer than that. So it is 
// still possible for a lower priority sound to interrupt a higher priority sound if is triggered after TBS_SIGNAL_mS the first one
// was triggered, unless we specifically told the first sound to remain on indefinitely (pass true to TriggerSpecialSound). 
// Anyway, this is all sort of complicated, but the main reason for all of it is so that when the machine gun is firing, other sounds
// won't stop it (like squeaks). But you can tweak the priorities to create other effects as well. The important thing to remember is
// make sure SOUND_OFF has priority = 0 and every other sound has at least priority = 1. 
typedef struct {
	int16_t Pulse;
	uint8_t Priority;
} Prop3Settings;
const PROGMEM Prop3Settings Prop3[13] = {
{1500, 0},	// No sound
{1000, 1},	// Turret rotation
{1083, 1}, 	// Cannon fire
{1165, 2}, 	// Machine gun fire	- higher priority because it needs to repeat
{1248, 1}, 	// Cannon hit
{1331, 1}, 	// Machine gun hit
{1413, 1}, 	// Vehicle destroyed
{1587, 1}, 	// Headlights
{1669, 1}, 	// Squeak 1
{1752, 1}, 	// Squeak 2
{1835, 1},	// Squeak 3 
{1917, 3},	// User Sound 1 - even higher priority than MG
{2000, 3}	// User Sound 2 - even higher priority than MG
};
#define Prop3SoundPulse(s)	  pgm_read_word_near(&Prop3[s].Pulse)
#define Prop3SoundPriority(s) pgm_read_byte_near(&Prop3[s].Priority)

#define TBS_SIGNAL_mS		 30			// How long to send a temporary signal for TBS to get it. 20ms didn't seem stable, so it needs to be greater than that, but as small as possible. 

#define DEFAULT_SQUEAK_MIN_mS 800		// Min time between squeaks defaults to 0.8 seconds
#define DEFAULT_SQUEAK_MAX_mS 3500		// Max time between squeaks defaults to 3.5 seconds
#define SQUEAK_DELAY_mS		  3000		// We don't start squeaking until this amount of time has passed after we first start moving

// Let's create descriptions of these 12 sounds so we can print them out during the teaching routine
// THESE MUST MATCH THE 12 DEFINES ABOVE ^
const char sndDescr1[] PROGMEM = "Turret Rotation";
const char sndDescr2[] PROGMEM = "Cannon Fire";
const char sndDescr3[] PROGMEM = "Machine Gun";
const char sndDescr4[] PROGMEM = "Cannon Hit Received - Damage";
const char sndDescr5[] PROGMEM = "MG Hit Received - Damage";
const char sndDescr6[] PROGMEM = "Hit Received - Destroyed";
const char sndDescr7[] PROGMEM = "Headlights on/off";
const char sndDescr8[] PROGMEM = "Squeak 1 - frequent";
const char sndDescr9[] PROGMEM = "Squeak 2 - medium frequency";
const char sndDescr10[] PROGMEM = "Squeak 3 - less frequent";
const char sndDescr11[] PROGMEM = "User Sound #1";
const char sndDescr12[] PROGMEM = "User Sound #2";
// Then set up a table to refer to our strings
const char* const sound_descr_table[] PROGMEM = {sndDescr1, sndDescr2, sndDescr3, sndDescr4, sndDescr5, sndDescr6, sndDescr7, sndDescr8, sndDescr9, sndDescr10, sndDescr11, sndDescr12};


class OP_TBS
{
	// We are using static for everything because we only want one instance of this class. 
public:
	// Initalize
	OP_TBS();									// Constructor
    static void begin(void);					// Attach servo outputs and initialize

	// PROP1: Engine speed sound
	static void SetEngineSpeed(int);			// Send the engine speed to TBS
	static void IdleEngine(void);				// Idle engine
	
	// PROP2: Engine startup/shutdown and second sound 
	static void PROP2_OFF(void);				// Direct control of the Prop2 switch
	static void ToggleEngineSound(void);		// Toggle engine startup/shutdown
	static void Repair(void);					// Tank repair sound
	static void StopRepairSound(void);			// Explicit call to quit repair sound

//  All Beeping stuff was removed to make space for another sound
//	static void Beep(void);						// Program beep
//	static void ForceBeep(void);				// Blocking call to beep
//	static void ForceBeeps(int);				// Beep number of times in a row (blocks code)

	// PROP3: Generic special sound functions
	static void TriggerSpecialSound(int, bool);	// eg, TriggerSpecialSound(SOUND_CANNON); // will trigger cannon sound. The second parameter is an optional boolean 
												// to keep the sound on indefinitely, rather than triggering it once (if nothing is passed, it will trigger once)
	static void StopSpecialSounds(void);		// Most special sounds only run once, but for repeated sounds (like machine gun), we can use this to turn them off.

	// PROP3: Individual Sounds
	static void Cannon(void);					// Play cannon fire sound
	static void MachineGun(void);				// Play machine gun sound
	static void StopMachineGun(void);			// Explicit call to stop the machine gun
	static void Turret(void);					// Play turret rotation sound
	static void MGHit(void);					// Play machine gun hit sound
	static void CannonHit(void);				// Play cannon hit sound
	static void Destroyed(void);				// Play tank destroyed sound
	static void HeadlightSound(void);			// Play the headlight on/off sound
	static void UserSound1(void);				// Play user sound 1 once
	static void UserSound1_Repeat(void);		// Repeat user sound 1
	static void UserSound1_Stop(void);			// Stop user sound 1
	static void UserSound2(void);				// Play user sound 2 once
	static void UserSound2_Repeat(void);		// Repeat user sound 2
	static void UserSound2_Stop(void);			// Stop user sound 2
	
	// Set enabled status of certain sounds
	static void Squeak1_SetEnabled(boolean);		// Enabled or disable Squeak1 
	static void Squeak2_SetEnabled(boolean);		// Enabled or disable Squeak2
	static void Squeak3_SetEnabled(boolean);		// Enabled or disable Squeak3 
	static void HeadlightSound_SetEnabled(boolean);	// Headlight sound enabled or not
	static void TurretSound_SetEnabled(boolean);	// Turret sound enabled or not
	
	static void StartSqueaks(void);				// starts all squeaks
	static void StopSqueaks(void);				// Stops all squeaks
	static boolean AreSqueaksActive(void);		// Returns true or false if sqeaks are active
	static void SetSqueak1_Interval(unsigned int, unsigned int);
	static void SetSqueak2_Interval(unsigned int, unsigned int);
	static void SetSqueak3_Interval(unsigned int, unsigned int);

	// Utilities
	static void UpdateTimer(void);
	static void TeachEncoder(void);

	

 private:	
    static OP_Servos  * TBSProp;
	static OP_SimpleTimer TBSTimer;
	// PROP 2
	static int 			TBSProp2TimerID;
	static boolean 		Prop2TimerComplete;	
	static void 		StartProp2Timer(void);
	static void			StartProp2Timer(int);	// Overloaded, in case we want to keep it on for a set amount of time. 
	static void 		ClearProp2Timer(void);
	// PROP 3
	static int 			TBSProp3TimerID;
	static boolean 		Prop3TimerComplete;	
	static void 		StartProp3Timer(void);
	static void 		ClearProp3Timer(void);
	static void 		PulseDelayProp3(int);
	static uint8_t		currentProp3SoundNum;	// Which sound number is currently playing
	static boolean		HeadlightSound_Enabled;
	static boolean 		TurretSound_Enabled;
	
	static void 		StartSqueaksForReal(void);
	static void 		Squeak1_Activate(void);
	static void 		Squeak1_Pause(void);
	static void 		Squeak1(void);
	static void 		Squeak2_Activate(void);
	static void 		Squeak2_Pause(void);
	static void 		Squeak2(void);
	static void 		Squeak3_Activate(void);
	static void 		Squeak3_Pause(void);	
	static void 		Squeak3(void);
	static boolean 		Squeak1_Enabled;	// Enabled means, are we going to use this sqeak or not
	static boolean 		Squeak2_Enabled;	// The user has the option of disabling some or all of the sqeaks in settings
	static boolean 		Squeak3_Enabled;
	static boolean 		AllSqueaks_Active;
	static boolean 		Squeak1_Active;		// Active means, is this sqeak now sqeaking, or is it waiting to sqeak
	static boolean 		Squeak2_Active;		// Squeaks are only active while the tank is moving. 
	static boolean 		Squeak3_Active;
	static int 			Squeak1TimerID;
	static int 			Squeak2TimerID;
	static int 			Squeak3TimerID;
	static unsigned int SQUEAK1_MIN_mS;
	static unsigned int SQUEAK1_MAX_mS;
	static unsigned int SQUEAK2_MIN_mS;
	static unsigned int SQUEAK2_MAX_mS;
	static unsigned int SQUEAK3_MIN_mS;
	static unsigned int SQUEAK3_MAX_mS;
	
	
};


#endif

