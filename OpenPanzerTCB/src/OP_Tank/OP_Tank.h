/* OP_Tank.h        Open Panzer Tank - functions primarily related to battle
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

#ifndef OP_Tank_h
#define OP_Tank_h

#include <Arduino.h>
#include "../OP_Settings/OP_Settings.h"
#include "../OP_Driver/OP_Driver.h"
#include "../OP_IRLib/OP_IRLib.h"
#include "../OP_SimpleTimer/OP_SimpleTimer.h"
#include "../OP_Motors/OP_Motors.h"
#include "../OP_Smoker/OP_Smoker.h"
#include "../OP_Sound/OP_Sound.h"
#include "../LedHandler/LedHandler.h"
#include "OP_BattleTimes.h"

#define MECHRECOIL_TRIGGER_MODE     RISING  // Arduino defines these as:
#define AIRSOFT_TRIGGER_MODE        FALLING // #define CHANGE   1       (01)
                                            // #define FALLING  2       (10)
                                            // #define RISING   3       (11)

#define HIT_FILTER_mS               1100    // After taking a hit, we ignore any further hits for this length of time in milliseconds. This prevents a single
                                            // shot from being recorded as multiple hits. Should be at least 1000 mS because stock Tamiya fires the hit signal
                                            // repeatedly for 1 full second

#define MUZZLE_FLASH_TRIGGER_mS     50      // Trigger signal length for Asiatam/Taigen high-intensity flash unit, or for user-supplied LED

// NEITHER OF THESE ARE BEING USED FOR NOW. In testing I experimented with kicking off the mechanical recoil/airsoft motor and then waiting some period of time before
// enabling the trigger interrupt - this length of time was represented by the two defines below. The thought was that we could avoid transients that might occur 
// at the beginning of the motor's movement that might falsely trigger a stop. In practice this wasn't an issue, or more accurately, it was, but was dealt with in 
// other ways. I'm leaving them here in case they are required again for future testing. 
//#define AIRSOFT_DEBOUNCE_mS       100     // How long to wait after starting the airsoft motor before enabling the pin interrupt that will detect the falling edge when the airsoft fires. 
//#define MECH_RECOIL_DEBOUNCE_mS   100     // How long to wait after starting the mechanical recoil, before enabling the pin interrupt that will detect the recoil movement is over. 
                                            // Not presently used, but can be implemented if we have issues

// The IR receiver class needs to know which external interrupt to use. 
#define IR_RECEIVE_INT_NUM          0       // For the TCB board, this must be Arduino Interrupt 0 / Pin 2 (Atmega Interrupt 4 / Port E4) 

// These variables are used to create a flickering effect on the hit notification LEDs, similar to the way Tamiya does
#define MAX_BRIGHT                  255     // Maximum LED brightness during the flicker effect (should be 255)
#define MIN_BRIGHT                  10      // Minimum LED brightness during the flicker effect
#define BRIGHT_FADE_BREAK           150     // We will always ramp up to some value above this. Must be less than MAX_BRIGHT.
#define DIM_FADE_BREAK              130     // We will always ramp down to some value below this. Must by higher than MIN_BRIGHT.
#define MAX_FADE_STEP               50      // Maximum amount we will increase/decrease the brightness per step
#define MIN_FADE_STEP               10      // Minimum amount we will increase/decrease the brightness per step
#define FADE_UPDATE_mS              20      // How often we will increment/decrement the brightness
#define FLICKER_EFFECT_LENGTH_mS    3000    // How long to flicker the lights using the random fade up/down effect
// This defines how long a single machine-gun hit blink will last
#define MG_HIT_BLINK_TIME           100     // in milliSeconds

// There are four possible weight classes - the three standard Tamiya classes, 
// and one custom class defined by the user. 
typedef char WEIGHTCLASS; 
#define WC_CUSTOM       0
#define WC_LIGHT        1
#define WC_MEDIUM       2
#define WC_HEAVY        3
#define LAST_WEIGHT_CLASS   WC_HEAVY
const __FlashStringHelper *ptrWeightClassName(WEIGHTCLASS wClass); //Returns a character string that is name of tank class (see OP_Tank.cpp)


// See the Damage function in OP_Tank.cpp for definitions
typedef char DAMAGEPROFILES;
#define TAMIYA_DAMAGE       0       // Stock Tamiya damage profile
#define OPENPANZER_DAMAGE   1       // Open Panzer damage profile
#define NO_DAMAGE           2       // No damage
//#define ADDITIONAL (number)
#define LAST_DAMAGE_PROFILE NO_DAMAGE
const __FlashStringHelper *ptrDamageProfile(DAMAGEPROFILES dProfile); //Returns a character string that is name of the damage profile 


// All the types of IR receptions possible
typedef char HIT_TYPE;
#define HIT_TYPE_NONE       0       // No hit, signal couldn't be decoded, or it didn't apply to us
#define HIT_TYPE_CANNON     1
#define HIT_TYPE_MG         2
#define HIT_TYPE_REPAIR     3

// A collection of settings for the tank
typedef struct weightClassSettings{
    uint16_t reloadTime;        // How long (in mS) does it take to reload the cannon. Depends on weight class
    uint16_t recoveryTime;      // How long does recovery mode last (invulnerability time when tank is regenerating after being destroyed). Class-dependent. 
    uint8_t  maxHits;           // How many hits can the tank sustain before being destroyed. Depends on weight class
    uint8_t  maxMGHits;         // How many hits can the tank sustain from machine gun fire before being destroyed. Only applies to custom weight classes, 
                                // and only if Accept_MG_Damage = TRUE
};
typedef struct battle_settings{
    char     WeightClass;       // What is the tank's current weight class
    weightClassSettings ClassSettings;  // What are the settings for the weight class (max hits, reload time, recovery time)
    IRTYPES  IR_FireProtocol;   // Which battle protocol are we *sending* by cannon fire
    IRTEAMS  IR_Team;           // Does this tank belong to a team - only applies to a few protocols
    IRTYPES  IR_HitProtocol_2;  // We can accept hits from up to 2 protocols
    IRTYPES  IR_RepairProtocol; // Which repair protocol are we using
    IRTYPES  IR_MGProtocol;     // Which machine gun protocol are we using
    boolean  Use_MG_Protocol;   // If true, the Machine Gun IR code will be sent when firing the machine gun, otherwise, it will be skipped. 
    boolean  Accept_MG_Damage;  // If true, the vehicle will be susceptible to MG fire. 
    char     DamageProfile;     // Which Damage Profile are we using
    boolean  SendTankID;        // Do we include the Tank ID in the cannon IR transmission
    uint16_t TankID;            // What is this tank's ID number
};


class OP_Tank
{   public:
        OP_Tank(void);                              // Constructor
        static void     begin(battle_settings, bool, bool, bool, int, bool, bool, uint16_t, uint8_t, bool, Servo_RECOIL *, OP_Sound *, OP_SimpleTimer *);     
        // battle_settings, boolean mechanical barrel with cannon, boolean airsoft, boolean servo recoil with cannon, int recoil delay, bool flash with cannon, uint16_t aux flash time, bool aux light flash with cannon, uint8_t machine gun blink interval, boolean cannon reload blink, pointers to recoil servo, sound object, and sketch's SimpleTimer
        
        // Functions - Cannon Fire
        static void     Fire(void);                 // Fires the correct IR signal based on the IR protocol 
        static boolean  CannonReloaded(void);       // Has the cannon finished reloading?
        static boolean  AirsoftFired(void);         // Has the airsoft unit fired yet?
        static void     ClearAirsoft(void);         // Reset the airsoft fired flag
        
        // Direct control over portions of the typical cannon fire event
        static void     TriggerMechBarrel(void);    // Will trigger the mechanical recoil or airsoft unit (depending on which is selected), apart from any cannon fire. Used for manual control.
        static void     TriggerMuzzleFlash(void);
        static void     TriggerAuxFlash(void);
        static void     SetMechBarrelWithCannon(boolean);   // Allows the sketch to attach/detach the mechanical barrel (Airsoft or mechanical recoil) from the Cannon Fire event: True = attached, False = detached
        static boolean  isMechBarrelSetWithCannon(void);    // Returns the current setting
        
        // Functions - IR receiving (ie, getting hit!)
        static HIT_TYPE WasHit(void);               // Have we been hit
        static IRTYPES  LastHitProtocol(void);      // What were we hit with
        static IRTEAMS  LastHitTeam(void);          // Which team hit us (if applicable)
        static uint8_t  PctDamaged(void);           // Returns a number from 0-100 of the percent damage taken
        static uint8_t  PctHealthRemaining(void);   // Returns a number from 0-100 of the percent of health remaining
        static boolean  isRepairOngoing(void);      // Returns the status of a repair operation
        static void     Damage(Motor*, Motor*, Motor*, Motor*, OP_Smoker*, boolean, DRIVETYPE); // Pass 5 motor objects - left tread, right tread, turret elevation, turret azimuth, smoker, plus we pass the drive type and whether the smoker should be included or not
        static battle_settings BattleSettings;      // Battle settings struct
        static boolean  isInvulnerable;             // Is the tank presently invulnerable to incoming fire
        static boolean  isDestroyed;                // Is the tank destroyed
        static uint8_t  CannonHitsTaken;            // How many cannon hits have we sustained
        static uint8_t  MGHitsTaken;                // How many machine gun hits have we sustained
        static void     EnableIR(void);             // Enable IR functionality
        static void     DisableIR(void);            // Disable IR functionality
        static void     ToggleIR(void);             // Toggle IR functionality
        static boolean  IsIREnabled(void);          // Return status of IR
                
        // Mechanical recoil switch interrupt
        static void     RECOIL_ISR(void);           // The actual ISR will call this public member function, in order that it can access class variables
                                                    // We don't need this to be a public function but Arduino gives us an error if we put it in protected
        // Machine Gun
        static void     MachineGun(void);
        static void     MachineGun_Stop(void);
        
        // Misc
        static void     Update(void);               // For actions that need to be polled by the sketch for updating
        static boolean  isRepairTank(void);         // Returns status of fight/repair switch on the TCB.
        static void     StopRepair(void);           // The only time the sketch might need to call this is a failsafe, LVC or other dire situation occured, and we want to
                                                    // shut everything down. Some functions check for a repair before they actually do anything (ie EngineOn/EngineOff in the sketch).
                                                    // If we force a repair to be over first then those other functions will be sure to run as expected.
                                                    // This function is not used to actually stop a repair in normal practice, that is taken care of automatically. 
        
    private:
        // Setup
        static void     SetupTamiyaWeightClass(char);   // Initializes the weight class settings for a standard Tamiya weight class
    
        // IR objects
        static IRsend     IR_Tx;
        static IRrecvPCI *IR_Rx;    
        static IRdecode   IR_Decoder;

        // Mechanical Barrel functions (mechanical recoil/airsoft)
        static boolean  _MechBarrelWithCannon;
        static boolean  _Airsoft;   
        static boolean  _AirsoftFired;              // The Airsoft unit doesn't fire right away when powered, but after it has cocked. This variable gets set to true when the cocked limit switch is activated.
        static boolean  _RecoilServoWithCannon;
        static void     FireAirsoft(void);
        static void     MechanicalRecoil(void);
        static void     StartMechRecoilMotor(void);
        static void     StopMechRecoilMotor(void);
        static void     Enable_MechRecoilInterrupt(void);
        static void     Disable_MechRecoilInterrupt(void);
        static int      _RecoilDelay;
        static int      MechRecoilTimeoutTimerID;

        // Cannon Firing
        static void     Fire_Part2(void);
        static void     Cannon_Flash(void);
        static void     Cannon_Sound(void);
        static void     Cannon_SendIR(void);
        static void     Cannon_RepairLights(void);
        static void     Cannon_StartReload();
        static void     ReloadComplete(void);
        static boolean  CannonReloadComplete;
        static boolean  _CannonReloadBlink;
    
        // High Intensity Flash / Aux Light Flash
        static boolean  _HiFlashWithCannon;
        static void     ClearMuzzleFlash(void);
        static boolean  _AuxFlashWithCannon;
        static uint16_t _AuxFlashTime_mS;
        static void     ClearAuxFlash(void);

        // Incoming hits
        static void     EnableHitReception(void);
        static void     DisableHitReception(void);
        static void     ResetBattle(void);
        static void     ResetBattleImmediate(void);
        static IRTYPES  _lastHit;
        static IRTEAMS  _lastTeam;
        static void     EnableIR_Internal(void);
        static void     DisableIR_Internal(void);
        
        // Hit notification LEDs
        static boolean  HitLEDsOn;                  // True if currently ON or DIM, False if OFF
        static void     HitLEDs_On(void);
        static void     HitLEDs_Off(void);
        static void     HitLEDs_Toggle(void);
        static void     HitLEDs_SetDim(uint8_t level);
        static void     HitLEDs_CannonHit(void);    // Cannon-hit damage light effect
        static void     CannonHitLEDs_Update(void);
        static void     CannonHitLEDs_Stop(void);
        static void     HitLEDs_MGHit(void);        // Machine gun-hit damage light effect
        static void     HitLEDs_Destroyed(void);    // Destroyed light effect
        static void     Repair_BlinkHandler(void);  // Repair light effect
        static void     HitLEDs_Repair(void);       // Repair light effect
        static int16_t  FadeStep;
        static int16_t  FadeTarget;
        static int16_t  CurrentFadeLevel;
        static boolean  FadeOut;
        static int      FadeStep_TimerID;
        static int      HitLED_TimerID;
        static LedHandler AppleLEDs;                // Might have been useful to have the LedHandler class years ago... 

        // Machine Gun
        static void     MG_BlinkLight(void);        // Blink MG light
        static void     MG_LightOn(void);           // For direct control of the machine gun light
        static void     MG_LightOff(void);          //      
        static uint8_t  _MGLightBlink_mS;           // How quickly should the machine gun light blink in milliseconds
        static int      MG_BlinkTimerID;            // Timer ID for blinking the machine gun light
        static void     MG_Fire_IR(void);           // Sends the machine gun IR signal
        static int      MG_FireTimerID;             // Timer ID for firing the machine gun IR code


        // Damage/Repair
        static uint8_t  HitsTaken_Cannon;           // How many hits have we sustained
        static uint8_t  HitsTaken_MG;               // How many machine gun hits have we sustained      
        static float    DamagePct;                  // Damage the vehicle has sustained, in percent
        static float    DamagePctPerCannonHit;      // How many damage does a single cannon hit inflict
        static float    DamagePctPerMGHit;          // How many damage does a single round of machine gun fire inflict
        static boolean  RepairOngoing;              // Flag gets set if tank receives a repair code, and remains set until the operation is completed (see REPAIR_TIME_mS)
        static void     CancelRepair(void);         // If the model receive an enemy hit in the middle of a repair operation, we cancel the repair operation, do not increase the
                                                    // the health level, and apply damage as usual. 
        static void     RepairOver(void);           // This gets called if the repair is completed successfully. This is where the health is increased. 
        static int      RepairTimerID;              // Timer ID for the repair operation

        // Misc
        static boolean  IR_Enabled;                 // True if either cannon or MG enabled, false if both disabled
        static OP_SimpleTimer * TankTimer;
        static Servo_RECOIL * _RecoilServo;
        static OP_Sound   * _TankSound;
        
};


#endif //OP_Tank_h
