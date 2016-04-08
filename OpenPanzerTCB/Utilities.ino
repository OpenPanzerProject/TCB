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
    DebugSerial->println(F("-----------------------------------------------------"));
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

float Convert_mS_to_Sec(int mS)
{
    return float(mS) / 1000.0;
}

void DumpSysInfo()
{
    DumpVersion();
    DumpRadioInfo();
    DumpTankInfo();
    DumpBattleInfo();
    DumpDriveSettings();
    DumpIMUInfo();
    DumpFunctionTriggers();
    DumpVoltage();
    DumpBaudRates();
    DebugSerial->println();
    PrintDebugLine();
}

void DumpVersion()
{
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->print(F("FIRMWARE VERSION: "));
    String str = FIRMWARE_VERSION;
    DebugSerial->println(str);
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

void DumpTankInfo()
{
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("MECHANICAL INFO"));
    PrintDebugLine();
    DebugSerial->print(F("Drive Motors:      ")); DebugSerial->println(ptrDriveType(eeprom.ramcopy.DriveMotors)); 
    DebugSerial->print(F("Turret Rotation:   ")); DebugSerial->print(ptrDriveType(eeprom.ramcopy.TurretRotationMotor)); 
    switch (eeprom.ramcopy.TurretRotationMotor) {
        case ONBOARD: DebugSerial->println(F(" (A)")); break;
        case SABERTOOTH: DebugSerial->println(F(" (M1)")); break;
        case POLOLU: DebugSerial->println(F(" (M0)")); break;
        default: DebugSerial->println(); }
    DebugSerial->print(F("Barrel Elevation:  ")); DebugSerial->print(ptrDriveType(eeprom.ramcopy.TurretElevationMotor)); 
    switch (eeprom.ramcopy.TurretElevationMotor) {
        case ONBOARD: DebugSerial->println(F(" (B)")); break;
        case SABERTOOTH: DebugSerial->println(F(" (M2)")); break;
        case POLOLU: DebugSerial->println(F(" (M1)")); break;
        default: DebugSerial->println(); }
    DebugSerial->print(F("Mechanical Barrel: ")); if (eeprom.ramcopy.Airsoft) DebugSerial->print(F("Airsoft")); else DebugSerial->print(F("Mechanical recoil")); 
    DebugSerial->println();
}

void DumpDriveSettings()
{
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("DRIVE SETTINGS"));
    PrintDebugLine();
    DebugSerial->print(F("Accel Ramp Enabled:     ")); PrintYesNo(eeprom.ramcopy.AccelRampEnabled);
    if (eeprom.ramcopy.AccelRampEnabled) 
    { 
        DebugSerial->print(F(" (Level: ")); 
        DebugSerial->print(eeprom.ramcopy.AccelSkipNum); 
        if (eeprom.ramcopy.AccelSkipNum != Driver.getAccelRampFrequency())
        {
            DebugSerial->print(F(" (Adjusted to "));
            DebugSerial->print(Driver.getAccelRampFrequency());
            DebugSerial->print(F(")")); 
        }
        DebugSerial->print(F(", Preset: ")); 
        DebugSerial->print(eeprom.ramcopy.AccelPreset, DEC);
        DebugSerial->println(F(")"));
    }
    DebugSerial->print(F("Decel Ramp Enabled:     ")); PrintYesNo(eeprom.ramcopy.DecelRampEnabled);
    if (eeprom.ramcopy.DecelRampEnabled) 
    { 
        DebugSerial->print(F(" (Level: ")); 
        DebugSerial->print(eeprom.ramcopy.DecelSkipNum); 
        if (eeprom.ramcopy.DecelSkipNum != Driver.getDecelRampFrequency())
        {
            DebugSerial->print(F(" (Adjusted to "));
            DebugSerial->print(Driver.getDecelRampFrequency());
            DebugSerial->print(F(")")); 
        }
        DebugSerial->print(F(", Preset: ")); 
        DebugSerial->print(eeprom.ramcopy.DecelPreset, DEC);
        DebugSerial->println(F(")"));
    }
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
    DebugSerial->print(F("Reverse Speed Limited:  ")); 
    if (eeprom.ramcopy.MaxReverseSpeedPct < 100) { DebugSerial->print(F("Yes - ")); DebugSerial->print(eeprom.ramcopy.MaxReverseSpeedPct); DebugSerial->println(F("%")); }
    else PrintLnYesNo(false);
    DebugSerial->print(F("Shift time:             ")); DebugSerial->print(Convert_mS_to_Sec(eeprom.ramcopy.TimeToShift_mS),1); DebugSerial->println(F(" sec"));
    DebugSerial->print(F("Engine pause time:      ")); DebugSerial->print(Convert_mS_to_Sec(eeprom.ramcopy.EnginePauseTime_mS),1); DebugSerial->println(F(" sec"));
    DebugSerial->print(F("Transmission delay:     ")); DebugSerial->print(Convert_mS_to_Sec(eeprom.ramcopy.TransmissionDelay_mS),1); DebugSerial->println(F(" sec"));
    DebugSerial->print(F("Neutral turn allowed:   ")); PrintYesNo(eeprom.ramcopy.NeutralTurnAllowed);
    if (eeprom.ramcopy.NeutralTurnAllowed) DebugSerial->print(F(" - ")); DebugSerial->print(eeprom.ramcopy.NeutralTurnPct); DebugSerial->print(F("%"));
    DebugSerial->println();
    DebugSerial->print(F("Turn mode:              ")); DebugSerial->println(Driver.getTurnMode());  
    DebugSerial->print(F("Turret stick functions: ")); PrintLnYesNo(Radio.UsingSpecialPositions);
    if (Radio.UsingSpecialPositions)
    {
        DebugSerial->print(F("Turret movement delay:  ")); DebugSerial->print(Convert_mS_to_Sec(eeprom.ramcopy.IgnoreTurretDelay_mS),2); DebugSerial->println(F(" sec"));    
    }
    DebugSerial->print(F("Recoil delay:           ")); 
    if (eeprom.ramcopy.RecoilDelay > 0)
    {
        DebugSerial->println(Convert_mS_to_Sec(eeprom.ramcopy.RecoilDelay),2); DebugSerial->println(F(" sec"));
    }
    else { PrintLnYesNo(0); }    
}

void DumpIMUInfo()
{
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("INERTIAL MEASUREMENT UNIT"));
    PrintDebugLine();
    if (IMU_Present)
    {
        DebugSerial->print(F("Sensor detected:              Yes")); DebugSerial->println();
        DebugSerial->print(F("Barrel Stabilization Enabled: ")); PrintYesNo(eeprom.ramcopy.EnableBarrelStabilize);
        if (eeprom.ramcopy.EnableBarrelStabilize) PrintBarrelSensitivity(); 
        DebugSerial->println();
        DebugSerial->print(F("Hill Physics Enabled:         ")); PrintYesNo(eeprom.ramcopy.EnableHillPhysics);
        if (eeprom.ramcopy.EnableHillPhysics) PrintHillSensitivity();
        DebugSerial->println();
    }
    else
    {
        DebugSerial->println(F("Sensor not detected!")); 
    }
}

void PrintBarrelSensitivity()
{
    DebugSerial->print(F(" (Sensitivity ")); 
    DebugSerial->print(eeprom.ramcopy.BarrelSensitivity); 
    if (eeprom.ramcopy.BarrelSensitivity != BarrelSensitivity)
    {
        DebugSerial->print(F(" - Adjusted to "));
        DebugSerial->print(BarrelSensitivity);
    }
    DebugSerial->print(F(")"));  
}

void PrintHillSensitivity()
{
    DebugSerial->print(F(" (Sensitivity ")); 
    DebugSerial->print(eeprom.ramcopy.HillSensitivity); 
    if (eeprom.ramcopy.HillSensitivity != HillSensitivity)
    {
        DebugSerial->print(F(" - Adjusted to "));
        DebugSerial->print(HillSensitivity);
    }
    DebugSerial->print(F(")"));        
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
        DebugSerial->print(F("Current Voltage:  "));
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
    
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("FUNCTION TRIGGERS"));
    PrintDebugLine();
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
                if (eeprom.ramcopy.SF_Trigger[i].specialFunction < 10) PrintSpace();
                PrintSpaceDash();
                strcpy_P(buffer, (char*)pgm_read_word_far(&(function_name_table[eeprom.ramcopy.SF_Trigger[i].specialFunction])));
                DebugSerial->print(buffer);
                DebugSerial->println();
            }
        }
    }
    else
    {
        DebugSerial->println(F("No function triggers defined"));
    }
}

