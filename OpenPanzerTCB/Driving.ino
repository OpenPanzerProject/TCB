
// NUDGE
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void Nudge_End()
{
    Nudge = false;
}

// TRACK RECOIL
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void StartTrackRecoil()
{
    // The track recoil action is accomplished by changing the drive mode
    // For now, we only permit track recoil if the vehicle is stopped
    if (DriveModeActual == STOP) DriveModeActual = TRACK_RECOIL;
}
    

// SPECIAL FUNCTIONS: Driving Profile
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void SetDrivingProfile(uint8_t profile)
{
    if (profile == 2)
    {
        Driver.setDrivingProfileSettings(
                 eeprom.ramcopy.AccelRampEnabled_2, 
                 eeprom.ramcopy.DecelRampEnabled_2, 
                 eeprom.ramcopy.AccelPreset_2,
                 eeprom.ramcopy.DecelPreset_2,
                 eeprom.ramcopy.AccelSkipNum_2, 
                 eeprom.ramcopy.DecelSkipNum_2);            
    }
    else
    {
        Driver.setDrivingProfileSettings(
                 eeprom.ramcopy.AccelRampEnabled_1, 
                 eeprom.ramcopy.DecelRampEnabled_1, 
                 eeprom.ramcopy.AccelPreset_1,
                 eeprom.ramcopy.DecelPreset_1,
                 eeprom.ramcopy.AccelSkipNum_1, 
                 eeprom.ramcopy.DecelSkipNum_1);
    }

    // Update our global variable
    if (profile != DrivingProfile)
    {
        DrivingProfile = profile;

        if (DEBUG)
        {
            DebugSerial->print(F("Driving Profile set to #"));
            DebugSerial->println(profile);
        }
    }
}

void ToggleDrivingProfile(void)
{
    if (DrivingProfile == 1) SetDrivingProfile(2);
    else                     SetDrivingProfile(1);
}


// SPECIAL FUNCTIONS: Engine
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void EngineOn()
{   
    // We don't permit engine status changes during repair operations, because it will cause the sounds to get out of sync
    if (!Tank.isRepairOngoing())
    {
        // But if we're operating a manual transmission require the gear to be in neutral first
        if (ManualGear && ManualGear != GEAR_NEUTRAL) { if (DEBUG) DebugSerial->println(F("Manual transmission - put in neutral gear before starting engine!")); return; }
        
        // Start the engine object
        if (TankEngine.StartEngine())
        {
            // Play the engine start sound
                TankSound->StartEngine();
            // Should the transmission be engaged now too? 
                if (ManualGear == GEAR_NA)  // Only if we are not manually manipulating the transmission
                {
                    if (eeprom.ramcopy.TransmissionDelay_mS > 0)
                    {   // In this case the user has set a delay from the time the engine starts to when the transmission should be engaged. Typically this is to prevent the 
                        // transmission from engaging before the engine startup sound is complete. So we set a timer that will engage it after the set amount of time. 
                        skipTransmissionSound = true;  // We skip the transmission sound unless the user is manually manipulating the transmission, but this is just an automatic engage
                        timer.setTimeout(eeprom.ramcopy.TransmissionDelay_mS, TransmissionEngage); 
        
                        // Also start the smoker. We start in fast idle until the transmission engages
                        SetSmoker_FastIdle();
                    }
                    else 
                    {   // If there is no delay specified, engage the transmission right away. This will also set the smoker speed. 
                        skipTransmissionSound = true;  // No need to clunk the transmission just because we're turning the engine on
                        TransmissionEngage(); 
                    }
                }
            // Finally, if the user has set the engine auto-off feature, we start a timer for the specified length of time.
            // If this timer expires, the engine will be automatically shut off. The timer is restarted each time throttle command goes beyond idle. 
            // In other words, if the tank sits idle for too long, the engine will automatically turn off. 
                UpdateEngineIdleTimer();
        }
    }
}
void EngineOff()
{
    // We don't permit engine status changes during repair operations, because it will cause the sounds to get out of sync.
    // If you really need to shut down the engine (ie, Failsafe), make sure you call Tank.StopRepair() first
    // (for example, see StopEverything() below)
    if (!Tank.isRepairOngoing())
    {
        // Stop the engine object 
            TankEngine.StopEngine();
        // Stop the smoker
            // As opposed to StopSmoker, Shutdown will let the smoker turn off slowly for a more realistic effect. 
            // We need to tell the Shutdown effect where it is starting from, which depends on whether the transmission is currently engaged or not.
            TransmissionEngaged ? ShutdownSmoker(true) : ShutdownSmoker(false);
        // Now disengage the transmission object (no need to have transmission in gear if engine is off)
            skipTransmissionSound = true;  // No need to clunk the transmission just because we're turning the engine off
            TransmissionDisengage();
        // Play the engine stop sound
            TankSound->StopEngine();
        // Stop the drive motor(s)
            StopDriveMotors();
        // Clear the engine idle timer
            UpdateEngineIdleTimer();  
    }    
}
void EngineToggle()
{
    TankEngine.Running() ? EngineOff() : EngineOn();
}
void UpdateEngineIdleTimer()
{
    if (eeprom.ramcopy.EngineAutoStopTime_mS > 0)
    {
        if (TankEngine.Running())
        {
            if (timer.isEnabled(IdleOffTimerID)) timer.restartTimer(IdleOffTimerID);
            else IdleOffTimerID = timer.setTimeout(eeprom.ramcopy.EngineAutoStopTime_mS, EngineIdleOff);
        }
        else
        {
            StopEngineIdleTimer();
        }
    }
    
}
void StopEngineIdleTimer()
{
    if (timer.isEnabled(IdleOffTimerID)) timer.deleteTimer(IdleOffTimerID);
    IdleOffTimerID = 0;
}
void EngineIdleOff()
{
    // This function gets called if the tank has been sitting at idle for longer than the user auto-off setting.
    // So we shut down the engine
    if (TankEngine.Running()) 
    {
        EngineOff();
        if (DEBUG) DebugSerial->println(F("Engine Auto Shutdown"));
    }
}


// SPECIAL FUNCTIONS: Transmission
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void TransmissionEngage()
{
    // No point in messing with transmission if the engine isn't running
    // We also don't allow the transmission to be engaged if we're in the midst of a repair operation. 
    if (TankEngine.Running() && !Tank.isRepairOngoing() && !TransmissionEngaged) 
    { 
        TransmissionEngaged = true;
        SetSmoker_Idle();
        if (skipTransmissionSound)  skipTransmissionSound = false;          // Skip the sound, but reset the flag for next time
        else                        TankSound->EngageTransmission(true);    // Play the transmission engage sound
        if (DEBUG) DebugSerial->println(F("Engage Transmission")); 
    }
}

void TransmissionDisengage()
{
    // No point in messing with transmission if the engine isn't running
    if (TankEngine.Running() && TransmissionEngaged) 
    { 
        TransmissionEngaged = false;
        SetSmoker_FastIdle();
        if (skipTransmissionSound)  skipTransmissionSound = false;          // Skip the sound, but reset the flag for next time
        else                        TankSound->EngageTransmission(false);   // Play the transmission engage sound        
        if (DEBUG) DebugSerial->println(F("Disengage Transmission"));
    }
}

void TransmissionToggle()
{
    TransmissionEngaged ? TransmissionDisengage() : TransmissionEngage();
}

void ManualTransmission(_ManualTransGear m)
{
    // Save the setting to our manual gear global variable
    ManualGear = m; 
    
    switch (ManualGear)
    {
        case GEAR_FORWARD:
            TransmissionEngage();
            if (DEBUG) DebugSerial->println(F("Forward Gear"));
            break;

        case GEAR_REVERSE:
            TransmissionEngage();
            if (DEBUG) DebugSerial->println(F("Reverse Gear"));
            break;

        case GEAR_NEUTRAL:
            TransmissionDisengage();
            if (DEBUG) DebugSerial->println(F("Neutral Gear"));
            break;
    }
}


// SPECIAL FUNCTIONS: Turn Mode
// ----------------------------------------------------------------------------------------------------------------------------------------------->>
void SF_TurnMode1(uint16_t ignoreMe)            { if (Driver.getTurnMode() != 1) { Driver.setTurnMode(1); if (DEBUG) DebugSerial->println(F("Set Turn Mode: 1")); } }
void SF_TurnMode2(uint16_t ignoreMe)            { if (Driver.getTurnMode() != 2) { Driver.setTurnMode(2); if (DEBUG) DebugSerial->println(F("Set Turn Mode: 2")); } }
void SF_TurnMode3(uint16_t ignoreMe)            { if (Driver.getTurnMode() != 3) { Driver.setTurnMode(3); if (DEBUG) DebugSerial->println(F("Set Turn Mode: 3")); } }


// SPECIAL FUNCTIONS: Neutral Turns
// ----------------------------------------------------------------------------------------------------------------------------------------------->>
// Only enable neutral turns for tanks and halftracks, not cars. 
void SF_NT_Enable(uint16_t ignoreMe)            { if (eeprom.ramcopy.DriveType != DT_CAR && Driver.getNeutralTurnAllowed() == false) { Driver.setNeutralTurnAllowed(true);  if (DEBUG) DebugSerial->println(F("Enable Neutral Turns"));  } }
void SF_NT_Disable(uint16_t ignoreMe)           { if (Driver.getNeutralTurnAllowed() == true ) { Driver.setNeutralTurnAllowed(false); if (DEBUG) DebugSerial->println(F("Disable Neutral Turns")); } }
void SF_NT_Toggle(uint16_t ignoreMe)            { Driver.getNeutralTurnAllowed() ? SF_NT_Disable(0) : SF_NT_Enable(0); }


// OTHER DRIVING FUNCTIONS
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void StopDriveMotors()
{
    switch (eeprom.ramcopy.DriveType)
    {
        case DT_TANK:       { RightTread->stop(); LeftTread->stop();     }  break;
        case DT_HALFTRACK:  { RightTread->stop(); LeftTread->stop();     }  break;
        case DT_CAR:        { DriveMotor->stop();                        }  break;
        case DT_DKLM:       // Fall through
        case DT_DMD:        { DriveMotor->stop(); SteeringMotor->stop(); }  break;
        default:                                                            break;
    }

    // We're not moving, so stop the squeaking
    TankSound->StopSqueaks();
    // And stop any track overlay sounds too
    TankSound->SetVehicleSpeed(0);
}

void StopEverything()
{   // We use this in the event of a radio failsafe event, or just as a quick way to stop all movement. 
    // Stop drive motor(s) and any sound associated with them
    StopDriveMotors();
    // Make sure we clear any ongoing repairs, otherwise the engine won't turn off
    Tank.StopRepair(); 
    // Turn Engine off
    if (TankEngine.Running()) 
    { 
        EngineOff(); 
        // Sounds
        TankSound->IdleEngine();        // TBS throttle speed = idle
        TankSound->StopEngine();
    }
    // Stop turret motors
    TurretElevation->stop();
    TurretRotation->stop();
    // Stop smoker
    StopSmoker();
}





