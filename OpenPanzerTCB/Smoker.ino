
// --------------------------------------------------------------------------------------------->>
// SMOKER STUFF
// --------------------------------------------------------------------------------------------->>

// Typically the user will want the smoker to be controlled automatically by the TCB and set to 
// whatever the engine speed is. However we do give the user the option of controlling the smoker 
// manually with an analog input. The functions below are used by the main sketch to set the 
// smoker speed automatically, but they all check for the the SmokerControlAuto flag, and only
// perform the action if it is enabled. 

// In other words, we use these functions instead of direct calls to the Smoker object so we 
// can implement the auto check. 

// In some cases we also check if the user has the SmokerEnable flag set to true. 
// This flag can be controlled by the user on-the-fly if they want to temporarily disable the smoker
// from the transmitter. 

void EnableSmoker()
{
    if (!SmokerEnabled)
    {
        SmokerEnabled = true;
        if (DEBUG) { DebugSerial->println(F("Smoker enabled")); }
    }
}

void DisableSmoker()
{
    if (SmokerEnabled)
    {
        SmokerEnabled = false;
        StopSmoker();
        if (DEBUG) { DebugSerial->println(F("Smoker disabled")); }
    }
}

void ToggleSmoker()
{
    if (SmokerEnabled) DisableSmoker();
    else               EnableSmoker();
}

void StopSmoker(void)
{
    if (eeprom.ramcopy.SmokerControlAuto)
    {
        Smoker->stop();
    }
}
void ShutdownSmoker(boolean engaged)
{
    if (eeprom.ramcopy.SmokerControlAuto)
    {
        Smoker->Shutdown(engaged);
    }
}
void SetSmoker_Idle(void)
{
    if (eeprom.ramcopy.SmokerControlAuto && SmokerEnabled)
    {
        Smoker->setIdle();
    }
}
void SetSmoker_FastIdle(void)
{
    if (eeprom.ramcopy.SmokerControlAuto && SmokerEnabled)
    {
        Smoker->setFastIdle();
    }
}
void SetSmoker_Speed(int smoker_speed)
{
    if (eeprom.ramcopy.SmokerControlAuto && SmokerEnabled)
    {
        Smoker->setSpeed(smoker_speed);
    }
}
void Smoker_RestoreSpeed(void)
{
    if (eeprom.ramcopy.SmokerControlAuto && SmokerEnabled)
    {
        Smoker->restore_Speed(); 
    }
}
void Smoker_SetDestroyedSpeed()
{
    if (eeprom.ramcopy.SmokerControlAuto && SmokerEnabled && eeprom.ramcopy.SmokerDestroyedSpeed > 0)
    {
        Smoker->restore_Speed();    // The speed range may have been diminished during battle, we want to restore the full range
        Smoker->setSpeed(eeprom.ramcopy.SmokerDestroyedSpeed);  // Now apply the user's smoker speed setting for when the tank is destroyed
    }
}

// This function is called if the user wants to control the smoker output manually. Any motor could be attached to the output and run 
// in a single direction at variable speed with an analog input. 
void Smoker_ManualControl(uint16_t level)
{
    // This should be prevented from other sanity checking, but just in case, don't allow this function to do anything
    // if the smoker is being auto-controlled by the engine speed. 
    if (eeprom.ramcopy.SmokerControlAuto == false)
    {   // Recall, all analog special functions will receive values from 0-1023.
        // But the Smoker speed is a value from 0 to MOTOR_MAX_FWDSPEED, so we need to use the map function: 
        Smoker->setSpeed(map(level, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, 0, MOTOR_MAX_FWDSPEED));
    }
}

