/* OP_Tank.cpp      Open Panzer Tank - functions primarily related to battle
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


#include "OP_Tank.h"

// Static variables must be declared outside the class
battle_settings OP_Tank::BattleSettings;
OP_SimpleTimer * OP_Tank::TankTimer;    
boolean         OP_Tank::IR_Enabled;
IRsend          OP_Tank::IR_Tx;
IRrecvPCI     * OP_Tank::IR_Rx;
IRdecode        OP_Tank::IR_Decoder;
boolean         OP_Tank::CannonReloadComplete;
boolean         OP_Tank::_MechBarrelWithCannon;
boolean         OP_Tank::_Airsoft;
boolean         OP_Tank::_AirsoftFired;
boolean         OP_Tank::_RecoilServoWithCannon;
int             OP_Tank::_RecoilDelay = 0;      
boolean         OP_Tank::_HiFlashWithCannon;
boolean         OP_Tank::_AuxFlashWithCannon;
uint16_t        OP_Tank::_AuxFlashTime_mS;
boolean         OP_Tank::_CannonReloadBlink;
int             OP_Tank::MechRecoilTimeoutTimerID;
boolean         OP_Tank::isInvulnerable;            
boolean         OP_Tank::isDestroyed;           
Servo_RECOIL  * OP_Tank::_RecoilServo;
OP_Sound      * OP_Tank::_TankSound;
uint8_t         OP_Tank::_MGLightBlink_mS;
int             OP_Tank::MG_BlinkTimerID;
int             OP_Tank::MG_FireTimerID;
uint8_t         OP_Tank::CannonHitsTaken;
uint8_t         OP_Tank::MGHitsTaken;
float           OP_Tank::DamagePct;
float           OP_Tank::DamagePctPerCannonHit;
float           OP_Tank::DamagePctPerMGHit;
boolean         OP_Tank::RepairOngoing;
int             OP_Tank::RepairTimerID;
IRTYPES         OP_Tank::_lastHit;
IRTEAMS         OP_Tank::_lastTeam;

// Hit notification LED effect variables
boolean         OP_Tank::HitLEDsOn;
int16_t         OP_Tank::FadeStep;
int16_t         OP_Tank::FadeTarget;
int16_t         OP_Tank::CurrentFadeLevel;  
boolean         OP_Tank::FadeOut;
int             OP_Tank::FadeStep_TimerID;
int             OP_Tank::HitLED_TimerID;
LedHandler      OP_Tank::AppleLEDs;



// Return a character string of the name of the weight class, used for printing
const __FlashStringHelper *ptrWeightClassName(WEIGHTCLASS wClass) {
  if(wClass>LAST_WEIGHT_CLASS) wClass=LAST_WEIGHT_CLASS+1;
  const __FlashStringHelper *Names[LAST_WEIGHT_CLASS+2]={F("Custom"), F("Light"), F("Medium"), F("Heavy"), F("Unknown")};
  return Names[wClass];
};


// Return a character string of the name of the damage profile, used for printing
const __FlashStringHelper *ptrDamageProfile(DAMAGEPROFILES dProfile) {
  if(dProfile>LAST_DAMAGE_PROFILE) dProfile = LAST_DAMAGE_PROFILE+1;
  const __FlashStringHelper *Names[LAST_DAMAGE_PROFILE+2]={F("Tamiya Spec"), F("Open Panzer"), F("None"), F("Unknown")};
  return Names[dProfile];
};



// Constructor
OP_Tank::OP_Tank() 
{
    // Initialize
    CannonReloadComplete = true;
    IR_Enabled = false;
    RepairOngoing = false;
    HitLEDsOn = false;
    DisableHitReception();                      // We start by ignoring hits
    IR_Rx = new IRrecvPCI(IR_RECEIVE_INT_NUM);  // Pass the external interrupt number to the IRrecvPCI class (Arduino Interrupt 0 on the TCB - see OP_Tank.h)
    //IR_Rx->setBlinkingOnReceive(true);        // For testing only. This will cause the headlights to flash on any IR reception, whether the IR can be decoded or not.
    _AirsoftFired = false;                      // Will get set when the airsoft limit switch it tripped
    
}


void OP_Tank::begin(battle_settings BS, boolean mbwc, boolean airsoft, boolean rswc, int mrd, boolean hfwc, boolean afwc, uint16_t aftms, uint8_t mgint, bool crb, Servo_RECOIL * sr, OP_Sound * os, OP_SimpleTimer * t)
{
    // Save settings
    
    // Sketch's SimpleTimer
    TankTimer = t;
    
    // Battle Settings
    BattleSettings = BS;
    
    // Do a quick sanity check on the IR_Team value
    if (BattleSettings.IR_Team != IR_TEAM_NONE)
    {   // If we aren't one of these protocols, change IR_TEAM back to NONE
        if (BattleSettings.IR_FireProtocol != IR_FOV)
            BattleSettings.IR_Team = IR_TEAM_NONE;
    }
            
    // Misc Settings
    _MechBarrelWithCannon = mbwc;   // If not enabled, it won't be triggered automatically with cannon fire, but it can still be triggered manually. 
    _Airsoft = airsoft;
    _RecoilServoWithCannon = rswc;
    _RecoilDelay = mrd; 
    _MGLightBlink_mS = mgint;
    _HiFlashWithCannon = hfwc;
    _AuxFlashWithCannon = afwc;
    _AuxFlashTime_mS = aftms;
    _CannonReloadBlink = crb;
    
    // Pointers to objects
    _RecoilServo = sr;
    _TankSound = os;

    // LedHandler object for the Apple LEDs
    AppleLEDs.begin(pin_HitNotifyLEDs, false);      // false means not inverted                        

    // Sanity check the MG IR setting
    if (BattleSettings.IR_MGProtocol == IR_DISABLED) { BattleSettings.Use_MG_Protocol = false; }

    // Only enable the IR receiver if the user has enabled IR functionality (either the cannon or machine gun)
    if (BattleSettings.IR_FireProtocol == IR_DISABLED  && BattleSettings.IR_MGProtocol == IR_DISABLED)
    {
        IR_Enabled = false;
    }
    else
    {   // Some sort of battling is enabled, so enable IR
        EnableIR_Internal();    // We use the "internal" version so we skip the notification LED routine, unnecessary here

        // If we aren't using a custom weight class, setup the specified Tamiya weight class
        if (BattleSettings.WeightClass != WC_CUSTOM)
        {
            SetupTamiyaWeightClass(BattleSettings.WeightClass);
        }

        // Setup damage settings
        if (BattleSettings.IR_FireProtocol != IR_DISABLED && BattleSettings.IR_MGProtocol != IR_DISABLED && BattleSettings.Accept_MG_Damage)
        {
            // The vehicle will take damage from both cannon fire and machine gun fire. 
            DamagePctPerCannonHit = 100.0 / (float)BattleSettings.ClassSettings.maxHits;
            DamagePctPerMGHit =     100.0 / (float)BattleSettings.ClassSettings.maxMGHits;
        }
        else if (BattleSettings.IR_FireProtocol != IR_DISABLED)
        {
            // The vehicle will take damage from cannon fire only
            DamagePctPerCannonHit = 100.0 / (float)BattleSettings.ClassSettings.maxHits;
            DamagePctPerMGHit = 0.0;
        }
        else
        {
            // The vhicle will only take damage from machine gun fire (unlikely you would want this scenario)
            DamagePctPerCannonHit = 0.0;
            DamagePctPerMGHit = 100.0 / (float)BattleSettings.ClassSettings.maxMGHits;      
        }
    }
    //Serial.print(F("Damage per Cannon Hit: "));   Serial.println(DamagePctPerCannonHit,2);
    //Serial.print(F("Damage per MG Hit: "));       Serial.println(DamagePctPerMGHit,2);

    // Clear hit count and enable IR reception immediately. 
    ResetBattleImmediate();


#ifdef TCB_DIY
    // DIY version of TCB, for use with off-the shelf Arduino MEGA
    // Same commentary as below, only we are using INT0 (PORT D PIN D0)
    DDRD  &= ~(1 << DDD0);      // Set PD0 to input
    PORTD |=  (1 << PD0);       // Input pullups on
    EIMSK &= ~(1 << INT0);      // Disable INT0 interrupt for now
    EICRA =  (EICRA & ~((1 << ISC00) | (1 << ISC01)));                  // Clear interrupt sense control to start
    if (_Airsoft)   { EICRA |= (AIRSOFT_TRIGGER_MODE    << ISC00); }    // Now set appropriately
    else            { EICRA |= (MECHRECOIL_TRIGGER_MODE << ISC00); }

#else   
    // Set up an external interrupt to read the mechanical airsoft or recoil trigger switch
    // SETUP EXTERNAL INTERRUPT PIN
    // ------------------------------------------------------------------------------------------------------------------------>>
    // We are using a pin that is not brought out on the Arduino Mega board, so we can NOT USE ARDUINO PIN FUNCTIONS! 
    // The pin is Atmega pin 8 which is Port E6. We want the pin to be an input with pullups enabled.
    // See this page for a useful tutorial on direct port manipulation: http://maxembedded.com/2011/06/port-operations-in-avr/
    DDRE  &= ~(1 << DDE6);      // Set PE6 to input. We "and-not" a 1 with bit DDE6 of the Port E Data Direction Register. This sets it to Zero, which means, input. 
    PORTE |=  (1 << PE6);       // Set input pullups on. We accomplish this by writing a 1 to the PE6 bit of the PORTE register. 
    

    // START WITH INTERRUPT DISABLED 
    // ------------------------------------------------------------------------------------------------------------------------>>
    // We don't want the interrupt enabled for now, so we clear the INT6 bit in the External Interrupt Mask Register (EIMSK). 
    EIMSK &= ~(1 << INT6);  // AND-NOT, instead of OR
    // We've also put this in its own routine called Disable_MechRecoilInterrupt() in this same file

    // To turn on the external interrupt, which we will do later in the mechanical recoil routines, we would set the bit instead of clear it.
    // Set bit 6 by ORing a 1 shifted over to the correct bit. 
    // EIMSK |= (1 << INT6);    
    // We've also put this in its own routine called Enable_MechRecoilInterrupt() in this same file


    // SETUP EXTERNAL INTERRUPT
    // ------------------------------------------------------------------------------------------------------------------------>>
    // EICRB (External Interrupt Control Register B)
    // Only present on chips with more than 4 external interrupts, this one controls external interrupts 4-7
    // There are two bits in the control register for each of the 4 interrupts. We are interested in external interrupt 6
    // so we are going to be setting Interrupt Sense Control bits 6-0 and 6-1 (ISC60, ISC61). These bits determine
    // the type of interrupt that is triggered. 
    // 
    // These are the possible settings: 
    // ISCn1    ISCn0   Type
    // 0        0       Interrupt on low level
    // 0        1       Any change generates interrupt
    // 1        0       Interrupt on falling edge
    // 1        1       Interrupt on rising edge

    // We tell it which setting we want through the mode variable passed to this function. mode can be:
    // CHANGE
    // RISING
    // FALLING
    // These are defined elsewhere as:
    // #define CHANGE   1       (01)
    // #define FALLING  2       (10)
    // #define RISING   3       (11)

    // First we clear the two ISC6 bits:
    EICRB =  (EICRB & ~((1 << ISC60) | (1 << ISC61)));
    // We are doing an AND NOT
    // First piece: (EICRB & ~((1 << ISC60) | (1 << ISC61)))
    // This sets the two ISC60:61 bits to 1. Then it NOTs these (~ means NOT) so now both bits are zero. Then it ANDS these two bits with EICRB so in other words, 
    // the ISC60 and ISC61 bits of EICRB are now certainly both zero. 
    // Basically, all this does is clear the bits to be doubly sure (we are doing an OR after all) 
    
    // Now we set them according to the selected mode setting 
    if (_Airsoft)   { EICRB |= (AIRSOFT_TRIGGER_MODE    << ISC60); }
    else            { EICRB |= (MECHRECOIL_TRIGGER_MODE << ISC60); }
    // Second piece: |= (mode << ISC60)
    // This now sets the two bits to whatever is specified in MECHRECOIL/AIRSOFT_TRIGGER_MODE through an OR statement   
#endif

    // For good measure, make sure interrupt is off
    Disable_MechRecoilInterrupt();  
    // And for good measure, make sure the motor is off
    StopMechRecoilMotor();
}

boolean OP_Tank::isRepairTank()
{   
    // Repair tank setting is set by the position of a physical switch on the TCB board. 
    // If LOW (held to ground), tank is fighter. If HIGH (through input pullup), tank is repair. 
    return digitalRead(pin_RepairTank);
}

void OP_Tank::SetMechBarrelWithCannon(boolean isSet)
{
    _MechBarrelWithCannon = isSet;
}

boolean OP_Tank::isMechBarrelSetWithCannon(void)
{
    return _MechBarrelWithCannon;
}

void OP_Tank::SetupTamiyaWeightClass(char weight_class)
{
    // This routine assigns settings according to the given Tamiya weight class
    // Settings are defined by the Tamiya standard, see the insert to Tamiya #53447 "Hop Up Options: Battle System"
    // There are three Tamiya classes: LIGHT, MEDIUM, and HEAVY.
    // Tamiya classes do not accept hits from machine gun fire.

    // Of course the user also has the option of creating a custom weight class, in which case this routine is skipped and the custom settings are used instead. 

    BattleSettings.WeightClass = weight_class;
    
    switch (BattleSettings.WeightClass)
    {
        //case CUSTOM:                                              // CUSTOM
        // The custom case is handled in the begin() function above
        
        case WC_LIGHT:                                              // LIGHT
            BattleSettings.ClassSettings.reloadTime = 3000;         // 3 second reload
            BattleSettings.ClassSettings.maxHits = 3;               // 3 hits before destruction
            BattleSettings.ClassSettings.recoveryTime = 15000;      // 15 second recovery (invulnerability) time
            break;

        case WC_HEAVY:                                              // HEAVY
            BattleSettings.ClassSettings.reloadTime = 9000;         // 9 second reload! 
            BattleSettings.ClassSettings.maxHits = 9;               // 9 hits before destruction
            BattleSettings.ClassSettings.recoveryTime = 10000;      // 10 second recovery (invulnerability) time
            break;

        case WC_MEDIUM:                                             // MEDIUM
        default:                                                    // Anything unknown, default to MEDIUM
            BattleSettings.ClassSettings.reloadTime = 5000;         // 5 second reload
            BattleSettings.ClassSettings.maxHits = 6;               // 6 hits before destruction
            BattleSettings.ClassSettings.recoveryTime = 12000;      // 12 second recovery (invulnerability) time
    }
}



//------------------------------------------------------------------------------------------------------------------------>>
// CANNON FIRE
//------------------------------------------------------------------------------------------------------------------------>>

void OP_Tank::Fire(void)
{
    // There is a lot going on when we fire the cannon, and the order of things can be different between airsoft and mechanical recoil, 
    // or whether the tank is a Repair tank or not. So we break it down into small parts and call them one after the other. 
    
    // But first of all, if the tank is the middle of being repaired (or in the middle of repairing another tank if this is a bergepanzer), 
    // then we do not fire anything at all - firing is disabled
    if (!RepairOngoing) // Don't do anything if repair is ongoing
    {
        // Is this a repair tank? 
        if (isRepairTank())
        {
            // This is a repair tank. We skip mechanical/servo recoil and airsoft. We do have a repair sound, and we also do a
            // special light effect on the hit notification LEDs (in the apple). And of course we also send the repair IR code. 
            RepairOngoing = true;   // Set the repair flag. It is the same flag if we are being repaired as it is if we are repairing someone else. 
            _TankSound->Repair();       // Start playing the repair sound
            Repair_BlinkHandler();      // Do the special repair light effect (start blinking slow and gradually increase faster and faster)
            // Start the repair timer. During this time we can not fire the repair signal again, nor can we move (the move disabling is handled by the sketch)
            TankTimer->setTimeout(REPAIR_TIME_mS, RepairOver);   // REPAIR_TIME_mS is set in OP_BattleTimes.h
            Cannon_StartReload();   // Start the reload timer - but we actually still won't be able to fire again until after the repair is over, which takes longer than reloading.
            Cannon_SendIR();        // Send the IR code
        }
        // Or is this a fighting tank? 
        else 
        {   
            if (_MechBarrelWithCannon)
            {
                if (_Airsoft)
                {
                    // In this case, we are driving an airsoft unit. The way these work is that they must first cock the firing mechanism, which
                    // takes a few seconds. We don't want any of the other effects (sound, flash, etc.) to occur until right when the airsoft fires.
                    // We'll know that happens from a switch input that will trigger an interrupt. 
                    // So all we do now is start the airsoft motor and enable the interrupt. 
                    FireAirsoft();          
                    // But actually, if the user doesn't have an airsoft unit plugged, but they selected Airsoft in OP Config, what happens is that
                    // flash and sound never occur, because the trigger switch never gets triggered. This can be confusing, but that is just the way it has to be. 
                    // The solution is to plug in an airsoft unit, or change the barrel type to mechanical recoil. 
                    // One thing we can do though, is start the IR. It is invisible and silent anyway so it doesn't matter when it starts. When you are just 
                    // doing some IR testing on the bench you don't want to worry about remembering what kind of barrel you are supposed to have selected. 
                    Cannon_SendIR();
                }
                else
                {
                    // In this case, we are using a mechanical recoil
                    
                    // First thing we do, is start the mechanical recoil action
                    MechanicalRecoil();                 
                    // We also start the servo recoil at the same time, if the user has specified it to occur with cannon fire
                    if (_RecoilServoWithCannon) { _RecoilServo->Recoil(); }
                    
                    // The mechanical units may need to run for a short length of time before the recoil actually occurs - this is especially true
                    // with the Taigen unit (not so much Tamiya). 
                    // To coordinate all the actions, the user can specify a small delay so the remaining effects occur right when the barrel is recoiling
                    if (_RecoilDelay > 0)
                    {
                        TankTimer->setTimeout(_RecoilDelay, Fire_Part2);
                    }
                    else
                    {
                        // If we don't want a delay, go straight to the rest of the actions
                        Fire_Part2();
                    }
                }
            }
            else
            {   // In this case, mechanical recoil/airsoft are disabled. So we don't need to worry about starting those motors or waiting for an interrupt. 
                // Instead we go directly ahead and start the servo recoil, flash, sound, IR, whatever. 
                if (_RecoilServoWithCannon) { _RecoilServo->Recoil(); }
                Fire_Part2();
            }
        }
    }
}
void OP_Tank::Fire_Part2(void)
{
    // If airsoft active the main sketch will be polling to see when this gets set, so it can activate track recoil
    if (_Airsoft && _MechBarrelWithCannon)  _AirsoftFired = true;
    
    // Play cannon fire sound
    Cannon_Sound();
    
    // This will flash the high-intensity flash unit and/or the aux light output, if either are set to do so. 
    Cannon_Flash();
    
    // If Airsoft we already sent IR before this. If mech-recoil, or if no mechanical barrel is specified, we send the IR now.
    if (!_Airsoft || !_MechBarrelWithCannon) { Cannon_SendIR(); }
    
    // If Mechanical Recoil (or no mechanical barrel) we already started the recoil servo. If Airsoft, we do it now.
    if (_MechBarrelWithCannon && _Airsoft && _RecoilServoWithCannon) { _RecoilServo->Recoil(); }
    
    // Now start the reload timer
    Cannon_StartReload();
}
boolean OP_Tank::AirsoftFired(void)
{
    return _AirsoftFired;         // Has the airsoft unit fired yet?
}
void OP_Tank::ClearAirsoft(void)
{
    _AirsoftFired = false;         // Reset the airsoft fired flag
}
void OP_Tank::Cannon_Flash(void)
{
    if (_HiFlashWithCannon)  { TriggerMuzzleFlash(); }              // High intensity flash unit
    if (_AuxFlashWithCannon) { TriggerAuxFlash();    }              // Aux light output
}
void OP_Tank::Cannon_Sound(void)
{
    _TankSound->Cannon();               // Play cannon fired sound
}
void OP_Tank::Cannon_SendIR(void)
{
    // The user can choose to skip IR completely, so check first if it's enabled
    if (BattleSettings.IR_FireProtocol != IR_DISABLED && IR_Enabled == true)
    {
        // We don't want to hit ourselves. So while we are sending, we disable reception
        DisableHitReception();      
        if (isRepairTank())
        {   // We are a repair tank, send the repair signal if one is selected
            if (BattleSettings.IR_RepairProtocol != IR_DISABLED) IR_Tx.send(BattleSettings.IR_RepairProtocol);
        }
        else
        {   // We are a battle tank, send the battle signal. 
            // But also check if we need to send a team-specific signal. 
            if (BattleSettings.IR_Team == IR_TEAM_NONE) { IR_Tx.send(BattleSettings.IR_FireProtocol); }
            else
            {   // This is a team-specific protocol
            
                // FOV TEAMS
                if (BattleSettings.IR_FireProtocol == IR_FOV)
                {
                    switch (BattleSettings.IR_Team)
                    {   // Team 1 is free-for-all and would have been sent above because we set it to IR_TEAM_NONE
                        case IR_TEAM_FOV_2: IR_Tx.send(IR_FOV, FOV_TEAM_2_VALUE); break;
                        case IR_TEAM_FOV_3: IR_Tx.send(IR_FOV, FOV_TEAM_3_VALUE); break;
                        case IR_TEAM_FOV_4: IR_Tx.send(IR_FOV, FOV_TEAM_4_VALUE); break;
                    }
                }
                // CHECK SOME OTHER PROTOCOLS WITH TEAMS HERE 
                //else if (BattleSettings.IR_FireProtocol == SOME_OTHER_PROTOCOL)
                //{
                //} 
            }
        }
        // Done sending, re-enable reception
        EnableHitReception();
    }
}
void OP_Tank::Cannon_StartReload(void)
{
    // Start the reload timer. Further canon fire will not be possible until the timer completes. 
    CannonReloadComplete = false;
    TankTimer->setTimeout(BattleSettings.ClassSettings.reloadTime, ReloadComplete);    // Will call function "ReloadComplete" after the correct amount of time has passed for this weight class. 
}
void OP_Tank::ReloadComplete(void)
{
    CannonReloadComplete = true;
    if (_CannonReloadBlink) AppleLEDs.DoubleTap();              // Give the user visual notification that the reload process is done by blinking the Apple LEDs
}
boolean OP_Tank::CannonReloaded(void)
{
    return CannonReloadComplete; 
}


//------------------------------------------------------------------------------------------------------------------------>>
// MACHINE GUN FIRE
//------------------------------------------------------------------------------------------------------------------------>>
void OP_Tank::MachineGun()
{   
    // Start the Machine Gun, unless we are already going for some reason
    if (!TankTimer->isEnabled(MG_BlinkTimerID))
    {
        MG_BlinkTimerID = TankTimer->setInterval(_MGLightBlink_mS, MG_BlinkLight);
        _TankSound->MachineGun();           // Start the machine gun sound 
        // Now also start a timer to send the IR code repeatedly, if specified
        if (BattleSettings.Use_MG_Protocol && !TankTimer->isEnabled(MG_FireTimerID))
        {
            // Start by firing once
            MG_Fire_IR();
            // Now set a timer to repeat the IR signal every MG_REPEAT_TIME milliseconds (defined in OP_IRLibMatch.h)
            MG_FireTimerID = TankTimer->setInterval(MG_REPEAT_TIME_mS, MG_Fire_IR);
        }
    }
}
void OP_Tank::MachineGun_Stop(void)
{
    if (TankTimer->isEnabled(MG_BlinkTimerID)) 
    {
        TankTimer->deleteTimer(MG_BlinkTimerID); 
        _TankSound->StopMachineGun();    // Stop machine gun sound
    }
    if (TankTimer->isEnabled(MG_FireTimerID))
    {
        TankTimer->deleteTimer(MG_FireTimerID);  // Stop sending the IR signal
    }
    MG_LightOff();   
}
void OP_Tank::MG_LightOn(void)
{
    // Machine Gun LED must be set directly. We "or" the port pin bit with 1 to set it to 1, this turns it on
    // These defines are set in OP_Settings.h
    MG_PORT |= (1 << MG_PORTPIN);
}
void OP_Tank::MG_LightOff(void)
{
    // Machine Gun LED must be set directly. We "and-not" the port pin bit with 1 to set it to 0, this turns it off
    // These defines are set in OP_Settings.h
    MG_PORT &= ~(1 << MG_PORTPIN);          
}
void OP_Tank::MG_BlinkLight(void)
{
    static boolean MG_LightState = true;
    MG_LightState ? MG_LightOff() : MG_LightOn();
    MG_LightState = !MG_LightState;
}
void OP_Tank::MG_Fire_IR(void)
{   // As long as the machine gun is firing, this routine will be called by the TankTimer every MG_REPEAT_TIME_mS milliseconds. 
    // Check if we even need to be sending this...
    if (BattleSettings.Use_MG_Protocol == true && IR_Enabled == true)
    {
        // To avoid hitting ourselves, we must disable hit reception while firing the IR code. But we only do it briefly, 
        // just while the code is being sent 
        DisableHitReception();      
        IR_Tx.send(BattleSettings.IR_MGProtocol);
        // Done sending, re-enable reception
        EnableHitReception();
    }
}


//------------------------------------------------------------------------------------------------------------------------>>
// MECHANICAL RECOIL / AIRSOFT MOTOR
//------------------------------------------------------------------------------------------------------------------------>>
void OP_Tank::TriggerMechBarrel(void)
{
    // This function can be triggered by the user and will activate the airsoft or mechanical recoil unit, independent of any cannon firing. 
    if (_Airsoft)
    {
        FireAirsoft();  
    }
    else
    {
        MechanicalRecoil();             
    }
}
void OP_Tank::FireAirsoft(void)
{
    // STEP 1: Start the airsoft motor
    // ------------------------------------------------------------------------------------------------------------------------>>
    // It's called "MechRecoilMotor" but it's the same plug we use for Airsoft motors
        StartMechRecoilMotor();

    // STEP 2: Enable interrupt on the trigger pin
    // ------------------------------------------------------------------------------------------------------------------------>>
    // As the motor starts moving the trigger switch will (should) already be open. It will remain open until the unit is cocked, at which point it will briefly close 
    // and tie the trigger input pin to ground. Due to the internal pullup on the input pin, this means when the switch closes we will detect
    // a falling edge. In the begin() function above we set the intterupt to occur on a falling edge for airsoft.  
    
    // To avoid any momentary bouncing as the motor first begins that might be falsely interpreted as a falling edge, we could enable the interrupt
    // after a slight delay. However in practice this wasn't necessary. 
        //TankTimer->setTimeout(AIRSOFT_DEBOUNCE_mS, Enable_MechRecoilInterrupt);
    // So instead I enable the interrupt right away. There will be transients, but the interrupt will know to discard them. 
        Enable_MechRecoilInterrupt();
    
    // STEP 3: Start a watchdog timer
    // ------------------------------------------------------------------------------------------------------------------------>>
    // In case something goes wrong, and we never get the signal from the trigger switch telling us to stop the airsoft motor, we want to shut it 
    // down after some length of time. The time is arbitrary... here it is 5 seconds
        MechRecoilTimeoutTimerID = TankTimer->setTimeout(5000, StopMechRecoilMotor);     
}
void OP_Tank::MechanicalRecoil(void)
{
    // We are using the Tamiya mechanical recoil system. Although physically it appears the same as the Asiatam mechanical recoil, electronically they are not the same!
    
    // The Asiatam/Taigen recoil has two wires to the recoil motor that are essentially always live. One of these wires goes through a normally closed contact switch to the motor.
    // When the recoil unit is at rest, the contact switch is depressed (open) so that no current flows to the motor. A third "trigger" wire is connected directly to the motor
    // bypassing the wire through the switch. To start the recoil, this trigger wire is energized and the motor start moving. The trigger wire is only hot for a very short period
    // of time, not long enough to run the motor for the whole recoil. But as soon as the motor start moving, the contact switch closes so the always live wire can power the motor.
    // The motor keeps running until it hits the limit switch, and the limit switch physically cuts the power to the motor, so it stops. 
    
    // The Tamiya would seem to work in the same manner, and you can hook it up the same way and provide it the same electrical signals. It might work depending on resistance
    // and the rigidity of the linkage, but many times it will keep cycling over and over. This is because the Tamiya unit has enough momentum (in stock form it is not fighting
    // against any spring the way the Asiatam/Taigen unit does) that it will keep going even after it hit the switch to turn itself off. It will keep going just long enough for the
    // switch to close again, thus re-powering the unit. Round and round it will go and never stop. 
    
    // If you examine the electrical signals that Tamiya uses, the limit switch is actually used not as the physical cut-off for the motor, but as an input to the controller, 
    // and then the controller turns off the motor electrically. When the recoil unit is running, the normally-closed trigger switch is held to ground, but as soon as the recoil 
    // action is complete and the switch is triggered, the contacts open and the microcontroller actually sees a positive voltage (a rising edge). As soon as the controller sees 
    // this change, it shuts off the recoil motor. 
    
    // We use this same Tamiya approach here. The wiring is the same either way so it will also still work with the Asiatam/Taigen units. Althought slightly more complex, 
    // it allows compatibility with both brands (other than connector differences) and we only need three wires to the recoil unit rather than the 4 wires most implementations use. 
    
    // STEP 1: Start the recoil motor
    // ------------------------------------------------------------------------------------------------------------------------>>
    // We are not powering the motor through the switch. We are powering it directly. We can turn the recoil motor on or off from the control board at any time
    // regardless of what the switch is doing. 
        StartMechRecoilMotor();

    // STEP 2: Enable interrupt on the trigger pin
    // ------------------------------------------------------------------------------------------------------------------------>>
    // When the motor starts moving, the trigger switch will close, sending a ground signal to the controller. When it goes back high, 
    // we know we're at the end and we can stop the recoil motor. We have the interrupt set to only occur on rising edges, but to avoid
    // any momentary bouncing as the motor first begins that might be falsely interpreted as a rising edge, we only enable the interrupt
    // after a slight delay
        // TankTimer->setTimeout(MECH_RECOIL_DEBOUNCE_mS, Enable_MechRecoilInterrupt);
        Enable_MechRecoilInterrupt();
        // EDIT ABOVE: The only problem with this is that sending the IR signal can take some time and while it's going the TankTimer is not being updated.
        // So we need to make sure this really does get enabled before the recoil is over. So we enable it right away and then take care of bouncy-signals
        // in the ISR instead. 
        
    
    // STEP 3: Start a watchdog timer
    // ------------------------------------------------------------------------------------------------------------------------>>
    // In case something goes wrong, and we never get the signal from the trigger switch telling us to stop the recoil motor, we want to shut it 
    // down after some length of time. Tamiya will run it for 10 seconds before shutting down, I think 5 seconds is more than enough. 
        MechRecoilTimeoutTimerID = TankTimer->setTimeout(5000, StopMechRecoilMotor);

}
void OP_Tank::StartMechRecoilMotor(void)
{   
    // P-channel MOSFET, logic low is off, high is on
    digitalWrite(pin_MechRecoilMotor, HIGH);
}
void OP_Tank::StopMechRecoilMotor(void)
{
    digitalWrite(pin_MechRecoilMotor, LOW);
    if (TankTimer->isEnabled(MechRecoilTimeoutTimerID)) TankTimer->deleteTimer(MechRecoilTimeoutTimerID);
}
void OP_Tank::Enable_MechRecoilInterrupt(void)
{
#ifdef TCB_DIY
    // Enable external interrupt 0
    EIMSK |= (1 << INT0);   
#else
    // Enable external interrupt 6
    EIMSK |= (1 << INT6);   
#endif
}
void OP_Tank::Disable_MechRecoilInterrupt(void)
{
#ifdef TCB_DIY
    // Disable external interrupt 0
    EIMSK &= ~(1 << INT0);  
#else
    // Disable external interrupt 6
    EIMSK &= ~(1 << INT6);  
#endif
}

#ifdef TCB_DIY
// This is Atmega external Interrupt 0 on Atmega2560 pin 43 (TQFP), Arduino Pin 21 (D0)
ISR(INT0_vect){
    OP_Tank::RECOIL_ISR();
}
#else
// This is Atmega external Interrupt 6 on Atmega2560 pin 8 (TQFP). Arduino wouldn't call this anything because this pin is not brought out on the Arduino boards. 
ISR(INT6_vect){
    OP_Tank::RECOIL_ISR();
}
#endif

void OP_Tank::RECOIL_ISR()
{
    if (_Airsoft)
    {
        // The airsoft switch is normally-open (N.O.). While open the input pin is held high through an internal pullup. When the BB has finally been fired, the 
        // switch closes and ties the input pin to ground, giving us a falling edge which causes this ISR to be called. 

        // As with the mechanical recoil unit, this is our signal to cut power to the airsoft motor (it's done firing). But unlike the mechanical recoil unit, 
        // this is also when all the other effects *start* - flash, sound, and servo recoil. 

#ifdef TCB_DIY       
        // DIY version - same notes as below, only using pin D0 (Arduino 21) instead
        if ((PIND & (1<<PD0)) == 0) // pin low
#else
        // Regular TCB version - uses pin E6 (not available on Arduino)
        if ((PINE & (1<<PE6)) == 0) // pin low
#endif
        {
            // The pin is still low. If it were high, it means some low signal triggered the interrupt (that's why we're here) but it was so short
            // that now we're checking, it's gone. In that case it's a transient and we want to ignore those. But if the pin is still low now that we're
            // checking, we assume it really was the switch. Note: the switch is only closed momentarily. But still this brief moment seems like a long
            // time to the microcontroller. 
            Disable_MechRecoilInterrupt();  // These are called "MechRecoil" but it's the same plug for the airsoft motor
            TankTimer->setTimeout(300, StopMechRecoilMotor);    // Keep it running just a little bit longer to be sure we have passed the firing point
            
            // If this is part of a cannon-fire event, kick off the remaining actions
            if (_MechBarrelWithCannon)
            {
                Fire_Part2();           // This takes care of sound, flash, and recoil servo; IR we already did in the case of Airsoft
            }
            
            // Another Note: in the case of airsoft, the BB actually seems to fire just before the switch is triggered. That means our flash, sound, etc...
            // will appear to come just a moment after the gun fires. There's no real way to adjust for this, because the interrupt is the first notice
            // we get, and we can't guess an amount of time earlier than it (or we could, but we'd be even less accurate). So this is as good as it gets.
        }
    }
    else
    {
        // This is where we detect the recoil motor has reached the end. As it hits the switch, the switch opens, and the signal which had been held
        // to ground while closed, now goes high (if wired correctly). We've set this interrupt to only occur on a rising edge, so if we're here, 
        // we know the wire was previously held to ground, and is now High. 
        
        // All we need to do is stop the motor, and also disable the interrupt. But wait! What about switch bounce, or noisy signals? Good question.
        // In fact the signal *is* noisy because it picks up crap from the motor, and some false positives can get through. 
        // What we do is check the status of the pin. If it is still High, we consider it good. If it is back to ground by the time we read it, then 
        // the signal was just a transient and we ignore it. 
        
        // As it turns out, transients are very short, so even immediately as we enter this ISR and do a check, it is probably already gone. 
        // In testing a false positive has never gotten through in countless cycles of both the Tamiya and Asiatam recoil units. But if we 
        // need a longer time here, put in a small delay, like delay(1);
        //delay(1);
#ifdef TCB_DIY     
        // DIY version - same notes as below, only using pin D0 (Arduino 21) instead  
        if (PIND & (1<<PD0))    // pin is high
#else    
        // Regular TCB version - uses pin E6 (not available on Arduino)
        if (PINE & (1<<PE6))    // pin is high
#endif
        {
            // If the bit is still high now we're reading it, we really must have hit the end
            Disable_MechRecoilInterrupt();
            StopMechRecoilMotor();
        }
    }
}



//------------------------------------------------------------------------------------------------------------------------>>
// MUZZLE FLASH
//------------------------------------------------------------------------------------------------------------------------>>
void OP_Tank::TriggerMuzzleFlash(void)
{
    // This one is a PNP transistor, so logic HIGH = OFF, LOW = ON
    digitalWrite(pin_MuzzleFlash, LOW);
    TankTimer->setTimeout(MUZZLE_FLASH_TRIGGER_mS, ClearMuzzleFlash);
}
void OP_Tank::ClearMuzzleFlash(void)
{
    // Muzzle flash is over. Remember, for PNP HIGH = off
    digitalWrite(pin_MuzzleFlash, HIGH);
}
void OP_Tank::TriggerAuxFlash(void)
{
    // This is an N-Channel MOSFET, so logic HIGH = ON, LOW = OFF
    digitalWrite(pin_AuxOutput, HIGH);    
    TankTimer->setTimeout(_AuxFlashTime_mS, ClearAuxFlash);
}
void OP_Tank::ClearAuxFlash(void)
{
    // Aux flash is over. Turn off light.
    digitalWrite(pin_AuxOutput, LOW);   
}



//------------------------------------------------------------------------------------------------------------------------>>
// RECEIVING HITS AND TAKING DAMAGE
//------------------------------------------------------------------------------------------------------------------------>>

void OP_Tank::EnableIR(void)
{
    // But only enable if we have valid protocols selected
    if (BattleSettings.IR_FireProtocol == IR_DISABLED  && BattleSettings.IR_MGProtocol == IR_DISABLED)
    {
        DisableIR();
    }
    else
    {   
        EnableIR_Internal();
        AppleLEDs.Fade(FADE_IN, 100, true);    // true meaning add a double-blip after the fade-in completes
    }
}

void OP_Tank::EnableIR_Internal(void)
{   // We split these out in the case we want to enable without the LED notification
    IR_Rx->enableIRIn();
    IR_Enabled = true;
}

void OP_Tank::DisableIR(void)
{
    DisableIR_Internal();
    AppleLEDs.Fade(FADE_OUT, 200, true);       // true meaning add a double-blip before the fade-out starts
}

void OP_Tank::DisableIR_Internal(void)
{   // We split these out in the case we want to disable without the LED notification
    IR_Rx->disableIRIn();
    IR_Enabled = false;
}

void OP_Tank::ToggleIR(void)
{
    IR_Enabled ? DisableIR() : EnableIR();
}

boolean OP_Tank::IsIREnabled(void)
{
    return IR_Enabled;
}

// Returns the HIT_TYPE if the tank was hit
HIT_TYPE OP_Tank::WasHit(void)
{
// Initialize to false
boolean hit = false; 
boolean TwoShotHit = false;

    if (isInvulnerable || isDestroyed || IR_Enabled == false)
    {
        // The tank can't be hit if it is invulnerable, so don't even bother checking.
        // Same goes if the tank is already destroyed. 
        // Also if IR is disabled none of this matters. 
        return HIT_TYPE_NONE;
    }
    else
    {
        if (IR_Rx->GetResults(&IR_Decoder)) // If true, some IR signal was received 
        {   // Now we have to decode the signal, and see if it applies to us
            
            // For testing
                //IR_Decoder.decode();
                //Serial.print(F("Decoded: ")); Serial.print(ptrIRName(IR_Decoder.decode_type)); Serial.print(F(" Value: ")); Serial.println(IR_Decoder.value);
                //IR_Decoder.DumpResults();
    
            // There are multiple types of IR signals that can be received: Cannon, Machine Gun, Repair
            // Clear these to start, they will get set as we proceed to whatever protocol/team hit us
            _lastHit = IR_UNKNOWN;
            _lastTeam = IR_TEAM_NONE;

            // CANNON 
            // The user can specify up to 2 protocols. Here we check the first one - which is the same protocol they use to fire with
            if (BattleSettings.IR_FireProtocol != IR_DISABLED)
            {
                // Were we hit with the primary IR protocol? 
                hit = IR_Decoder.decode(BattleSettings.IR_FireProtocol);
                // If so, save it to the _lastHit variable
                if (hit) _lastHit = BattleSettings.IR_FireProtocol;
                
                // If the FireProtocol is set to Tamiya 2-Shot and we were hit, we want to save that because damage will be different 
                if (hit && BattleSettings.IR_FireProtocol == IR_TAMIYA_2SHOT) { TwoShotHit = true; }
                // But even if we weren't hit, because they set it to the 2-shot protocol, we automatically check for regular 1/16 Tamiya code as well
                if (!hit && BattleSettings.IR_FireProtocol == IR_TAMIYA_2SHOT) { hit = IR_Decoder.decode(IR_TAMIYA); if (hit) { _lastHit = IR_TAMIYA; } }
                // Likewise, if the FireProtocol is set to Tamiya, automatically check for Tamiya 2-Shot kill code as well
                if (!hit && BattleSettings.IR_FireProtocol == IR_TAMIYA) { hit = TwoShotHit = IR_Decoder.decode(IR_TAMIYA_2SHOT); if (hit) { _lastHit = IR_TAMIYA_2SHOT; } }
            }
            // Now we also check the second one, but only if the first one didn't already return a hit, and if the second one is not set to null or the same as the first
            if (!hit && BattleSettings.IR_HitProtocol_2 != IR_DISABLED && BattleSettings.IR_HitProtocol_2 != BattleSettings.IR_FireProtocol)
            {
                // Were we hit with the secondary IR protocol?
                hit = IR_Decoder.decode(BattleSettings.IR_HitProtocol_2);
                // If so, save it to the _lastHit variable
                if (hit) _lastHit = BattleSettings.IR_HitProtocol_2;
                
                // Same deal here, if the user wants to check one Tamiya code, we automatically also check the other. 
                // If the HitProtocol_2 is set to Tamiya 2-Shot and we were hit, we want to save that because damage will be different 
                if (hit && BattleSettings.IR_HitProtocol_2 == IR_TAMIYA_2SHOT) { TwoShotHit = true; }
                // But even if we weren't hit, because they set it to the 2-shot protocol, we automatically check for regular 1/16 Tamiya code as well
                if (!hit && BattleSettings.IR_HitProtocol_2 == IR_TAMIYA_2SHOT) { hit = IR_Decoder.decode(IR_TAMIYA); if (hit) { _lastHit = IR_TAMIYA; } }
                // Likewise, if the HitProtocol_2 is set to Tamiya, automatically check for Tamiya 2-Shot kill code as well
                if (!hit && BattleSettings.IR_HitProtocol_2 == IR_TAMIYA) { hit = TwoShotHit = IR_Decoder.decode(IR_TAMIYA_2SHOT); if (hit) { _lastHit = IR_TAMIYA_2SHOT; } }
            }
            
            // If hit is true, we were hit with cannon fire. But some protocols implement teams and if we were hit with one of those we want to record
            // which team hit us. If we are on the same team ourselves, we will ignore the hit and take no damage. 
            if (hit)
            {   
                // FOV TEAMS
                if (_lastHit == IR_FOV)
                {   
                    // Save the team to _lastTeam variable. If the team that hit us is the same team that we're on, set hit = false
                    switch (IR_Decoder.value) 
                    {
                        case FOV_TEAM_1_VALUE: _lastTeam = IR_TEAM_NONE; break; // FOV Team 1 is considered "No team" and all teams take hits from it
                        case FOV_TEAM_2_VALUE: _lastTeam = IR_TEAM_FOV_2; if (BattleSettings.IR_Team == IR_TEAM_FOV_2) { hit = false; } break;
                        case FOV_TEAM_3_VALUE: _lastTeam = IR_TEAM_FOV_3; if (BattleSettings.IR_Team == IR_TEAM_FOV_3) { hit = false; } break;
                        case FOV_TEAM_4_VALUE: _lastTeam = IR_TEAM_FOV_4; if (BattleSettings.IR_Team == IR_TEAM_FOV_4) { hit = false; } break;
                    }
                }
            }

            
            // Ok, now if hit is still true we really were hit by cannon fire
            if (hit)
            {   
                DisableHitReception();      // Temporary invulnerability after being hit (mostly so we only proces the hit a single time)
                
                // What about if we were in the middle of being repaired? We need to cancel the repair and turn off the repair lights
                // to make way for the cannon hit lights. 
                if (RepairOngoing) CancelRepair(); 
                
                CannonHitsTaken += 1;       // Increment number of cannon hits taken
                
                // Increment our overall damage percent 
                if (TwoShotHit)
                {
                    DamagePct += 50;    // Two-shot hits increase damage by 50 percent each time
                }
                else
                {
                    DamagePct += DamagePctPerCannonHit; // Regular hits increase by the amount-per-cannon-hit
                }
                
                if (DamagePct >= 100.0)
                {
                    // We've been destroyed. 
                    _TankSound->Destroyed();
                                        
                    // Don't let damage go above 100%
                    DamagePct = 100.0;
                                        
                    // After destruction, the tank becomes inoperative for some period of time (15 seconds is the Tamiya spec - NOT the same as recovery/invulnerability time!)
                    // After that time it will automatically recover itself. During invulnerability time, the tank can fire but is impervious to enemy fire. 
                    // Invulnerabilty time is dependent on the weight class. 
                    isDestroyed = true;
                    TankTimer->setTimeout(DESTROYED_INOPERATIVE_TIME_mS, ResetBattle);   // DESTROYED_INOPERATIVE_TIME_mS is defined in OP_BattleTimes.h
                    // Start the destroyed light effect
                    HitLEDs_CannonHit();    // After the cannon hit effect, because isDestroyed is true, the subsequent HitLEDs_Destroyed effect will start automatically
                }
                else
                {
                    // Not destroyed, just damaged. Play the cannon hit sound. 
                    _TankSound->CannonHit();
                    // Flash the hit notification LEDs
                    HitLEDs_CannonHit();
                    // Start a brief invulnerability timer. Each IR signal is sent multiple times, but we only want to count 
                    // one hit per shot. For the next second after being hit, we ignore further hits
                    TankTimer->setTimeout(HIT_FILTER_mS, EnableHitReception);
                }
                return HIT_TYPE_CANNON; // Return cannon hit type 
            }
            // If that didn't match, we may still have been hit, but by machine gun fire.
            // Check that, but only if the user has specified MG damange and an MG protocol
            else if (BattleSettings.Accept_MG_Damage && BattleSettings.IR_MGProtocol != IR_DISABLED && IR_Decoder.decode(BattleSettings.IR_MGProtocol))
            {
                // We were hit with a machine gun
                
                // Save the protocol to the _lastHit variable
                _lastHit = BattleSettings.IR_MGProtocol;

                // What about if we were in the middle of being repaired? We need to cancel the repair and turn off the repair lights
                // to make way for the MG hit lights. 
                if (RepairOngoing) CancelRepair(); 
                
                // Unlike cannon fire, we don't disable IR reception, because we allow multiple MG hits to occur in quick succession
                MGHitsTaken += 1;               // Increment number of machine gun hits taken
                DamagePct += DamagePctPerMGHit; // Increment our overall damage percent
                
                if (DamagePct >= 100.0)
                {
                    // We've been destroyed. 
                    _TankSound->Destroyed();
                    
                    // Don't let damage go above 100%
                    DamagePct = 100.0;
                    
                    // After destruction, the tank becomes inoperative for some period of time (15 seconds is the Tamiya spec - NOT the same as recovery/invulnerability time!)
                    // After that time it will automatically recover itself. During invulnerability time, the tank can fire but is impervious to enemy fire. 
                    // Invulnerability time is dependent on the weight class. 
                    isDestroyed = true;
                    TankTimer->setTimeout(DESTROYED_INOPERATIVE_TIME_mS, ResetBattle);   // DESTROYED_INOPERATIVE_TIME_mS is defined in OP_BattleTimes.h
                    // Start the destroyed light effect directly
                    HitLEDs_Destroyed();    
                }
                else
                {
                    // Not destroyed, just damaged. Play the machine gun hit sound. 
                    _TankSound->MGHit();
                    // Flash the hit notification LEDs, but this is a different pattern from when being hit by a cannon
                    HitLEDs_MGHit();
                    // Reenable hit reception immediately, we can take MG hits as fast as someone can send them. Even though we didn't call DisableHitReception()
                    // we still have to call EnableHitReception because the IR_Receiver automatically stops after being decoded. 
                    EnableHitReception();
                }
                return HIT_TYPE_MG; // Return MG hit type
            }
            // If that didn't match, we may still have been hit, but by a repair tank. 
            // Check but only if we haven't sustained any damage yet (otherwise there is no repair needed)
            // And also ignore it if we are already in the process of being repaired
            else if (DamagePct > 0.0 && !RepairOngoing && IR_Decoder.decode(BattleSettings.IR_RepairProtocol))
            {
                _lastHit = BattleSettings.IR_RepairProtocol;// Save the protocol to the _lastHit variable
                RepairOngoing = true;       // Set the repair flag
                _TankSound->Repair();       // Play the repair sound
                Repair_BlinkHandler();      // Do the special repair light effect (start blinking slow and gradually increase faster and faster)
                // Reenable hit reception immediately. The point is that while being repaired, the tank is vulnerable. 
                EnableHitReception();
                // Start the repair timer. During this time we can not be repaired again, nor can we move (the move disabling is handled by the sketch)
                RepairTimerID = TankTimer->setTimeout(REPAIR_TIME_mS, RepairOver);   // REPAIR_TIME_mS is set in OP_BattleTimes.h
                // Return "hit" type
                return HIT_TYPE_REPAIR;
                // Note - we don't decrease the damage just yet. That only happens at the end of the repair operation, if the vehicle makes it that long
                // without being hit by the enemy. 
            }
            else
            {
                // We weren't hit. Re-enable reception
                EnableHitReception();
            }
        }
    }
    return HIT_TYPE_NONE;   // If we make it to here, we weren't hit
}

IRTYPES OP_Tank::LastHitProtocol(void)
{
    return _lastHit;    // What protocol was the last successful hit
}

IRTEAMS OP_Tank::LastHitTeam(void)
{
    return _lastTeam;   // Which team last hit us (if applicable)
}

void OP_Tank::CancelRepair(void)
{
    // We cancel a repair if we were hit by enemy fire in the middle of one. 
    // Health is not increased. 
    // We can just use our StopRepair function here. 
    StopRepair();
}

void OP_Tank::RepairOver(void)
{
    // Repair is over, we successfully made it through the whole 15 seconds. 
    RepairOngoing = false;

    // Now we do the opposite of taking damage.
    DamagePct -= DamagePctPerCannonHit;     // Subtract a cannon hit
    if (DamagePct < 0.0) DamagePct = 0.0;   // But don't go below zero      

    // Call the repair blink hander, it will turn off the lights. 
    Repair_BlinkHandler();
}

void OP_Tank::StopRepair(void)
{
    // Stop this timer from running
    if (TankTimer->isEnabled(RepairTimerID)) { TankTimer->deleteTimer(RepairTimerID); }
    
    if (RepairOngoing) 
    { 
        RepairOngoing = false;
        
        // Call the repair blink hander, it will turn off the lights. 
        Repair_BlinkHandler();
    }
}

boolean OP_Tank::isRepairOngoing()
{
    return RepairOngoing;
}

uint8_t OP_Tank::PctDamaged(void)
{
    return round(DamagePct);
}

uint8_t OP_Tank::PctHealthRemaining(void)
{
    return (100 - constrain(round(DamagePct),0,100));
}

void OP_Tank::DisableHitReception(void)
{
    isInvulnerable = true;      // The tank will now ignore hits
}
    
void OP_Tank::EnableHitReception(void)
{
    if (IR_Enabled == true)             // Only re-enable if the user has allowed it
    {
        if (IR_Tx.isSendingDone()) 
        {
            IR_Decoder.Reset();         // Clear the decoder of anything that may have come in
            IR_Rx->resume();            // Resume IR reception
            isInvulnerable = false;     // We are now vulnerable to hits
        }
        else
        {
            // We want to enable reception, but we are also still in the middle of sending a signal out. 
            // Start a repeating timer that will keep checking back, and auto enable hit reception
            // when the transmission is done. 
            TankTimer->setTimeout(5, EnableHitReception);
        }
    }
}


// Cut speed to motors (ie, "damage"). 
void OP_Tank::Damage(Motor* Right_orRear, Motor* Left_orSteering, Motor* elevation, Motor* azimuth, OP_Smoker* smoke, boolean includeSmoker, DRIVETYPE driveType)
{
int cut_Pct;

    // If driveType = DT_CAR or DT_DKLM, then "Right_orRear" will be the rear axle/propulsion motor, and "Left_OrSteering" will be the steering servo/motor.
    // But if driveType = DT_TANK or DT_HALFTRACK, then "Right_orRear" will be the right tread, and "Left_OrSteering" will be the left tread. 

    // We can define multiple damage characteristics, aka "damage profiles" 
    switch (BattleSettings.DamageProfile)
    {
        // Open Panzer damage profile
        case OPENPANZER_DAMAGE:
            // This has not yet been defined. But note we have available in this function the speed of both treads as well as turret rotation and elevation and the smoker.
            // (Note the smoker includes another variable called "includeSmoker" - if that is false we should NOT modify the smoker motor because it is being used for 
            //  other purposes by the user)
            // In the case of a car, we also have the steering servo object.  We can also access in addition to the total DamagePct, the number of cannon vs MG hits. 
            // So there are lots of possibilities for creative damage effects. We could setup a tier system like Tamiya, but make it more gradual 
            // (rather than just 50%, 75%, Destroyed). We could also randomly damage, or apply damage to a single tread, or the turret, or whatever. 
            // 
            // But this feature will be added at a later time. For now we just fall through to TAMIYA_DAMAGE...
            
        case TAMIYA_DAMAGE:
            // For reference, see the package insert to Tamiya #53447 "Hop Up Options: Battle System".
            // When a Tamiya tank is hit, the drive motor speed is reduced. Tamiya follows a consistent formula based on the number of hits
            // taken compared to the max number of hits the given weight class can take before being destroyed. The formula is as follows: 
            // 1. Subtract 1 from the number of max hits (because the last hit destroys the tank, it doesn't damage it)
            // 2. Divide the remaining number of hits by 2. If an odd number, round up to the nearest integer. We will call this Halfway.
            // 3. If the number of hits taken so far is less than or equal to Halfway, reduce the speed of the drive motors to 50%. 
            // 4. If the number of hits taken is greater than Halfway, reduce the speed of the drive motors to 25%. 
            // 5. If the number of hits taken equals the max number of hits allowed, the tank is destroyed. 
            // Because this formula only needs to know the max number of hits allowed, we can easily apply it to any custom weight class the
            // user may create, as well as the standard Tamiya classes: 
            /*
            MaxHits = BattleSettings.ClassSettings.maxHits;     // The max number of hits for this weight class
            Halfway = round(float(MaxHits-1)/2.0);              // The Tamiya formula to calculate Halfway
            if      (hitsTaken <= Halfway) { cut_Pct = 50;  }   // Reduce speed to 50%
            else if (hitsTaken <  MaxHits) { cut_Pct = 75;  }   // Reduce speed to 25% (cut by 75%)
            else                           { cut_Pct = 100; }   // Destroyed (cut by 100%)
            */
            
            // EDIT: Well, that's the way Tamiya does it, but if you want to include machine gun damage, we need a slightly different approach. 
            // This should give us the same result as the Tamiya approach, but also allow damage due to machine gun fire. 
            if      (DamagePct <= 0.0)                          cut_Pct = 0;
            else if (DamagePct >  0.0 && DamagePct <= 50.0 )    cut_Pct = 50;
            else if (DamagePct > 50.0 && DamagePct <  100.0)    cut_Pct = 75;
            else                                                cut_Pct = 100;
                    
            // Serial.print(F("Speed cut to "));
            // Serial.print(100-cut_Pct);
            // Serial.println(F("%"));
            if (driveType == DT_TANK || driveType == DT_HALFTRACK)
            {   // Two independent treads, we cut/restore them both
                if (cut_Pct > 0)
                {
                    Right_orRear->cut_SpeedPct(cut_Pct);
                    Left_orSteering->cut_SpeedPct(cut_Pct);
                }
                else    
                {   // If the tank is repaired enough, it is possible that it returns to full health, in which case, cut_Pct will equal 0
                    // In that case, we just call the restore_Speed function
                    Right_orRear->restore_Speed();
                    Left_orSteering->restore_Speed();
                }
            }
            else if (driveType == DT_CAR || driveType == DT_DKLM || driveType == DT_DMD)
            {   // A single rear axle or drive motor
                if (cut_Pct > 0) Right_orRear->cut_SpeedPct(cut_Pct);
                else             Right_orRear->restore_Speed(); 
                // In this case, we leave the steering servo alone because Tamiya has no damage setting specified for steering servos. 
                // We also leave the steering motor alone in the case of DKLM gearbox. 
            }
            break;
        
        case NO_DAMAGE:
            // We do nothing here
            break;
    }

}
void OP_Tank::ResetBattle(void)
{
    // This function is called when the tank is "regenerating" or "recovering" after being destroyed (or when the TCB has just rebooted). 
    // During invulnerability time the tank is invulnerable to enemy fire for a length of time dependent on its class. 
    isDestroyed = false;        // We are no longer destroyed
    CannonHitsTaken = 0;        // Reset the hit counter
    MGHitsTaken = 0;
    DamagePct = 0;
    DisableHitReception();      // Ignore enemy fire
    TankTimer->setTimeout(BattleSettings.ClassSettings.recoveryTime, EnableHitReception);    // Enable hits after recovery (invulnerability) time has passed
}
void OP_Tank::ResetBattleImmediate(void)
{
    // This function is called when the TCB has just rebooted. 
    // We do the same thing as ResetBattle() except we don't wait to enable hit reception.
    isDestroyed = false;        // We are not longer destroyed
    CannonHitsTaken = 0;        // We have not been hit by anything yet
    MGHitsTaken = 0;
    DamagePct = 0;              // We have no damage yet
    EnableHitReception();       // Enable hits right now
}


//------------------------------------------------------------------------------------------------------------------------>>
// HIT NOTIFICATION LEDs 
//------------------------------------------------------------------------------------------------------------------------>>
void OP_Tank::HitLEDs_On(void)
{
    digitalWrite(pin_HitNotifyLEDs, HIGH);
    HitLEDsOn = true;
}
void OP_Tank::HitLEDs_Off(void)
{
    digitalWrite(pin_HitNotifyLEDs, LOW);
    HitLEDsOn = false;
}
void OP_Tank::HitLEDs_Toggle(void)
{
    HitLEDsOn ? HitLEDs_Off() : HitLEDs_On();
    HitLEDsOn = !HitLEDsOn;
}
void OP_Tank::HitLEDs_SetDim(uint8_t level)
{
    analogWrite(pin_HitNotifyLEDs, level);
    level > 0 ? HitLEDsOn = true : HitLEDsOn = false;
}

//------------------------------------------------------------------------------------------------------------------------>>
// HIT NOTIFICATION LEDs - CANNON HIT
//------------------------------------------------------------------------------------------------------------------------>>
// These flicker the LEDs that are typically installed in the IR "apple" to indicate damage received or tank destroyed. 
void OP_Tank::HitLEDs_CannonHit(void)
{   // The Cannon Hit effect randomly flickers the lights the same way Tamiya does. 

    // If we are still in the middle of running a flickering effect, just 
    // extend the time it runs. 
    if (TankTimer->isEnabled(HitLED_TimerID)) 
    { 
        // Delete the currently running timeout timer
        TankTimer->deleteTimer(HitLED_TimerID); 
        // Start a new one
        HitLED_TimerID = TankTimer->setTimeout(FLICKER_EFFECT_LENGTH_mS, CannonHitLEDs_Stop);
        // In the meanwhile, the flickering effect will continue to run
    }
    
    // In this case the previous effect is over, so start a new one
    if (!TankTimer->isEnabled(FadeStep_TimerID))
    {
        // We will set FadeOut = true later, when we are done with the effect and want to fade out
        FadeOut = false;

        // We start off at max brightness, 
        CurrentFadeLevel = 255;
        HitLEDs_On();
        
        // Pick some random brightness to fade down to
        FadeTarget = random(MIN_BRIGHT, DIM_FADE_BREAK);
        // Pick some random fade speed to get there
        FadeStep = -random(MIN_FADE_STEP, MAX_FADE_STEP);   // negative step value      
        // Start the fade timer, it will decrement the brightness by FadeStep every FADE_UPDATE_mS milliseconds until it reaches FadeTarget
        FadeStep_TimerID = TankTimer->setInterval(FADE_UPDATE_mS, CannonHitLEDs_Update);
        // Now start another one-shot timer to cancel the overall effect after FLICKER_EFFECT_LENGTH_mS milliseconds
        HitLED_TimerID = TankTimer->setTimeout(FLICKER_EFFECT_LENGTH_mS, CannonHitLEDs_Stop);
    }
}
void OP_Tank::CannonHitLEDs_Update(void)
{
static boolean StopNextTime = false; 

    // Increase or decrease the brightness level by FadeStep amount
    CurrentFadeLevel += FadeStep;
    // Set the LEDs to this brightness
    HitLEDs_SetDim(CurrentFadeLevel);
    
    // That's usually it, unless we've reached our target brightness. If so, we pick some
    // new random numbers and fade in the opposite direction
    if (FadeStep > 0 && CurrentFadeLevel >= FadeTarget)
    {
        // We reached the brightness target - now swap to some dim value
        if (FadeOut)
        {
            // This will be the last fade, and we will go slowly
            FadeTarget = 0;
            FadeStep = -4;  // Negative, because we are fading out. Small number so the fade takes time.  
            StopNextTime = true;        // This lets us know after we're done fading, that's it. 
        }
        else
        {   // This is a random fade
            FadeTarget = random(MIN_BRIGHT, DIM_FADE_BREAK);
            FadeStep = -random(MIN_FADE_STEP, MAX_FADE_STEP);   // negative step value
        }
    }
    else if (FadeStep < 0 && CurrentFadeLevel <= FadeTarget)
    {
        if (StopNextTime)
        {
            // We've reached the end of the last fade. Stop the timer and turn off the lights
            // (but they should already be off)
            if (TankTimer->isEnabled(FadeStep_TimerID))
            {
                TankTimer->deleteTimer(FadeStep_TimerID);
                HitLEDs_Off();
                CurrentFadeLevel = 0;
                // Reset
                StopNextTime = false;
                
                // BUT! If the tank is destroyed, we now start the HitLEDs_Destroyed effect
                if (isDestroyed) HitLEDs_Destroyed();
            }
        }
        else
        {
            // We reached our random dim target - now swap to some random bright value
            FadeTarget = random(BRIGHT_FADE_BREAK, MAX_BRIGHT);
            FadeStep = random(MIN_FADE_STEP, MAX_FADE_STEP);   // positive value
        }
    }
}
void OP_Tank::CannonHitLEDs_Stop(void)
{
    if (TankTimer->isEnabled(FadeStep_TimerID))
    {
        FadeOut = true;
    }
}

//------------------------------------------------------------------------------------------------------------------------>>
// HIT NOTIFICATION LEDs - MACHINE GUN HIT
//------------------------------------------------------------------------------------------------------------------------>>
void OP_Tank::HitLEDs_MGHit(void)
{
    // If we're hit by machine gun fire, we do a simple effect - two short blinks
    // But because we can, we make a nice little routine that lets us adjust precisely the
    // two blinks and the space between them. In fact with this approach you could create any 
    // sequence of blinks and spaces of any length of time.
    
    static uint8_t curStep = 0;
    const uint8_t steps = 3;
    const uint8_t time[steps] = {100,60,40};
  
    if (curStep < steps)
    {
        // Odd numbers get turned off
        if (curStep & 1)    
        {
            HitLEDs_Off();
            // Come back to this routine after the specified time for this step is up
            TankTimer->setTimeout(time[curStep], HitLEDs_MGHit);
        } 
        // Even numbers get turned on
        else        
        {
            HitLEDs_On();
            // Come back to this routine after the specified time for this step is up
            TankTimer->setTimeout(time[curStep], HitLEDs_MGHit);
        }
        curStep += 1;
    }
    else
    {
        // We're done, turn off the lights and don't come back
        HitLEDs_Off();
        curStep = 0;    // reset
    }
    
}


//------------------------------------------------------------------------------------------------------------------------>>
// HIT NOTIFICATION LEDs - DESTROYED
//------------------------------------------------------------------------------------------------------------------------>>
void OP_Tank::HitLEDs_Destroyed()
{
    static int DestroyedBlinkerID = 0;
    static boolean started = false;
    static int16_t fadeLevel = 255; // This needs to be a signed two-byte integer
    #define SLOW_FADE_OUT_STEP 2
    
    if (!started && isDestroyed)
    {   // In this case, we are just starting at the beginning of being destroyed.
    
        // Shoudn't need to, but delete the timer if it already exists
        if (TankTimer->isEnabled(DestroyedBlinkerID)) { TankTimer->deleteTimer(DestroyedBlinkerID); } 
        
        // Turn the lights on
        HitLEDs_On();   
        
        // Now set a timer to keep coming back here after a short interval so we can blink the lights
        DestroyedBlinkerID = TankTimer->setInterval(450, HitLEDs_Destroyed);     // This is a slow blink, about half a second
        
        // The effect has been started
        started = true;
    }
    else if (started && isDestroyed)
    {   // In this case, we already started the effect and we are coming back here just to blink the lights.  
        HitLEDs_Toggle();
    }
    else if (started && !isDestroyed)
    {   // Ok, the tank is done being destroyed. What we do now depends on whether the light is currently on or off
        if (HitLEDsOn)
        {
            // In this case, we can start fading out slowly
            TankTimer->deleteTimer(DestroyedBlinkerID);
            // Start a new timer that will call us much more frequently
            DestroyedBlinkerID = TankTimer->setInterval(FADE_UPDATE_mS, HitLEDs_Destroyed);
            started = false; // Change this to false
        }
        else
        {
            // We need to wait for the light to be on before we can fade off, so do one more toggle
            HitLEDs_Toggle();
        }
    }
    else if (!started && !isDestroyed)
    {
        // Now we are in the final fade-out phase
        fadeLevel -= SLOW_FADE_OUT_STEP;
        if (fadeLevel > 0)
        {   
            HitLEDs_SetDim(fadeLevel); // Dim the light
        }
        else
        {
            // We're completely done
            TankTimer->deleteTimer(DestroyedBlinkerID);
            HitLEDs_Off();
            fadeLevel = 255;    // reset for next time
        }
    }
}


//------------------------------------------------------------------------------------------------------------------------>>
// HIT NOTIFICATION LEDs REPAIR EFFECT - used both when firing repair code, and receiving repair code
//------------------------------------------------------------------------------------------------------------------------>>
void OP_Tank::Repair_BlinkHandler(void)
{   // Only start/continue the blinking effect if we are in the midst of being repaired
    if (RepairOngoing) 
    {
        HitLEDs_Repair(); 
    }
    else 
    {   // Stop the repair blinking effect
        if (TankTimer->isEnabled(HitLED_TimerID)) TankTimer->deleteTimer(HitLED_TimerID);
        HitLEDs_Off();
    }
}

void OP_Tank::HitLEDs_Repair(void)
{
    static boolean LightState = false;
    
    static boolean effectStarted = false;
    const int StartInterval = 500;   
    const int InitialSubtract = 43; 
    const int Knee1 = 36;
    const int Knee2 = 18;
    const int Knee3 = 1;
    const uint8_t SC_1 = 4;
    const uint8_t SC_2 = 2;
    const uint8_t SC_3 = 1;
    static int Interval;
    static int Subtract;

    if (!effectStarted) 
    {
        Interval = StartInterval;
        Subtract = InitialSubtract;
        effectStarted = true;
    }
    else
    {
        if      (Subtract > Knee1) Subtract -= SC_1;
        else if (Subtract > Knee2) Subtract -= SC_2;
        else if (Subtract > Knee3) Subtract -= SC_3;
        if      (Subtract < 0)     Subtract = 0;
    }

    // Toggle the light state
    LightState ? HitLEDs_Off() : HitLEDs_On();    
    LightState = !LightState;      // Flop states

    if (Interval <= 0)
    {
        effectStarted = false;
        HitLEDs_Off();
        // Effect is done, go back to Repair_BlinkHandler. It will restart the blinking effect if appropriate. 
        HitLED_TimerID = TankTimer->setTimeout(250,Repair_BlinkHandler);
    }
    else
    {
        // Now set a timer to come back here and toggle it after the correct length of time has passed.
        HitLED_TimerID = TankTimer->setTimeout(Interval, HitLEDs_Repair);
    }

    // For next time
    Interval -= Subtract;    
}

//------------------------------------------------------------------------------------------------------------------------>>
// LedHandler objects and other items that need polled updates
//------------------------------------------------------------------------------------------------------------------------>>
// The sketch will call this function every loop
void OP_Tank::Update(void)
{
    AppleLEDs.update();
    
    
}


