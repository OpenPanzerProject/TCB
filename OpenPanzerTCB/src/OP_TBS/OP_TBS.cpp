/* OP_TBS.cpp       Open Panzer TBS - a library for controlling the Benedini TBS Mini sound unit
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

#include "OP_TBS.h"
 
// Static variables must be initialized outside the class 
OP_SimpleTimer * OP_TBS::TBSTimer;
OP_Servos      * OP_TBS::TBSProp;
boolean         OP_TBS::Micro;
int             OP_TBS::TBSProp2TimerID;
boolean         OP_TBS::Prop2TimerComplete;
int             OP_TBS::TBSProp3TimerID;
boolean         OP_TBS::Prop3TimerComplete;
uint8_t         OP_TBS::currentProp3SoundNum;
uint8_t         OP_TBS::previousProp3SoundNum;
// Squeaky stuff
boolean         OP_TBS::Squeak1_Enabled;
boolean         OP_TBS::Squeak2_Enabled;
boolean         OP_TBS::Squeak3_Enabled;
boolean         OP_TBS::Squeak1_Active;
boolean         OP_TBS::Squeak2_Active;
boolean         OP_TBS::Squeak3_Active;
boolean         OP_TBS::AllSqueaks_Active;
int             OP_TBS::Squeak1TimerID;
int             OP_TBS::Squeak2TimerID;
int             OP_TBS::Squeak3TimerID;
unsigned int    OP_TBS::SQUEAK1_MIN_mS;
unsigned int    OP_TBS::SQUEAK1_MAX_mS;
unsigned int    OP_TBS::SQUEAK2_MIN_mS;
unsigned int    OP_TBS::SQUEAK2_MAX_mS;
unsigned int    OP_TBS::SQUEAK3_MIN_mS;
unsigned int    OP_TBS::SQUEAK3_MAX_mS;


// CONSTRUCTOR 
OP_TBS::OP_TBS(OP_SimpleTimer * t, boolean m) 
{
    TBSTimer = t;                       // We will use the sketch's SimpleTimer object rather than creating a new instance of the class
    
    Micro = m;                          // Are we interfacing with the Benidini Micro? If not, it's the Mini

    TBSProp = new OP_Servos; 
    
    // Initialize
    Prop2TimerComplete = true;
    Prop3TimerComplete = true;
    TBSProp2TimerID = 0;
    TBSProp3TimerID = 0;
    currentProp3SoundNum = SOUND_OFF;
    previousProp3SoundNum = SOUND_OFF;
    
    // Initialize squeak times, and set them to off to begin
    // Later we will load in the user's settings for squeak times and whether they are enabled or not. 
    SQUEAK1_MIN_mS = DEFAULT_SQUEAK_MIN_mS - 300;   // Squeak 1 is most frequent
    SQUEAK1_MAX_mS = DEFAULT_SQUEAK_MAX_mS - 500;
    SQUEAK2_MIN_mS = DEFAULT_SQUEAK_MIN_mS;         // Squeak 2 is medium frequency
    SQUEAK2_MAX_mS = DEFAULT_SQUEAK_MAX_mS;
    SQUEAK3_MIN_mS = DEFAULT_SQUEAK_MIN_mS + 1000;  // Squeak 3 is least frequent
    SQUEAK3_MAX_mS = DEFAULT_SQUEAK_MAX_mS + 1000;  
    Squeak1_Enabled = false;
    Squeak2_Enabled = false;
    Squeak3_Enabled = false;
    Squeak1_Active = false;
    Squeak2_Active = false;
    Squeak3_Active = false;
    Squeak1TimerID = 0;
    Squeak2TimerID = 0;
    Squeak3TimerID = 0;
    
    // Initialize these as well, but again, they will in the end be set by the user's preference
    HeadlightSound_Enabled = true;
    TurretSound_Enabled = true;
    BarrelSound_Enabled = true;
}


void OP_TBS::begin()
{
    TBSProp->attach(PROP1);
    TBSProp->attach(PROP2);
    if (Micro == false) TBSProp->attach(PROP3); // We only use Prop3 on the Mini
    InitializeOutputs();
}

void OP_TBS::InitializeOutputs(void)
{
    TBSProp->writeMicroseconds(PROP1, PROP1_IDLE);          // Initialize to speed = 0
    TBSProp->writeMicroseconds(PROP2, PROP2_SWITCH_OFF);    // Initialize to engine off
    if (Micro == false)
    {   
        TBSProp->writeMicroseconds(PROP3, Prop3SoundPulse(SOUND_OFF));  // Initialize to no special sounds, only if using Mini
        ClearProp3Timer();
    }
}


//------------------------------------------------------------------------------------------------------------------------>>
// PROP 1 - SETS THE ENGINE SOUNDS ACCORDING TO SPEED 
//------------------------------------------------------------------------------------------------------------------------>>
void OP_TBS::SetEngineSpeed(int speed)
{
    // speed will be somewhere in the range from MOTOR_MAX_REVSPEED (-255) to MOTOR_MAX_FWDSPEED (255) 
    // Map it to the TBS throttle range:
    speed = map(speed, 0, MOTOR_MAX_FWDSPEED, PROP1_IDLE, PROP1_FULL_SPEED);
    // Now send it to the TBS
    TBSProp->writeMicroseconds(PROP1, speed);
}

void OP_TBS::IdleEngine(void)
{   // This is a faster way than SetEngineSpeed to put the engine to idle
    TBSProp->writeMicroseconds(PROP1, PROP1_IDLE);
}

void OP_TBS::ClearThrottleBlip(void)
{   // Same as IdleEngine which however we can't call from this static function since it is public, and we need this one static so it can be used with SimpleTimer...
    TBSProp->writeMicroseconds(PROP1, PROP1_IDLE);  // Return the throttle to idle from our brief blip (only used with Mini)
}

//------------------------------------------------------------------------------------------------------------------------>>
// PROP 2 - Two sounds in 2nd Coder column of TBS Flash
//          When using the Mini:  1) ENGINE STARTUP/SHUTDOWN, 2) Tank Repair sound
//          When using the Micro: 1) Cannon Fire              2) Machine Gun
//------------------------------------------------------------------------------------------------------------------------>>
// The way this is set up, the two Prop2 sounds block each other. So if the engine is starting up/shutting down, the repair sound 
// can't play (but the startup/shutdown sounds don't last long). But, if we are in the midst of a repair operation (lasts 15 seconds)
// and the user decides to start or stop the engine, the sound won't play, and then the sound will be out of sync (engine sound playing when
// engine off, or engine sound silent when engine on). We attempt to prevent this problem by preventing the engine from changing status
// during repair operations (on the Driving tab of the main Sketch)
void OP_TBS::StartProp2Timer(void)
{
    // Start the timer to briefly toggle the Prop2 input
    Prop2TimerComplete = false;
    TBSProp2TimerID = TBSTimer->setTimeout(TBS_SIGNAL_PROP2_mS, ClearProp2Timer);
}

void OP_TBS::StartProp2Timer(int HowLong_mS)
{
    // Start the timer and leave it on for the set amount of time
    Prop2TimerComplete = false;
    TBSProp2TimerID = TBSTimer->setTimeout(HowLong_mS, ClearProp2Timer);
}

void OP_TBS::ClearProp2Timer(void)
{
    Prop2TimerComplete = true;
    // Turn off the Prop2 switch
    TBSProp->writeMicroseconds(PROP2, PROP2_SWITCH_OFF);
}

void OP_TBS::PROP2_OFF(void)
{
    // We probably don't need this, but this is a direct way of setting the switch to the off position
    TBSProp->writeMicroseconds(PROP2, PROP2_SWITCH_OFF);
}

void OP_TBS::StartEngine(void)
{
    // In the case of the Micro we don't have a function specifically to start/stop the engine. Instead we ask the user to set the Micro to engine Auto Start
    // and we will blip the throttle instead. 
    if (Micro)
    {
        SetEngineSpeed(75); // Not full throttle, but enough it should be registered
        TBSTimer->setTimeout(TBS_SIGNAL_mS, ClearThrottleBlip); // After a brief time the blip will be reverted to idle
    }
    else ToggleEngineSound();
}

void OP_TBS::StopEngine(void)
{
    if (Micro) return;  // We have no way of turning off the engine on the Micro, other than letting it turn off itself after some idle time. 
    else ToggleEngineSound();
}

void OP_TBS::ToggleEngineSound(void)
{
    if (Micro) return;
    else
    {
        if (Prop2TimerComplete) // Only send a signal if we are done with the last signal
        {
            TBSProp->writeMicroseconds(PROP2,PROP2_SWITCH_1);
            StartProp2Timer();
        }
    }
}

void OP_TBS::Repair(void)
{
    if (Micro) return;
    if (Prop2TimerComplete) // Only send a signal if we are done with the last signal
    {
        TBSProp->writeMicroseconds(PROP2,PROP2_SWITCH_2);   // Set to 2nd position
        StartProp2Timer(REPAIR_TIME_mS);                    // REPAIR_TIME_mS is defined in OP_BattleTimes.h (in OP_Tank folder)
    }
}

void OP_TBS::StopRepairSound(void)
{
    if (Micro) return;
    ClearProp2Timer();
}


//------------------------------------------------------------------------------------------------------------------------>>
// PROP 3 - Sixteen sounds in the 1st Coder column of TBS Flash (v3.0.1 and later) 
// Only when using the Mini 
//------------------------------------------------------------------------------------------------------------------------>>
// Prop 3 - can only play one sound at a time. If a sound is playing and a second sound begins before the first is finished, 
// the first will stop. This can occasionally cause odd behavior. For example, assume you have a cannon fire sound that lasts 
// three seconds, and you fire the cannon with the turret stick held in a corner. Then for example assume you immediately move the 
// stick away from the corner, such that the turret traverse sound begins playing. The turret sound will interrupt your cannon sound, 
// which won't complete. This is just the nature of the beast. You could set cannon sound priority higher than turret sound priority, 
// but since the cannon sound is a quick trigger sound, so far as the code knows the sound is over almost immediately, so it will still
// let the turret traverse sound start. The only foolproof way around this would be to hard-code the exact lengths of every sound into
// this class, but that isn't practical. 
// Another oddity that could happen, which we have programmed around, is that when firing the machine gun while moving, a squeak
// will interrup the machine gun sound, which is fine because squeaks are very short, but the machine gun would not re-start after
// the squeak, even though it might still be firing (the light will still blink). We have worked around this very case by setting
// the machine gun to be a "constant-on" sound (until specifically turned off - that is why you must set both an MG on trigger AND
// and MG off trigger). And we also set the MG sound to be a higher priority than squeaks. So now when you start the MG sound, it stays on,
// and no sound with a lower priority will play until it is turned off. 
void OP_TBS::StartProp3Timer(void)
{
    if (Micro) return;
    // Start the timer to briefly send a special sound signal. 
    Prop3TimerComplete = false;
    TBSProp3TimerID = TBSTimer->setTimeout(TBS_SIGNAL_mS, ClearProp3Timer);    
}

void OP_TBS::ClearProp3Timer(void)
{
    if (Micro) return;
    // Quit sending the special sound signal
    TBSProp->writeMicroseconds(PROP3, Prop3SoundPulse(SOUND_OFF));
    Prop3TimerComplete = true;
    currentProp3SoundNum = SOUND_OFF;
    previousProp3SoundNum = SOUND_OFF;
}

void OP_TBS::TriggerSpecialSound(int soundNum, boolean oneTime /* = true */)
{
    if (Micro) return;
    // Only send a special sound if it has a higher priority than the sound currently playing.
    // If no sound is playing the actual current sound will be SOUND_OFF. We made sure to set 
    // SOUND_OFF at priority 0 and all other sounds at least to priority 1, so they will always supersede SOUND_OFF. 
    if (Prop3SoundPriority(soundNum) > Prop3SoundPriority(currentProp3SoundNum))
    {   
        // If another sound was playing and its stop timer was still running, clear it so it won't stop this new sound
        if (TBSTimer->isEnabled(TBSProp3TimerID)) TBSTimer->deleteTimer(TBSProp3TimerID);
        // Now set the new pulse
        TBSProp->writeMicroseconds(PROP3, Prop3SoundPulse(soundNum));
        previousProp3SoundNum = currentProp3SoundNum;
        currentProp3SoundNum = soundNum;
        if (oneTime)    StartProp3Timer();  // If it's just a one-time sound, we start a brief timer so the pulse has time to register with the TBS, then it will turn the pulse off. 
    }
}

void OP_TBS::StopSpecialSounds(int soundNum)
{   
    if (Micro) return;
    // If the priority of the sound we want to stop is equal to or greater than the current sound,
    // stop the current sound. We need the equal-to check because we are probably trying to stop the 
    // current sound, in other words, it will be equal. 
    if (Prop3SoundPriority(soundNum) >= Prop3SoundPriority(currentProp3SoundNum))
    {
        ClearProp3Timer();
    }
}



//------------------------------------------------------------------------------------------------------------------------>>
// FIRE CANNON SOUND
//------------------------------------------------------------------------------------------------------------------------>>
void OP_TBS::Cannon(void)
{
    if (Micro)
    {
        TBSProp->writeMicroseconds(PROP2,PROP2_SWITCH_1);
        StartProp2Timer();
    }
    else    TriggerSpecialSound(SOUND_CANNON);  // Mini
}

//------------------------------------------------------------------------------------------------------------------------>>
// MACHINE GUN SOUND - Constant (needs an explicit call to StopSpecialSounds to turn off)
//------------------------------------------------------------------------------------------------------------------------>>
void OP_TBS::MachineGun(void)
{
    if (Micro) TBSProp->writeMicroseconds(PROP2,PROP2_SWITCH_2);    // Go to position 2 on the Prop 2 output
    else TriggerSpecialSound(SOUND_MG, false);  // We pass false, meaning this sound will Not occur just once, but instead remain active (repeating)
                                                // until explicitly turned off, or until interrupted by another sound with a higher priority. 
}
void OP_TBS::StopMachineGun(void)
{
    if (Micro) TBSProp->writeMicroseconds(PROP2, PROP2_SWITCH_OFF); // Turn off the Prop2 switch (return to center)
    else StopSpecialSounds(SOUND_MG);           // I previously was doing some other stuff that required a little bit different procedure
                                                // to turn off MG, hence this function instead of just calling StopSpecialSounds directly. 
}                                               // We'll leave it in case we need something like that again. The only place that calls this is OP_Tank.cpp - MachineGun_Stop() 


//------------------------------------------------------------------------------------------------------------------------>>
// SECOND MACHINE GUN SOUND - Constant (needs an explicit call to StopSpecialSounds to turn off)
//------------------------------------------------------------------------------------------------------------------------>>
// The Second machine gun sound, if implemented, takes the place of User Sound 3
void OP_TBS::SecondMachineGun(void)
{
    if (Micro) return;
    TriggerSpecialSound(SOUND_USER_3, false); // We pass false, meaning this sound will Not occur just once, but instead remain active (repeating)
                                            // until explicitly turned off, or until interrupted by another sound with a higher priority. 
}
void OP_TBS::StopSecondMachineGun(void)
{
    if (Micro) return;
    StopSpecialSounds(SOUND_USER_3);        // I previously was doing some other stuff that required a little bit different procedure
                                            // to turn off MG, hence this function instead of just calling StopSpecialSounds directly. 
}                                           // We'll leave it in case we need something like that again. The only place that calls this is the Battle tab of the sketch - MG2_Stop() 


//------------------------------------------------------------------------------------------------------------------------>>
// TURRET TRAVERSE
//------------------------------------------------------------------------------------------------------------------------>>
void OP_TBS::TurretSound_SetEnabled(boolean enabled)
{
    if (Micro) return;
    TurretSound_Enabled = enabled;
}

void OP_TBS::Turret(void)
{
    if (Micro) return;
    if (TurretSound_Enabled) TriggerSpecialSound(SOUND_TURRET);
}

void OP_TBS::StopTurret(void)
{
    if (Micro) return;
    if (TurretSound_Enabled) StopSpecialSounds(SOUND_TURRET);
}

//------------------------------------------------------------------------------------------------------------------------>>
// BARREL ELEVATION 
//------------------------------------------------------------------------------------------------------------------------>>
void OP_TBS::BarrelSound_SetEnabled(boolean enabled)
{
    if (Micro) return;
    BarrelSound_Enabled = enabled;
}

void OP_TBS::Barrel(void)
{
    if (Micro) return;
    if (BarrelSound_Enabled) TriggerSpecialSound(SOUND_BARREL);
}

void OP_TBS::StopBarrel(void)
{
    if (Micro) return;
    if (BarrelSound_Enabled) StopSpecialSounds(SOUND_BARREL);
}

//------------------------------------------------------------------------------------------------------------------------>>
// MACHINE GUN HIT - DAMAGE
//------------------------------------------------------------------------------------------------------------------------>>
void OP_TBS::MGHit(void)
{
    if (Micro) return;
    TriggerSpecialSound(SOUND_MG_HIT);
}

//------------------------------------------------------------------------------------------------------------------------>>
// CANNON HIT - DAMAGE
//------------------------------------------------------------------------------------------------------------------------>>
void OP_TBS::CannonHit(void)
{
    if (Micro) return;
    TriggerSpecialSound(SOUND_CANNON_HIT);
}

//------------------------------------------------------------------------------------------------------------------------>>
// BATTLE HIT - TANK DESTROYED
//------------------------------------------------------------------------------------------------------------------------>>
void OP_TBS::Destroyed(void)
{
    if (Micro) return;
    TriggerSpecialSound(SOUND_BATTLE_DESTROY);
}

//------------------------------------------------------------------------------------------------------------------------>>
// HEADLIGHTS On/Off SOUND
//------------------------------------------------------------------------------------------------------------------------>>
void OP_TBS::HeadlightSound_SetEnabled(boolean enabled)
{
    if (Micro) return;
    HeadlightSound_Enabled = enabled;
}

void OP_TBS::HeadlightSound(void)
{
    if (Micro) return;
    if (HeadlightSound_Enabled) TriggerSpecialSound(SOUND_HEADLIGHTS);
}

//------------------------------------------------------------------------------------------------------------------------>>
// USER SOUNDS
//------------------------------------------------------------------------------------------------------------------------>>
void OP_TBS::UserSound_Play(uint8_t s)
{
    if (Micro) return;
    switch (s)
    {
        case 1: TriggerSpecialSound(SOUND_USER_1); break;
        case 2: TriggerSpecialSound(SOUND_USER_2); break;
        case 3: TriggerSpecialSound(SOUND_USER_3); break;
        case 4: TriggerSpecialSound(SOUND_USER_4); break;
        case 5: TriggerSpecialSound(SOUND_USER_5); break;
        case 6: TriggerSpecialSound(SOUND_USER_6); break;
    }
}
void OP_TBS::UserSound_Repeat(uint8_t s)
{
    if (Micro) return;
    switch (s)
    {
        case 1: TriggerSpecialSound(SOUND_USER_1, false); break;    // We pass false, meaning this sound will Not occur just once, but instead remain active (repeating)
        case 2: TriggerSpecialSound(SOUND_USER_2, false); break;    // until explicitly turned off, or until interrupted by another sound with a higher priority. 
        case 3: TriggerSpecialSound(SOUND_USER_3, false); break;
        case 4: TriggerSpecialSound(SOUND_USER_4, false); break;
        case 5: TriggerSpecialSound(SOUND_USER_5, false); break;
        case 6: TriggerSpecialSound(SOUND_USER_6, false); break;        
    }
}
void OP_TBS::UserSound_Stop(uint8_t s)
{
    if (Micro) return;
    switch (s)
    {
        case 1: StopSpecialSounds(SOUND_USER_1); break;
        case 2: StopSpecialSounds(SOUND_USER_2); break;
        case 3: StopSpecialSounds(SOUND_USER_3); break;
        case 4: StopSpecialSounds(SOUND_USER_4); break;
        case 5: StopSpecialSounds(SOUND_USER_5); break;
        case 6: StopSpecialSounds(SOUND_USER_6); break;        
    }
}     

//------------------------------------------------------------------------------------------------------------------------>>
// VOLUME
//------------------------------------------------------------------------------------------------------------------------>>
void OP_TBS::IncreaseVolume(void)
{
    if (Micro) return;
    TriggerSpecialSound(SOUND_VOLUME_UP, false);    // We pass false, meaning this sound will Not occur just once, but instead remain active (repeating)
}                                                   // until explicitly turned off, or until interrupted by another sound with a higher priority. 
void OP_TBS::DecreaseVolume(void)
{
    if (Micro) return;
    TriggerSpecialSound(SOUND_VOLUME_DN, false);    // We pass false, meaning this sound will Not occur just once, but instead remain active (repeating)
}                                                   // until explicitly turned off, or until interrupted by another sound with a higher priority. 
void OP_TBS::StopVolume(void)
{
    if (Micro) return;
    StopSpecialSounds(SOUND_VOLUME_UP);             // It doesn't matter if we pass volume up or down here, as long as they both have the same priority. 
}


//------------------------------------------------------------------------------------------------------------------------>>
// SQUEAKS! 
//------------------------------------------------------------------------------------------------------------------------>>
void OP_TBS::StartSqueaks(void)
{   // We actually don't start squeaking right away because that can sound weird. We wait until the tank has been moving for
    // SQUEAK_DELAY_mS time before truly starting them
    if (Micro) return;    
    if (AllSqueaks_Active == false)
    {   // It doesn't matter what ID we use for this, so long as it's one that will get cleared when StopSqueaks() gets called
        Squeak1TimerID = TBSTimer->setTimeout(SQUEAK_DELAY_mS, StartSqueaksForReal);
        AllSqueaks_Active = true;
    }
}
void OP_TBS::StartSqueaksForReal(void)
{
    if (Micro) return;
    if (Squeak1_Enabled) Squeak1_Activate();
    if (Squeak2_Enabled) Squeak2_Activate();
    if (Squeak3_Enabled) Squeak3_Activate();
}
void OP_TBS::StopSqueaks(void)
{
    if (Micro) return;
    Squeak1_Pause();
    Squeak2_Pause();
    Squeak3_Pause();
    AllSqueaks_Active = false;
}
boolean OP_TBS::AreSqueaksActive(void)
{
    return AllSqueaks_Active;
}


void OP_TBS::SetSqueak_Interval(uint8_t s, unsigned int min, unsigned int max)
{
    switch (s)
    {
        case 1: 
            SQUEAK1_MIN_mS = min;
            SQUEAK1_MAX_mS = max;            
            break;
        case 2: 
            SQUEAK2_MIN_mS = min;
            SQUEAK2_MAX_mS = max;            
            break;
        case 3: 
            SQUEAK3_MIN_mS = min;
            SQUEAK3_MAX_mS = max;            
            break;
    }
}
void OP_TBS::Squeak_SetEnabled(uint8_t s, boolean enabled)
{   // This is for enabling the squeak or not, which is a user setting. It is not the same thing
    // as active/inactive (that determines if the squeak is squeaking or just waiting to squeak).
    // If a squeak is disabled, the sound won't play. 
    switch (s)
    {
        case 1: Squeak1_Enabled = enabled; break;
        case 2: Squeak2_Enabled = enabled; break;
        case 3: Squeak3_Enabled = enabled; break;
    }
}


void OP_TBS::Squeak1(void)
{
    if (Micro) return;
    if (Squeak1_Active)
    {
        // Play the squeak sound
        TriggerSpecialSound(SOUND_SQUEAK_1);
        // Wait some random amount of time, then call me again
        Squeak1TimerID = TBSTimer->setTimeout(random(SQUEAK1_MIN_mS,SQUEAK1_MAX_mS), Squeak1);    
    }
}
void OP_TBS::Squeak1_Activate(void)
{
    if (Micro) return;
    // Only activate me if I'm not already running and if I'm enabled in the first place. 
    if (Squeak1_Active == false && Squeak1_Enabled == true)
    {
        Squeak1_Active = true;
        Squeak1();      // Squeak1 will play itself over and over until disabled
    }
}
void OP_TBS::Squeak1_Pause(void)
{
    if (Micro) return;
    TBSTimer->deleteTimer(Squeak1TimerID);
    Squeak1_Active = false;
}


void OP_TBS::Squeak2(void)
{   
    if (Micro) return;
    if (Squeak2_Active)
    {
        // Play the squeak sound
        TriggerSpecialSound(SOUND_SQUEAK_2);
        // Wait some random amount of time, then call me again
        Squeak2TimerID = TBSTimer->setTimeout(random(SQUEAK2_MIN_mS,SQUEAK2_MAX_mS), Squeak2);       
    }
}
void OP_TBS::Squeak2_Activate(void)
{
    if (Micro) return;
    // Only activate me if I'm not already running and if I'm enabled in the first place. 
    if (Squeak2_Active == false && Squeak2_Enabled == true) 
    {
        Squeak2_Active = true;
        Squeak2();      // Squeak will play itself over and over until disabled
    }
}
void OP_TBS::Squeak2_Pause(void)
{
    if (Micro) return;
    TBSTimer->deleteTimer(Squeak2TimerID);
    Squeak2_Active = false;
}


void OP_TBS::Squeak3(void)
{
    if (Micro) return;
    if (Squeak3_Active)
    {
        // Play the squeak sound
        TriggerSpecialSound(SOUND_SQUEAK_3);
        // Wait some random amount of time, then call me again
        Squeak3TimerID = TBSTimer->setTimeout(random(SQUEAK3_MIN_mS,SQUEAK3_MAX_mS), Squeak3);    
    }
}
void OP_TBS::Squeak3_Activate(void)
{
    if (Micro) return;
    // Only activate me if I'm not already running and if I'm enabled in the first place. 
    if (Squeak3_Active == false && Squeak3_Enabled == true) 
    {
        Squeak3_Active = true;
        Squeak3();      // Squeak will play itself over and over until disabled
    }
}
void OP_TBS::Squeak3_Pause(void)
{
    if (Micro) return;
    TBSTimer->deleteTimer(Squeak3TimerID);
    Squeak3_Active = false;
}



//------------------------------------------------------------------------------------------------------------------------>>
// TEST ALL PROP 3 POSITIONS
//------------------------------------------------------------------------------------------------------------------------>>
// Used in development to test Benedini Flash v3.x
/*
void OP_TBS::testProp3(void)
{
    char buffer[30];    // This needs to be large enough to hold any string from our sound_descr_table (see OP_TBS.h)
    uint32_t soundNameTableAddress = pgm_get_far_address(_sound_descr_table_);  // This is the starting address of our sound names table in far progmem. 

    for (int i=1; i<PROP3_NUM_SOUNDS; i++)
    {
        // Example:
        // ...Sound 1: Cannon Fire
        Serial.print(F("...Sound ")); Serial.print(i); Serial.print(F(": "));
        // Using the string table in FAR program memory requires the use of a special function to retrieve the data.
        strcpy_PFAR(buffer, soundNameTableAddress, i*SOUNDNAME_CHARS);
        Serial.println(buffer);
        // Now send the signal for this sound number briefly, then wait two seconds before sending the next one (according to TBS teaching specs)

        // This is hardcoded with delays. We don't use it in normal practice, just for testing
        TBSProp->writeMicroseconds(PROP3, Prop3SoundPulse(i));
        delay(2000);
        TBSProp->writeMicroseconds(PROP3, Prop3SoundPulse(SOUND_OFF));

        // Wait
        delay(1000);
    }
}
*/

