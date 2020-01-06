
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


// SPECIAL FUNCTIONS: On-the-fly speed reductions
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void ReduceSpeed(uint8_t speedAmt)
{
    static int SavedForwardSpeed_Max = ForwardSpeed_Max;
    static int SavedReverseSpeed_Max = ReverseSpeed_Max;
    
    // speedAmt will be the new percentage we want, not the percentage we want reduced.
    // For example, if speedAmt is 75, reduce max speed to 75%
    if (speedAmt < 100)
    {
        ForwardSpeed_Max = (int)(((float)speedAmt / 100.0) * (float)SavedForwardSpeed_Max); 
        ReverseSpeed_Max = (int)(((float)speedAmt / 100.0) * (float)SavedReverseSpeed_Max); 
        if (DEBUG) 
        {
            DebugSerial->print(F("Speed reduced to "));
            DebugSerial->print(speedAmt);
            DebugSerial->println(F("%"));
        }
    }
    else
    {
        // Restore speed
        ForwardSpeed_Max = SavedForwardSpeed_Max; 
        ReverseSpeed_Max = SavedReverseSpeed_Max; 
        if (DEBUG) DebugSerial->println(F("Speed restored"));
    }
}


// SPECIAL FUNCTIONS: Engine
// -------------------------------------------------------------------------------------------------------------------------------------------------->
boolean StartEngine()
{
    UpdateEngineStatusDelayTimer();
    if (EngineTimerComplete && !EngineRunning)  // Only change engine status if some length of time has passed since last change
    {                                           // AND only if engine is not already running
        EngineRunning = true;                   // Change state to running
        EngineTimerComplete = true;             // Reset engine timer
        StartEngineTimer();
        if (DEBUG) { DebugSerial->println(F("Turn Engine On")); }
        return true;
    }
    else
    {   // Engine wasn't started, return false 
        return false;
    }
}
boolean StopEngine(boolean msg)
{
    UpdateEngineStatusDelayTimer();
    if ((EngineTimerComplete && EngineRunning) || EngineInPreheat)   // Only change engine status if some length of time has passed since last change (if already on), or if we are in the preheat stage and the engine is scheduled to start
    {                                           // AND only if engine is already running
        EngineRunning = false;                  // Change state to stopped
        EngineTimerComplete = true;             // Reset engine timer
        StartEngineTimer();
        if (DEBUG && msg) 
        { 
            DebugSerial->print("Turn Engine Off"); 
            if (EngineInPreheat) DebugSerial->print(" (engine pre-heat cancelled)");
            DebugSerial->println();
        }        
        return true;
    }
    else
    {   // Engine wasn't stopped, return false
        return false;
    }    
}
void StartEngineTimer()
{
    if (eeprom.ramcopy.EnginePauseTime_mS > 0)
    {
        // Start the engine timer. Until it expires, it will not be possible to change that status of the engine. 
        if (EngineTimerComplete)    // Meaning, the timer is not running
        {
            EngineTimerStartTime = millis();
            EngineTimerComplete = false;
        }
    }
    else
    {
        // No pause time specified, we allow immediate changes
        EngineTimerComplete = true;
    }
}
void UpdateEngineStatusDelayTimer()
{
    if (!EngineTimerComplete)
    {
        if ((millis() - EngineTimerStartTime) >= eeprom.ramcopy.EnginePauseTime_mS)
        {   // Time's up
            EngineTimerComplete = true;
        }
    }
}
void EngineOn()
{   
boolean Proceed = true;
    
    // We don't permit engine status changes during repair operations, because it will cause the sounds to get out of sync
    if (!Tank.isRepairOngoing())
    {
        // But if we're operating a manual transmission require the gear to be in neutral first
        if (ManualGear && ManualGear != GEAR_NEUTRAL) { if (DEBUG) DebugSerial->println(F("Manual transmission - put in neutral gear before starting engine!")); Proceed = false; return; }
        
        // Start the engine object, but first check if we need to pre-heat the smoker
        if (eeprom.ramcopy.SmokerDeviceType != SMOKERTYPE_ONBOARD_STANDARD && SmokerPreHeat_Sec > 0 && EngineInPreheat == false)
        {   
            // In this case the preheater has not been started yet, so start it if appropriate            
            if (eeprom.ramcopy.HotStartTimeout_Sec > 0 && LastStopTime > 0 && ((millis() - LastStopTime) < (eeprom.ramcopy.HotStartTimeout_Sec * 1000L)))
            {
                // Here we are re-starting the engine within the hot start time since the last shutdown, so skip the pre-heat
                Proceed = true;
            }
            else
            {
                // In this case there will be a delay between the time the user commands engine start and when we actually start it, in order to give the heating element time to heat up
                Smoker->preHeat();              // Turn on the heater
                TankSound->PreHeatSound();      // Play the pre-heat sound
                EngineInPreheat = true;         // Set a flag so we know the next time we come back here
                PreheatTimerID = timer.setTimeout((SmokerPreHeat_Sec * 1000L), EngineOn);  // Set a timer to return here to turn the engine on after the pre-heat time has transpired. 
                if (DEBUG) { DebugSerial->print(F("Smoker pre-heat started - engine will turn on in ")); DebugSerial->print(SmokerPreHeat_Sec); DebugSerial->println(F(" seconds")); }
                Proceed = false;
            }
        }
        
        if (Proceed && StartEngine())
        {   
            // Give the user a notification if we are skipping the smoker pre-heat
                if (EngineInPreheat == false && eeprom.ramcopy.SmokerDeviceType != SMOKERTYPE_ONBOARD_STANDARD && SmokerPreHeat_Sec > 0 && eeprom.ramcopy.HotStartTimeout_Sec > 0 && LastStopTime > 0 && ((millis() - LastStopTime) < (eeprom.ramcopy.HotStartTimeout_Sec * 1000L))) 
                {
                    DebugSerial->print(F("Engine last stopped "));
                    DebugSerial->print(((millis() - LastStopTime) / 1000L),1);
                    DebugSerial->print(F(" seconds ago - smoker preheat skipped"));
                    DebugSerial->println();
                }
            // Clear the pre-heat flag if set
                EngineInPreheat = false;
            // Set the engine start ad-hoc trigger bit 
                bitSet(AdHocTriggers, ADHOCT_BIT_ENGINE_START);
            // Play the engine start sound
                TankSound->StartEngine();
            // Should the transmission be engaged now too? 
                if (ManualGear == GEAR_NA)  // Only if we are not manually manipulating the transmission
                {
                    if (eeprom.ramcopy.TransmissionDelay_mS > 0)
                    {   // In this case the user has set a delay from the time the engine starts to when the transmission should be engaged. Typically this is to prevent the 
                        // transmission from engaging before the engine startup sound is complete. So we set a timer that will engage it after the set amount of time. 
                        skipTransmissionSound = true;  // We skip the transmission sound unless the user is manually manipulating the transmission, but this is just an automatic engage
                        timer.setTimeout(eeprom.ramcopy.TransmissionDelay_mS, TransmissionEngageWithMsg); 
        
                        // Also start the smoker. We start in fast idle until the transmission engages
                        StartSmoker(false); // We pass false for "transmission not engaged"
                    }
                    else 
                    {   // If there is no delay specified, engage the transmission right away. This will also set the smoker speed. 
                        skipTransmissionSound = true;       // No need to clunk the transmission just because we're turning the engine on
                        smokerStartupWithEngage = true;     // We want the transmission engage function to issue the smoker startup command. Otherwise, for normal toggling of the transmission 
                                                            // after the engine has been started, the transmission engage function will simply set the smoker idle speed. 
                        TransmissionEngage(false);          // Pass false here to skip the message about engaging the transmission, since it is being done automatically. 
                    }
                }
            // Finally, if the user has set the engine auto-off feature, we start a timer for the specified length of time.
            // If this timer expires, the engine will be automatically shut off. The timer is restarted each time throttle command goes beyond idle. 
            // In other words, if the tank sits idle for too long, the engine will automatically turn off. 
                UpdateEngineIdleTimer();
        }
    }
}
void EngineOff(boolean debugMsg=true);
void EngineOff(boolean debugMsg)
{
    // We don't permit engine status changes during repair operations, because it will cause the sounds to get out of sync.
    // If you really need to shut down the engine (ie, Failsafe), make sure you call Tank.StopRepair() first
    // (for example, see StopEverything() below)
    if (!Tank.isRepairOngoing())
    {
        if (StopEngine(debugMsg))   // We may not actually be able to stop the engine yet if the user has a delay specified - or if it is already stopped        
        {        
            // Clear the pre-heat flag if set
                if (EngineInPreheat)
                {
                    if (PreheatTimerID > 0) timer.deleteTimer(PreheatTimerID);  // Cancel the delayed engine start if it was active
                    EngineInPreheat = false;
                }
            // Set the engine stop ad-hoc trigger bit 
                bitSet(AdHocTriggers, ADHOCT_BIT_ENGINE_STOP);
            // Stop the smoker
                // As opposed to StopSmoker, Shutdown will let the smoker turn off slowly for a more realistic effect. 
                // We need to tell the Shutdown effect where it is starting from, which depends on whether the transmission is currently engaged or not.
                TransmissionEngaged ? ShutdownSmoker(true) : ShutdownSmoker(false);
            // Now disengage the transmission object (no need to have transmission in gear if engine is off)
                skipTransmissionSound = true;  // No need to clunk the transmission just because we're turning the engine off
                TransmissionDisengage();
            // Play the engine stop sound
                if (eeprom.ramcopy.SoundDevice == SD_BENEDINI_TBSMINI && Tank.isDestroyed)
                {
                    // The Benedini Mini doesn't like to play the engine shutdown and destroyed sounds simultaneously, but we definitely do still need
                    // to give it the engine off signal. If we delay it slightly the destroyed sound will play and the engine off signal still gets received.
                    timer.setTimeout(100, EngineOffSound); 
                }
                else EngineOffSound();  // Otherwise if not destroyed or not Benedini Mini we can play the sound immediately
            // Stop the drive motor(s)
                StopDriveMotors();
            // Clear the engine idle timer
                UpdateEngineIdleTimer();  
            // Save the time, we can use this to skip the smoker pre-heat delay if they start the engine again shortly after the last stop
                LastStopTime = millis();
        }
    }    
}
void EngineOffSound()
{
    TankSound->IdleEngine();  // TBS throttle speed = idle - makes no difference if included or not, or before or after stop, in terms of destroyed sound playing. 
    TankSound->StopEngine();
}
void EngineToggle()
{
    EngineRunning ? EngineOff() : EngineOn();
}
void UpdateEngineIdleTimer()
{
    if (eeprom.ramcopy.EngineAutoStopTime_mS > 0)
    {
        if (EngineRunning)
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
    if (EngineRunning) 
    {
        EngineOff();
        if (DEBUG) DebugSerial->println(F("Engine Auto Shutdown"));
    }
}


// SPECIAL FUNCTIONS: Transmission
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// Function prototype that defaults debugMsg = true is located at top of sketch main tab.
void TransmissionEngage(boolean debugMsg)
{
    // No point in messing with transmission if the engine isn't running
    // We also don't allow the transmission to be engaged if we're in the midst of a repair operation. 
    if (EngineRunning && !Tank.isRepairOngoing() && !TransmissionEngaged) 
    { 
        TransmissionEngaged = true;
        if (smokerStartupWithEngage)
        {   // In this case, the engine has just been started and there is no delay specified between the engine start and transmission engage,
            // so we issue the smoker startup command
            StartSmoker(true);
            smokerStartupWithEngage = false;
        }
        else
        {   // In this case we are just toggling the transmission after the engine has already been started, so we just set the idle speed
            SetSmoker_Idle();
        }
        
        if (skipTransmissionSound)  skipTransmissionSound = false;          // Skip the sound, but reset the flag for next time
        else                        TankSound->EngageTransmission(true);    // Play the transmission engage sound
        if (DEBUG && debugMsg) DebugSerial->println(F("Engage Transmission"));         
    }
}
void TransmissionEngageWithMsg(void)
{
    // We use this function when we want to engage the transmission via the timer class
    TransmissionEngage(true);
}
void TransmissionDisengage()
{
    // If the engine is running and we disengage, we need to possibly play a sound and adjust the smoker speed
    if (EngineRunning && TransmissionEngaged) 
    { 
        TransmissionEngaged = false;
        SetSmoker_FastIdle();
        if (skipTransmissionSound)  skipTransmissionSound = false;          // Skip the sound, but reset the flag for next time
        else                        TankSound->EngageTransmission(false);   // Play the transmission disengage sound        
        if (DEBUG) DebugSerial->println(F("Disengage Transmission"));
    }

    // Otherwise we might be disengaging the engine because we've just turned the engine off. In that case the smoker was already handled in the EngineOff function,
    // and we don't want to play a sound or print a message anyway, so just set this to false. 
    TransmissionEngaged = false;    // But even if the engine wasn't running, clear this variable
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
        case DT_DIRECT:     { RightTread->stop(); LeftTread->stop();     }  break;        
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
    if (EngineRunning) 
    { 
        // Now turn engine off, this will also play the engine stop sound
        EngineOff(); 
    }
    // Stop turret motors
    TurretElevation->stop();
    TurretRotation->stop();
    // Stop smoker
    StopSmoker();
}





