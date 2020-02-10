// mapf - Float version of the "map" function
float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// We may temporarily want to turn off debug messages, but restore the setting to whatever it originally was
void DisableDebug()
{
    DEBUG = false;
}
void RestoreDebug()
{
    DEBUG = SAVE_DEBUG;
}

void PrintDebugLine()
{
    for (uint8_t i=0; i<45; i++) { DebugSerial->print(F("-")); }
    DebugSerial->println(); 
    DebugSerial->flush();   // This causes a pause until the serial transmission is complete
}

void PrintSpaceDash()
{
    DebugSerial->print(F(" - "));
}

void PrintSpaceBar()
{
    DebugSerial->print(F(" | "));
}

void PrintSpace()
{
    DebugSerial->print(F(" "));
}    

void PrintSpaces(uint8_t num)
{
    if (num == 0) return;
    for (uint8_t i=0; i<num; i++) { PrintSpace(); }
}

void PrintLine()
{
    DebugSerial->println();
}

void PrintLines(uint8_t num)
{    
    if (num == 0) return;
    for (uint8_t i=0; i<num; i++) { PrintLine(); }
}

void PrintTrueFalse(boolean boolVal)
{
    if (boolVal == true) { DebugSerial->print(F("TRUE")); } else { DebugSerial->print(F("FALSE")); }
}

void PrintLnTrueFalse(boolean boolVal)
{
    PrintTrueFalse(boolVal);
    DebugSerial->println();
}

void PrintYesNo(boolean boolVal)
{
    if (boolVal == true) { DebugSerial->print(F("Yes")); } else { DebugSerial->print(F("No")); }
}

void PrintLnYesNo(boolean boolVal)
{
    PrintYesNo(boolVal);
    DebugSerial->println();
}

void PrintHighLow(boolean boolVal)
{
    if (boolVal == true) { DebugSerial->println(F("HIGH")); } else { DebugSerial->println(F("LOW")); }
}

void PrintPct(uint8_t pct)
{
    DebugSerial->print(pct);
    DebugSerial->print(F("%"));
}
void PrintLnPct(uint8_t pct)
{
    PrintPct(pct);
    DebugSerial->println();
}
void PrintWaitingForRadio()
{   // Only print this message if we are still waiting for the radio to be plugged in.
    if (SAVE_DEBUG && Radio.Status() != READY_state) { DebugSerial->println(F("Waiting for radio... ")); }
}

float Convert_mS_to_Sec(int mS)
{
    return float(mS) / 1000.0;
}

void DumpSysInfo()
{
    // All this printing can take some time, so we sprinkle some per-loop-updates throughout. 
    // I also added some flush commands, which pauses until all serial data has left the transmit buffer. 
    // As it turns out, all the garbled serial only happens when using the Arduino IDE serial monitor, or just
    // if I have the IDE open even if I am using some other program (such as Snoop in OP Config). 
    // Once the Arduino IDE is closed, then I get perfectly smooth serial. Not sure what the deal is with the IDE. 
    // The PerLoopUpdates() are good to leave in, and the serial flush commands are probably unnecessary, but
    // don't hurt anything, so I leave them too. 

    // This whole dump takes about 1/3 second. It is likely to cause a brief radio failsafe event, which will correct itself immediately after. 
    
    DumpVersion();
        PerLoopUpdates();
        DebugSerial->flush();
    DumpRadioInfo();
        PerLoopUpdates();
        DebugSerial->flush();
    DumpMotorInfo();
        PerLoopUpdates();
        DebugSerial->flush();    
    DumpDriveSettings();
        PerLoopUpdates();
        DebugSerial->flush();    
    DumpTurretInfo();
        PerLoopUpdates();
        DebugSerial->flush();    
    DumpSmokerInfo();
        PerLoopUpdates();
        DebugSerial->flush();
    DumpBattleInfo();
        PerLoopUpdates();
        DebugSerial->flush();
    DumpSoundInfo();
        PerLoopUpdates();
        DebugSerial->flush();        
    DumpFunctionTriggers();
        PerLoopUpdates();
        DebugSerial->flush();    
    DumpVoltage();
        PerLoopUpdates();
        DebugSerial->flush();    
    DumpBaudRates();
        PerLoopUpdates();
        DebugSerial->flush();    
    DebugSerial->println();
    PrintDebugLine();    
}

void DumpVersion()
{
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->print(F("FIRMWARE VERSION: "));
    String str = FIRMWARE_VERSION;
    DebugSerial->print(str);
    DebugSerial->print(" (EEPROM Size: ");
    DebugSerial->print(sizeof(_eeprom_data));
    DebugSerial->println(")");    
}

void DumpRadioInfo()
{
    DumpStickInfo();
    DumpAuxChannelsInfo();
    DumpChannelsDetectedUtilized();
}

void DumpStickInfo()
{
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->print(F("RADIO INFO - ")); DebugSerial->println(RadioProtocol(Radio.getProtocol()));
    PrintDebugLine();
    DebugSerial->println(F("Stick       Min    Center   Max    Deadband  Reversed"));
    PrintDebugLine();
    DebugSerial->print(F("Throttle   ")); if (Radio.Sticks.Throttle.Settings->pulseMin < 1000) { PrintSpace(); } DebugSerial->print(Radio.Sticks.Throttle.Settings->pulseMin); PrintSpaces(4); DebugSerial->print(Radio.Sticks.Throttle.Settings->pulseCenter); PrintSpaces(5); DebugSerial->print(Radio.Sticks.Throttle.Settings->pulseMax); PrintSpaces(4); DebugSerial->print(Radio.Sticks.Throttle.Settings->deadband); PrintSpaces(8); PrintLnTrueFalse(Radio.Sticks.Throttle.Settings->reversed); 
    DebugSerial->print(F("Turn       ")); if (Radio.Sticks.Turn.Settings->pulseMin < 1000) { PrintSpace(); } DebugSerial->print(Radio.Sticks.Turn.Settings->pulseMin); PrintSpaces(4); DebugSerial->print(Radio.Sticks.Turn.Settings->pulseCenter); PrintSpaces(5); DebugSerial->print(Radio.Sticks.Turn.Settings->pulseMax); PrintSpaces(4); DebugSerial->print(Radio.Sticks.Turn.Settings->deadband); PrintSpaces(8); PrintLnTrueFalse(Radio.Sticks.Turn.Settings->reversed); 
    DebugSerial->print(F("Elevation  ")); if (Radio.Sticks.Elevation.Settings->pulseMin < 1000) { PrintSpace(); } DebugSerial->print(Radio.Sticks.Elevation.Settings->pulseMin); PrintSpaces(4); DebugSerial->print(Radio.Sticks.Elevation.Settings->pulseCenter); PrintSpaces(5); DebugSerial->print(Radio.Sticks.Elevation.Settings->pulseMax); PrintSpaces(4); DebugSerial->print(Radio.Sticks.Elevation.Settings->deadband); PrintSpaces(8); PrintLnTrueFalse(Radio.Sticks.Elevation.Settings->reversed); 
    DebugSerial->print(F("Azimuth    ")); if (Radio.Sticks.Azimuth.Settings->pulseMin < 1000) { PrintSpace(); } DebugSerial->print(Radio.Sticks.Azimuth.Settings->pulseMin); PrintSpaces(4); DebugSerial->print(Radio.Sticks.Azimuth.Settings->pulseCenter); PrintSpaces(5); DebugSerial->print(Radio.Sticks.Azimuth.Settings->pulseMax); PrintSpaces(4); DebugSerial->print(Radio.Sticks.Azimuth.Settings->deadband); PrintSpaces(8); PrintLnTrueFalse(Radio.Sticks.Azimuth.Settings->reversed); 

    DebugSerial->println();
}
void DumpAuxChannelsInfo()
{
    uint8_t chanCount = Radio.getChannelCount(); 
    
    DebugSerial->println(F("Aux Chan.   Min     Max     Type"));
    PrintDebugLine();
    for (int a=0; a<AUXCHANNELS; a++)
    {
        DebugSerial->print(a+1);             
        a < 9 ? PrintSpaces(11) : PrintSpaces(10);
        if (Radio.AuxChannel[a].present)
        {   
             DebugSerial->print(Radio.AuxChannel[a].Settings->pulseMin); if (Radio.AuxChannel[a].Settings->pulseMin < 1000) { PrintSpace(); } PrintSpaces(4); DebugSerial->print(Radio.AuxChannel[a].Settings->pulseMax); PrintSpaces(4); if (Radio.AuxChannel[a].Settings->Digital) { DebugSerial->println(F("Digital")); } else {DebugSerial->println(F("Analog")); } 
        }
        else
        {   // If a is less than total possible channel count, but it's flagged as not present, it's because the user has chosen not to use it.
            // If a is greater than total possible channel count, the reason reasin it's flagged as not present is because the radio doesn't have that channel
            if (a < chanCount) {    DebugSerial->println(F("IGNORED"));      }
            else               {    DebugSerial->println(F("NOT DETECTED")); }
        }
    }
    PrintDebugLine();
}

void DumpChannelsDetectedUtilized()
{
    DebugSerial->print(F("Channels detected: ")); DebugSerial->println(Radio.getChannelCount());
    DebugSerial->print(F("Channels utilized: ")); DebugSerial->println(Radio.ChannelsUtilized);
}

void DumpMotorInfo()
{
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("MOTOR TYPES"));
    PrintDebugLine();
    DebugSerial->print(F("Drive Motors:      ")); DebugSerial->print(ptrDriveType(eeprom.ramcopy.DriveMotors)); 
    if (eeprom.ramcopy.DriveMotors == OP_SCOUT)
    {
        DebugSerial->print(F(" (Current Limit: "));
        DebugSerial->print(eeprom.ramcopy.ScoutCurrentLimit);
        DebugSerial->print(F(" Amps)"));
    }
    DebugSerial->println();
    DebugSerial->print(F("Turret Rotation:   ")); DebugSerial->print(ptrDriveType(eeprom.ramcopy.TurretRotationMotor)); 
    switch (eeprom.ramcopy.TurretRotationMotor) {
        case ONBOARD: DebugSerial->println(F(" (A)")); break;
        case OP_SCOUT:   // Fall through
        case SABERTOOTH: DebugSerial->println(F(" (M1)")); break;
        case POLOLU: DebugSerial->println(F(" (M0)")); break;
        default: DebugSerial->println(); }
    DebugSerial->print(F("Barrel Elevation:  ")); DebugSerial->print(ptrDriveType(eeprom.ramcopy.TurretElevationMotor)); 
    switch (eeprom.ramcopy.TurretElevationMotor) {
        case ONBOARD: DebugSerial->println(F(" (B)")); break;
        case OP_SCOUT:   // Fall through
        case SABERTOOTH: DebugSerial->println(F(" (M2)")); break;
        case POLOLU: DebugSerial->println(F(" (M1)")); break;
        default: DebugSerial->println(); }
    DebugSerial->print(F("Mechanical Barrel: ")); if (eeprom.ramcopy.Airsoft) DebugSerial->print(F("Airsoft")); else DebugSerial->print(F("Mechanical recoil")); 
    if (Tank.isMechBarrelSetWithCannon()) DebugSerial->print(F(" (Enabled)")); else DebugSerial->print(F(" (Disabled)"));
    DebugSerial->println();
}

void DumpDriveSettings()
{
boolean Profile_1; 
boolean AccelRampEnabled;
boolean DecelRampEnabled;

if (DrivingProfile == 2) Profile_1 = false;
else                     Profile_1 = true; 
Profile_1 ? AccelRampEnabled = eeprom.ramcopy.AccelRampEnabled_1 : AccelRampEnabled = eeprom.ramcopy.AccelRampEnabled_2; 
Profile_1 ? DecelRampEnabled = eeprom.ramcopy.DecelRampEnabled_1 : DecelRampEnabled = eeprom.ramcopy.DecelRampEnabled_2; 
    
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("DRIVE SETTINGS"));
    PrintDebugLine();
    DebugSerial->print(F("Vehicle Type:           ")); DebugSerial->println(printDriveType(eeprom.ramcopy.DriveType));
    DebugSerial->print(F("Active Driving Profile: ")); DebugSerial->println(DrivingProfile);
    DebugSerial->print(F("Accel Ramp Enabled:     ")); PrintYesNo(AccelRampEnabled); 
    if (AccelRampEnabled) 
    { 
        DebugSerial->print(F(" (Level: ")); 
        Profile_1 ? DebugSerial->print(eeprom.ramcopy.AccelSkipNum_1) : DebugSerial->print(eeprom.ramcopy.AccelSkipNum_2); 
        if ((Profile_1 == true && eeprom.ramcopy.AccelSkipNum_1 != Driver.getAccelRampFrequency()) || (Profile_1 == false && eeprom.ramcopy.AccelSkipNum_2 != Driver.getAccelRampFrequency()))
        {
            DebugSerial->print(F(" (Adjusted to "));
            DebugSerial->print(Driver.getAccelRampFrequency());
            DebugSerial->print(F(")")); 
        }
        DebugSerial->print(F(", Preset: ")); 
        if (Profile_1)
        {
            if (eeprom.ramcopy.AccelPreset_1 == 0) DebugSerial->print(F("None"));
            else DebugSerial->print(eeprom.ramcopy.AccelPreset_1, DEC);
        }
        else
        {
            if (eeprom.ramcopy.AccelPreset_2 == 0) DebugSerial->print(F("None"));
            else DebugSerial->print(eeprom.ramcopy.AccelPreset_2, DEC);
        }
        DebugSerial->print(F(")"));
    }
    DebugSerial->println();
    DebugSerial->print(F("Decel Ramp Enabled:     ")); PrintYesNo(DecelRampEnabled); 
    if (DecelRampEnabled) 
    { 
        DebugSerial->print(F(" (Level: ")); 
        Profile_1 ? DebugSerial->print(eeprom.ramcopy.DecelSkipNum_1) : DebugSerial->print(eeprom.ramcopy.DecelSkipNum_2); 
        if ((Profile_1 == true && eeprom.ramcopy.DecelSkipNum_1 != Driver.getDecelRampFrequency()) || (Profile_1 == false && eeprom.ramcopy.DecelSkipNum_2 != Driver.getDecelRampFrequency()))
        {
            DebugSerial->print(F(" (Adjusted to "));
            DebugSerial->print(Driver.getDecelRampFrequency());
            DebugSerial->print(F(")")); 
        }
        DebugSerial->print(F(", Preset: ")); 
        if (Profile_1)
        {
            if   (eeprom.ramcopy.DecelPreset_1 == 0) DebugSerial->print(F("None"));
            else DebugSerial->print(eeprom.ramcopy.DecelPreset_1, DEC);
        }
        else
        {
            if   (eeprom.ramcopy.DecelPreset_2 == 0) DebugSerial->print(F("None"));
            else DebugSerial->print(eeprom.ramcopy.DecelPreset_2, DEC);    
        }
        
        DebugSerial->print(F(")"));
    }
    DebugSerial->println();
    DebugSerial->print(F("Motor Nudge Enabled:    ")); 
    if (eeprom.ramcopy.MotorNudgePct == 0) PrintLnYesNo(false);
    else
    {   PrintYesNo(true);
        DebugSerial->print(F(" (")); 
        DebugSerial->print(eeprom.ramcopy.MotorNudgePct); 
        DebugSerial->print(F("% throttle for ")); 
        DebugSerial->print(eeprom.ramcopy.NudgeTime_mS);
        DebugSerial->println(F(" ms)"));
    }
    DebugSerial->print(F("Forward Speed Limited:  ")); 
    if (eeprom.ramcopy.MaxForwardSpeedPct < 100) { DebugSerial->print(F("Yes - ")); DebugSerial->print(eeprom.ramcopy.MaxForwardSpeedPct); DebugSerial->println(F("%")); }
    else PrintLnYesNo(false);
    DebugSerial->print(F("Reverse Speed Limited:  ")); 
    if (eeprom.ramcopy.MaxReverseSpeedPct < 100) { DebugSerial->print(F("Yes - ")); DebugSerial->print(eeprom.ramcopy.MaxReverseSpeedPct); DebugSerial->println(F("%")); }
    else PrintLnYesNo(false);
    DebugSerial->print(F("Transmission Type:      ")); ManualGear ? DebugSerial->println(F("Manual")) : DebugSerial->println(F("Automatic"));
    DebugSerial->print(F("Shift Time:             ")); DebugSerial->print(Convert_mS_to_Sec(eeprom.ramcopy.TimeToShift_mS),1); DebugSerial->println(F(" sec"));
    DebugSerial->print(F("Engine Pause Time:      ")); DebugSerial->print(Convert_mS_to_Sec(eeprom.ramcopy.EnginePauseTime_mS),1); DebugSerial->println(F(" sec"));
    DebugSerial->print(F("Transmission Delay:     ")); DebugSerial->print(Convert_mS_to_Sec(eeprom.ramcopy.TransmissionDelay_mS),1); DebugSerial->println(F(" sec"));
    DebugSerial->print(F("Neutral Turn Allowed:   "));  // Neutral turns only make sense with tanks
    if (eeprom.ramcopy.DriveType == DT_TANK || eeprom.ramcopy.DriveType == DT_DKLM) 
    {   
        PrintYesNo(eeprom.ramcopy.NeutralTurnAllowed);
        if (eeprom.ramcopy.NeutralTurnAllowed) { DebugSerial->print(F(" - ")); DebugSerial->print(eeprom.ramcopy.NeutralTurnPct); DebugSerial->print(F("%")); }
    }
    else if (eeprom.ramcopy.DriveType == DT_DMD) DebugSerial->print(F("Yes (Tamiya)"));  // Tamiya always allows neutral turns but it uses a different method to accomplish it
    else DebugSerial->print(F("N/A for vehicle type"));
    DebugSerial->println();   
    if (eeprom.ramcopy.DriveType == DT_HALFTRACK)
    {
        DebugSerial->print(F("Steering Pct to treads: "));
        DebugSerial->print(eeprom.ramcopy.HalftrackTreadTurnPct);
        DebugSerial->println(F("%"));
    }
    DebugSerial->print(F("Turn Mode:              "));  // Turn modes only apply to conventional tank drives (not clutch type) and halftracks
    if (eeprom.ramcopy.DriveType == DT_TANK || eeprom.ramcopy.DriveType == DT_HALFTRACK) DebugSerial->println(Driver.getTurnMode());
    else DebugSerial->println(F("N/A for vehicle type"));
    DebugSerial->print(F("Track Recoil Enabled:   "));  
    PrintYesNo(eeprom.ramcopy.EnableTrackRecoil);
    if (eeprom.ramcopy.EnableTrackRecoil) 
    { 
        DebugSerial->print(F(" (Kickback speed ")); DebugSerial->print(eeprom.ramcopy.TrackRecoilKickbackSpeed); 
        DebugSerial->print(F("%, Deceleration factor ")); DebugSerial->print(eeprom.ramcopy.TrackRecoilDecelerateFactor);
        DebugSerial->print(F(")"));
    }
    DebugSerial->println();        
}

void DumpTurretInfo()
{
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("TURRET SETTINGS"));
    PrintDebugLine();    
    DebugSerial->print(F("Turret Rotation Speed Limited:  ")); 
    if (eeprom.ramcopy.TurretRotation_MaxSpeedPct < 100) { DebugSerial->print(F("Yes - ")); DebugSerial->print(eeprom.ramcopy.TurretRotation_MaxSpeedPct); DebugSerial->println(F("%")); }
    else PrintLnYesNo(false);
    DebugSerial->print(F("Barrel Elevation Speed Limited: ")); 
    if (eeprom.ramcopy.TurretElevation_MaxSpeedPct < 100) { DebugSerial->print(F("Yes - ")); DebugSerial->print(eeprom.ramcopy.TurretElevation_MaxSpeedPct); DebugSerial->println(F("%")); }
    else PrintLnYesNo(false);
    DebugSerial->print(F("Recoil delay:                   ")); 
    if (eeprom.ramcopy.RecoilDelay > 0)
    {
        DebugSerial->println(Convert_mS_to_Sec(eeprom.ramcopy.RecoilDelay),2); DebugSerial->println(F(" sec"));
    }
    else { PrintLnYesNo(0); }    
    
    // Recoil servo preset - not presently used
    // DebugSerial->println(ptrRecoilPreset(eeprom.ramcopy.RecoilServo_PresetNum));
}

void DumpSmokerInfo()
{
    int f;
    int h;
    boolean i = false;  // Include heat settings
    
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("SMOKER"));
    PrintDebugLine();
    if (eeprom.ramcopy.SmokerControlAuto == false)
    {
        DebugSerial->println(F("Smoker output controlled manually"));
    }
    else
    {   
        DebugSerial->print(F("Smoker Type:          ")); DebugSerial->println(ptrSmokerType(eeprom.ramcopy.SmokerDeviceType)); 
        if (eeprom.ramcopy.SmokerDeviceType != SMOKERTYPE_ONBOARD_STANDARD) i = true;
        // Preheat and hot start
        DebugSerial->print(F("Pre-heat delay:       ")); 
        if (eeprom.ramcopy.SmokerPreHeat_Sec == 0) 
        { 
            DebugSerial->println(F("N/A")); 
        } 
        else 
        { 
            DebugSerial->print(eeprom.ramcopy.SmokerPreHeat_Sec); DebugSerial->print(F(" seconds")); 
            if (SmokerPreHeat_Sec == 0) DebugSerial->print(F(" (currently disabled)"));  // In this case the preheater has been disabled by the user
            Serial.println();
        }
/* Hidden for now because we are not using this feature, although it does work.
        if (eeprom.ramcopy.SmokerPreHeat_Sec > 0)
        {
            if (eeprom.ramcopy.HotStartTimeout_Sec > 0)
            {
                DebugSerial->print(F("Skip pre-heat delay if engine re-started within ")); 
                DebugSerial->print(eeprom.ramcopy.HotStartTimeout_Sec);
                DebugSerial->println(F(" seconds of last engine shutdown."));
            }
            else
            {
                DebugSerial->println(F("Hot start:            Disabled"));
            }
        }
*/
        // Idle
        f = (int)((((float)eeprom.ramcopy.SmokerIdleSpeed/(float)MOTOR_MAX_FWDSPEED)*100.0)+0.5);
        h = (int)((((float)eeprom.ramcopy.SmokerHeatIdleAmt/(float)MOTOR_MAX_FWDSPEED)*100.0)+0.5);
        DebugSerial->print(F("Fan idle speed:       ")); PrintPct(f); 
        if (f<100) { PrintSpace(); } if (f<10) { PrintSpace(); } PrintSpaces(3); if (i) { PrintHeatAmt(); PrintPct(h); } else { PrintHeat(); PrintSameHeatAsFan(); }
        PrintLine();
        // Fast idle
        f = (int)((((float)eeprom.ramcopy.SmokerFastIdleSpeed/(float)MOTOR_MAX_FWDSPEED)*100.0)+0.5);
        h = (int)((((float)eeprom.ramcopy.SmokerHeatFastIdleAmt/(float)MOTOR_MAX_FWDSPEED)*100.0)+0.5);
        DebugSerial->print(F("Fan fast idle speed:  ")); PrintPct(f); 
        if (f<100) { PrintSpace(); } if (f<10) { PrintSpace(); } PrintSpaces(3); if (i) { PrintHeatAmt(); PrintPct(h); } else { PrintHeat(); PrintSameHeatAsFan(); }
        PrintLine();
        // Max speed
        f = (int)((((float)eeprom.ramcopy.SmokerMaxSpeed/(float)MOTOR_MAX_FWDSPEED)*100.0)+0.5);
        h = (int)((((float)eeprom.ramcopy.SmokerHeatMaxAmt/(float)MOTOR_MAX_FWDSPEED)*100.0)+0.5);
        DebugSerial->print(F("Fan max speed:        ")); PrintPct(f); 
        if (f<100) { PrintSpace(); } if (f<10) { PrintSpace(); } PrintSpaces(3); if (i) { PrintHeatAmt(); PrintPct(h); } else { PrintHeat(); PrintSameHeatAsFan(); }
        PrintLine();            
        DebugSerial->print(F("Smoke when destroyed: ")); if (eeprom.ramcopy.SmokerDestroyedSpeed == 0) { DebugSerial->println(F("Disabled")); } else { DebugSerial->print((int)((((float)eeprom.ramcopy.SmokerDestroyedSpeed/(float)MOTOR_MAX_FWDSPEED)*100.0)+0.5)); DebugSerial->println(F("%")); }
    }   
}
void PrintHeatAmt()
{
    DebugSerial->print(F("Heat Amt: "));
}
void PrintHeat()
{
    DebugSerial->print(F("Heat: "));
}
void PrintSameHeatAsFan()
{ 
    DebugSerial->print(F("Same as fan"));
}

void DumpSoundInfo()
{
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("SOUND CARD"));
    PrintDebugLine();    
    DebugSerial->print(F("Sound card: ")); 
    DebugSerial->println(printSoundDevice(eeprom.ramcopy.SoundDevice));
/* 
    //Actually we are not dumping the rest of the sound-card specific settings, so either do all of them or none. For now we will do none.     
    if (eeprom.ramcopy.SoundDevice == SD_OP_SOUND_CARD)
    {
        PrintSoundBank1();
        DebugSerial->print(F("A"));
        PrintSoundBank2();
        PrintLnYesNo(eeprom.ramcopy.SoundBankA_Loop);
        PrintSoundBank1();
        DebugSerial->print(F("B"));
        PrintSoundBank2();
        PrintLnYesNo(eeprom.ramcopy.SoundBankB_Loop);
    }
*/
}

void PrintSoundBank1()
{
    DebugSerial->print(F("Sound Bank "));
}

void PrintSoundBank2()
{
    DebugSerial->print(F(" Auto-Loop: "));
}

void DumpBattleInfo()
{
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("BATTLE INFO"));
    PrintDebugLine();
    if (Tank.BattleSettings.IR_FireProtocol != IR_UNKNOWN)
    {
        DebugSerial->print(F("Is Repair Tank?   ")); PrintLnYesNo(Tank.isRepairTank());
        if (Tank.isRepairTank()) { DebugSerial->print(F("Fire Protocol:    ")); DebugSerial->println(ptrIRName(Tank.BattleSettings.IR_RepairProtocol)); }
        else
        {
            DebugSerial->print(F("Fire Protocol:    ")); DebugSerial->print(ptrIRName(Tank.BattleSettings.IR_FireProtocol));
            if (Tank.BattleSettings.IR_Team != IR_TEAM_NONE) { DebugSerial->print(F(" (Team ")); DebugSerial->print(ptrIRTeam(Tank.BattleSettings.IR_Team)); DebugSerial->print(F(")")); } DebugSerial->println();
        }
        DebugSerial->print(F("Hit Protocol 2:   ")); 
        if (Tank.BattleSettings.IR_HitProtocol_2 != IR_UNKNOWN ) { DebugSerial->println(ptrIRName(Tank.BattleSettings.IR_HitProtocol_2)); } else { DebugSerial->println(F("N/A")); }
        DebugSerial->print(F("Repaired by:      ")); 
        if (Tank.BattleSettings.IR_RepairProtocol != IR_UNKNOWN) { DebugSerial->println(ptrIRName(Tank.BattleSettings.IR_RepairProtocol)); } else { DebugSerial->println(F("N/A")); }
        DebugSerial->print(F("Send MG IR Code:  ")); 
        if (Tank.BattleSettings.Use_MG_Protocol) { DebugSerial->print(F("Yes (")); DebugSerial->print(ptrIRName(Tank.BattleSettings.IR_MGProtocol)); DebugSerial->println(")"); }
        else PrintLnYesNo(false);
        DebugSerial->print(F("Accept MG Damage: ")); 
        if (Tank.BattleSettings.Accept_MG_Damage) { DebugSerial->print(F("Yes (")); DebugSerial->print(ptrIRName(Tank.BattleSettings.IR_MGProtocol)); DebugSerial->println(")"); }
        else PrintLnYesNo(false);
    
        DebugSerial->print(F("Damage Profile:   ")); DebugSerial->println(ptrDamageProfile(Tank.BattleSettings.DamageProfile));
        DebugSerial->print(F("Weight Class:     ")); DebugSerial->println(ptrWeightClassName(Tank.BattleSettings.WeightClass)); 
        DebugSerial->print(F("(")); DebugSerial->print(Tank.BattleSettings.ClassSettings.maxHits); DebugSerial->print(F(" cannon hits, ")); if (Tank.BattleSettings.WeightClass == WC_CUSTOM) { DebugSerial->print(Tank.BattleSettings.ClassSettings.maxMGHits); DebugSerial->print(F(" MG hits, ")); } DebugSerial->print(Convert_mS_to_Sec(Tank.BattleSettings.ClassSettings.reloadTime),1); DebugSerial->print(F(" sec reload, ")); DebugSerial->print(Convert_mS_to_Sec(Tank.BattleSettings.ClassSettings.recoveryTime),1); DebugSerial->println(F(" sec recovery)"));    
    }
    else
    {
        DebugSerial->println(F("IR & Tank Battling Disabled"));
    }
}


void DumpBaudRates()
{
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("BAUD RATES"));
    PrintDebugLine();
    DebugSerial->print(F("USB Serial Baud:   ")); DebugSerial->println(eeprom.ramcopy.USBSerialBaud);
    DebugSerial->print(F("Motor Serial Baud: ")); DebugSerial->println(eeprom.ramcopy.MotorSerialBaud);
    DebugSerial->print(F("Aux Serial Baud:   ")); DebugSerial->println(eeprom.ramcopy.AuxSerialBaud);
    DebugSerial->print(F("Serial 3 Tx Baud:  ")); DebugSerial->println(eeprom.ramcopy.Serial3TxBaud);
}

void DumpVoltage()
{
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("BATTERY"));
    PrintDebugLine();
    DebugSerial->print(F("Battery Detected: "));
    if (IsBatteryUnplugged())
    {
        DebugSerial->println(F("No"));
    }
    else
    {
        DebugSerial->println(F("Yes"));
        DebugSerial->print(F("Voltage:          "));
        DebugSerial->print(ReadVoltage(),2);
        DebugSerial->println(F("v"));
    }
    DebugSerial->print(F("LVC Enabled:      "));
    PrintYesNo(eeprom.ramcopy.LVC_Enabled);
    if (eeprom.ramcopy.LVC_Enabled)
    {
        DebugSerial->print(F(" ("));
        DumpLVC_Voltage();
        DebugSerial->println(F("v cutoff)"));
    }
    else DebugSerial->println();
}

void DumpLVC_Voltage()
{
    float cutoff = float(eeprom.ramcopy.LVC_Cutoff_mV)/1000.0;
    DebugSerial->print(cutoff, 1);  // Print with one decimal precision
}

void DumpFunctionTriggers()
{
    char buffer[50];
    uint32_t fnameTableAddress = pgm_get_far_address(_FunctionNames_);  // This is the starting address of our function name table in far progmem. 
    
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("FUNCTION TRIGGERS"));
    PrintDebugLine();

    // Let the user know if there are any functions assigned to turret stick triggers
    DebugSerial->print(F("Turret stick functions: ")); if (Radio.UsingSpecialPositions) { DebugSerial->println(F("Yes")); } else { DebugSerial->println(F("None")); }
    if (Radio.UsingSpecialPositions)
    {
        DebugSerial->print(F("Turret movement delay:  ")); DebugSerial->print(Convert_mS_to_Sec(eeprom.ramcopy.IgnoreTurretDelay_mS),2); DebugSerial->println(F(" sec"));    
    } 

    // Now list all functions and their triggers
    if (MAX_FUNCTION_TRIGGERS > 0)
    {
        for (uint8_t i = 0; i <MAX_FUNCTION_TRIGGERS; i++)
        {   // A valid function-trigger will have a function number and a TriggerID > 0
            if (eeprom.ramcopy.SF_Trigger[i].specialFunction != SF_NULL_FUNCTION && eeprom.ramcopy.SF_Trigger[i].TriggerID > 0)
            {   
                PrintTriggerDescription(eeprom.ramcopy.SF_Trigger[i].specialFunction, eeprom.ramcopy.SF_Trigger[i].TriggerID);
                DebugSerial->print(F(" -> "));
                DebugSerial->print(F("Function #"));
                DebugSerial->print(eeprom.ramcopy.SF_Trigger[i].specialFunction);
                if (eeprom.ramcopy.SF_Trigger[i].specialFunction < 10) PrintSpaces(2);
                else if (eeprom.ramcopy.SF_Trigger[i].specialFunction < 100) PrintSpace();
                PrintSpaceDash();
                // This worked back in those innocent days when we didn't have progmem in far memory. 
                //strcpy_P(buffer, (char*)pgm_read_byte_far(&(function_name_table[eeprom.ramcopy.SF_Trigger[i].specialFunction])));
                // This line cost me a week of my life. The custom strcpy_PFAR is in OP_Settings.h. The function names are set in OP_FunctionsTriggers.h 
                strcpy_PFAR(buffer, fnameTableAddress, eeprom.ramcopy.SF_Trigger[i].specialFunction*FUNCNAME_CHARS);
                DebugSerial->println(buffer);
                DebugSerial->flush();  
            }
        }
    }
    else
    {
        DebugSerial->println(F("No function triggers defined"));
    }
}

void DumpVarInfo()
{
    
    uint16_t varID;         // Variable ID
    uint16_t varOffset;     // Offset of this variable within the _eeprom_data structure
    _vartype varType;       // Type - signed or unsigned int 8, 16 or 32, or boolean
    for (int i=0; i<NUM_STORED_VARS; i++)
    {   
        varID = pgm_read_word_far(pgm_get_far_address(STORAGEVARS) + (i*5));
        varOffset = pgm_read_word_far(pgm_get_far_address(STORAGEVARS) + (i*5) + 2);
        varType = pgm_read_byte_far(pgm_get_far_address(STORAGEVARS) + (i*5) + 4);
        DebugSerial->print(F("ID: ")); DebugSerial->print(varID);
        DebugSerial->print(F(" Offset: ")); DebugSerial->print(varOffset);
        DebugSerial->print(F(" Type: ")); DebugSerial->print(printVarType(varType)); 
        DebugSerial->println();
        delay(5);
    }
}




