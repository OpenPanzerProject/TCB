
// SPECIAL FUNCTIONS: Cannon
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// Cannon fire must trigger several things: mechanical recoil unit, servo recoil (we trigger both, the user will of course only have one hooked-up), 
// the high intensity flash unit, the cannon sound from the TBS unit, and last but not least, the IR pulse. 
// The cannon must also pause between shots for reloading, this time varies with the tank weight class. 

void FireCannon()
{
    if (Tank.CannonReloaded())          // Only fire if reloading is complete
    {   
        if (Tank.isRepairTank()) 
        {   
            if (!Tank.isRepairOngoing() && !RepairOngoing)
            {
                // If we are a repair tank, we immobilize the tank when firing the repair signal. 
                // This is very similar to what we do if we *receive* a repair signal
                RepairOngoing = REPAIR_OTHER;   // This marks the start of a repair operation - we are repairing an other vehicle
                if (DEBUG) { DebugSerial->println(F("Fire Repair Signal")); }
                
                // Disengage the transmission - we will keep it in neutral until the repair is over. 
                // Even if the user tries to re-engage it, the TransmissionEngage() function will check if a repair is ongoing, if so, it won't do anything. 
                // And without an engaged transmission, the tank will not move. 
                skipTransmissionSound = true;  // No need to clunk the transmission now
                TransmissionDisengage();
                
                // Now fire the repair signal. 
                Tank.Fire(); 
                CannonWasFired = true;
            }
        }
        else
        {
            // This is a fighting tank. But we can't fire the cannon if we're in the midst of being repaired by another tank.
            if (!Tank.isRepairOngoing())
            {
                if (DEBUG) 
                { 
                    DebugSerial->print(F("Fire Cannon")); 
                    if (Tank.IsIREnabled()) DebugSerial->print(F(" (with IR)"));
                    DebugSerial->println();
                } 
                Tank.Fire(); // See OP_Tank library. This triggers the mechanical and servo recoils, the high intensity flash unit, the cannon sound, and it sends the IR signal
                CannonWasFired = true;
                
                // Handle track recoil if the user has it enabled
                if (eeprom.ramcopy.EnableTrackRecoil)
                {
                    // When the track recoil starts depends on whether we we have a mechanical barrel action included and if so which one
                    if (Tank.isMechBarrelSetWithCannon())
                    {
                        if (eeprom.ramcopy.Airsoft)
                        {
                            // With the airsoft unit we must wait until the airsoft actually fires, which will happen after it has cocked. We will just have to poll the tank class to see when this is. 
                            // The CheckAirsoft will keep checking back, and when the airsoft has cocked, and if the vehicle is actually stopped, then it will kick off the track recoil
                            CheckAirsoftTimerID = timer.setInterval(50, CheckAirsoft);
                        }
                        else    // Mechanical recoil
                        {
                            if (DriveModeActual == STOP)    // If we are stopped, track recoil is possible
                            {
                                // With mechanical recoil we can start it more or less right away, or else after a single, pre-known delay if one is specified
                                if (eeprom.ramcopy.RecoilDelay > 0) timer.setTimeout(eeprom.ramcopy.RecoilDelay, StartTrackRecoil); // We may need to delay it
                                else StartTrackRecoil();                                                                            // Otherwise go directly to it
                            }
                        }
                    }
                    else
                    {   // Here there is no mechanical barrel action so we have nothing to wait for, go straight to track recoil
                        StartTrackRecoil(); 
                    }
                }
            }
        }
    }
}

void CheckAirsoft()
{
static uint8_t timesThrough = 0; 

    if (Tank.AirsoftFired())
    {
        // Ok, the airsoft has finally fired. 
        timer.deleteTimer(CheckAirsoftTimerID);         // Stop the polling timer
        CheckAirsoftTimerID = 0;
        Tank.ClearAirsoft();                            // Reset the airsoft flag in the Tank class
        if (DriveModeActual == STOP && eeprom.ramcopy.EnableTrackRecoil)
        {
            StartTrackRecoil();
        }
        timesThrough = 0;                               // Reset the count
    }
    else
    {   // Otherwise just increment our count and come back later. 
        timesThrough += 1;
    
        if (timesThrough >= 80)                         // 80 times 50mS = 4 seconds, something should have happend by now for sure. 
        {
            // In this case we've been coming back for some time now and the airsoft has never fired, so we assume something went wrong and cancel this timer so we don't keep wasting resources
            timer.deleteTimer(CheckAirsoftTimerID);     // Stop the polling timer
            CheckAirsoftTimerID = 0;
            Tank.ClearAirsoft();                        // Reset the airsoft flag in the Tank class   
            timesThrough = 0;         
        }    
    }
}

// SPECIAL FUNCTIONS: Machine Gun
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// If you are using the turret stick to toggle the Machine Gun, you need to specify two conditions - the stick position that will turn the MG on, 
// AND the stick position that will turn it off. The position that makes most sense for off is MC (Middle-Center, or in other words, stick returned to neutral). 
// If you are using a physical aux switch, you also have to specify an on/off of course.
// There is no Toggle special function for the MG the way there is for Headlights or Engine, etc... 
void MG_Start()
{   
    Tank.MachineGun();
    if (DEBUG) {DebugSerial->println(F("MG Start"));}
}
void MG_Stop()
{
    Tank.MachineGun_Stop();
    if (DEBUG) {DebugSerial->println(F("MG Stop"));}
}
void MG2_Start()
{   // We're not going to deal with this inside the tank class... just handle it locally as Light 2
    Light2Blink();
    TankSound->SecondMachineGun();
}
void MG2_Stop()
{   // Also debugging messages about the 2nd machine gun are handled in the Light 2 routines on the Lights tab
    Light2Off();    
    TankSound->StopSecondMachineGun();
}


// SPECIAL FUNCTIONS: Trigger Mechanical Barrel; Enable/Disable/Toggle Mechanical Barrel (Mechanical Barrel = Airsoft or Mechanical Recoil Unit)
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// If the setting "Trigger With Cannon" is checked in OP Config on the Motors tab, the mechanical barrel (airsoft or recoil) will trigger automatically
// with the FireCannon function above. If un-checked, FireCannon will still play the sound, flash, IR, etc.. but will not trigger the mechanical barrel. 
// Instead the mechanical barrel can be triggered manually by the user through this function. It will simply acivate the airsoft or mechanical recoil action
// (whichever is selected in OP Config) but that's it - no sound, flash, or anything else. This is to allow the user to create their own effects. 
void MechBarrel()
{
    Tank.TriggerMechBarrel();
    if (DEBUG) 
    { 
        if (eeprom.ramcopy.Airsoft) DebugSerial->println(F("Airsoft - Manual Trigger")); 
        else DebugSerial->println(F("Mechanical Recoil - Manual Trigger")); 
    }
}
// These functions let us include or not include the mechanical barrel action with the CannonFire function (mechanical barrel is either Airsoft or mechanical recoil unit, whichever the user has specified). 
// The setting can be specified once in OP Config, but the user may also want to change it on the fly - for example if they have a combination airsoft/IR setup, they may want to fire the IR on occasion
// without firing the airsoft, so this would let them turn airsoft off (disable the "mechanical" barrel). Note this has nothing to do with the recoil servo. 
void MechBarrel_Enable()
{
    Tank.SetMechBarrelWithCannon(true);
    if (DEBUG)
    { 
        if (eeprom.ramcopy.Airsoft) DebugSerial->println(F("Airsoft - Enabled")); 
        else DebugSerial->println(F("Mechanical Recoil - Enabled"));     
    }
}
void MechBarrel_Disable()
{
    Tank.SetMechBarrelWithCannon(false);
    if (DEBUG)
    { 
        if (eeprom.ramcopy.Airsoft) DebugSerial->println(F("Airsoft - Disabled")); 
        else DebugSerial->println(F("Mechanical Recoil - Disabled"));     
    }
}
void MechBarrel_Toggle()
{
    if (Tank.isMechBarrelSetWithCannon())
    {
        MechBarrel_Disable();
    }
    else
    {
        MechBarrel_Enable();
    }
}

// SPECIAL FUNCTIONS: Servo Recoil
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// This function allows the user to manually trigger the servo recoil effect
void TriggerServoRecoil()
{
    RecoilServo->Recoil();
    if (DEBUG) { DebugSerial->println(F("Servo Recoil - Manual Trigger")); }
}

// SPECIAL FUNCTIONS: Trigger Muzzle Flash
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// Typically the user doesn't need to worry about this, the muzzle flash will be triggered automatically when the FireCannon function is called. 
// However we give them the option of triggering it manually if they want, in order to create their own effects. 
void MuzzleFlash()
{
    Tank.TriggerMuzzleFlash();
    if (DEBUG) { DebugSerial->println(F("High Intensity Flash Unit - Manual Trigger")); }
}

// SPECIAL FUNCTIONS: Enable/Disable IR functionality
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void IR_Enable()
{
    Tank.EnableIR();
    if (DEBUG) 
    { 
        if (eeprom.ramcopy.IR_FireProtocol == IR_DISABLED && eeprom.ramcopy.IR_MGProtocol == IR_DISABLED) DebugSerial->println(F("IR NOT enabled - no IR protocols selected.")); 
        else if (Tank.IsIREnabled()) DebugSerial->println(F("IR enabled"));
        else DebugSerial->println(F("Unable to enable IR"));
    }
}

void IR_Disable()
{
    Tank.DisableIR();
    if (DEBUG) { DebugSerial->println(F("IR disabled")); }
}

void IR_Toggle()
{
    Tank.ToggleIR();
}




