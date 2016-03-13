// This tab doesn't really use the IMU, these are just some functions for turning off/on/toggle 
// functions related to the IMU

// See the SpecFunctions tab for the special functions related to changing the sensitivity parameters
// on the fly. 


// We use this to restrict how often we sample the IMU. After each sample the IMU_ReadyToSample flag is set to false.
// A timer is started that will last for SampleIMU_Delay length of time. While ReadyToSample = false, no samples will be taken.
// When the timer expires it calls this function which sets the ready flag to True, and a new sample is taken, starting
// the process over again. 
void SetReadyToSample(void)
{
    IMU_ReadyToSample = true;
}

void EnableBarrelStabilization(boolean enable)
{
    // We can't just enable barrel stabilization because the user says so. Several other conditions
    // must also be met. 
    if (enable && IMU_Present && eeprom.ramcopy.TurretElevationMotor == SERVO_PAN)
    {   // Ok, we can enable barrel stabilization
        eeprom.ramcopy.EnableBarrelStabilize = true;
        UseIMU = true;    // Make sure we're taking readings
        if (DEBUG) 
        {
            DebugSerial->print(F("Barrel Stabilization: Turned ON"));
            PrintBarrelSensitivity();
            DebugSerial->println();
        }
    }
    else
    {
        eeprom.ramcopy.EnableBarrelStabilize = false;
        if (DEBUG) DebugSerial->println(F("Barrel Stabilization: Turned OFF"));
        // If hill physics is also disabled, just quit using accelerometer completely
        // (If UseAccel = false, we won't bother taking readings, which are time-consuming)
        if (eeprom.ramcopy.EnableHillPhysics == false)
        {
            UseIMU = false;
        }        
    }
}

void ToggleBarrelStabilization()
{
    EnableBarrelStabilization(!eeprom.ramcopy.EnableBarrelStabilize);
}


void EnableHillPhysics(boolean enable)
{
    if (enable && IMU_Present)
    {
        eeprom.ramcopy.EnableHillPhysics = true;
        UseIMU = true;    // Make sure we're taking readings
        if (DEBUG) 
        {
            DebugSerial->print(F("Hill Physics: Turned ON"));
            PrintHillSensitivity();
            DebugSerial->println();
        }
    }
    else
    {
        eeprom.ramcopy.EnableHillPhysics = false;
        if (DEBUG) DebugSerial->println(F("Hill Physics: Turned OFF"));
        // If barrel stabilization is also disabled, just quit using accelerometer completely
        // (If UseAccel = false, we won't bother taking readings, which are time-consuming)
        if (eeprom.ramcopy.EnableBarrelStabilize == false)
        {
            UseIMU = false;
        }
    }    
}

void ToggleHillPhysics()
{
    EnableHillPhysics(!eeprom.ramcopy.EnableHillPhysics);
}

