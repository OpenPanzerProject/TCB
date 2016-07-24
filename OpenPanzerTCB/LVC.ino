
// LOW VOLTAGE CUTOFF 
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void CheckVoltage(void)
{
    // LOW VOLTAGE CUTOFF & BATTERY UNPLUGGED
    // ------------------------------------------------------------------------------------------------------------------------------------------------>  
    // The program starts with BatteryUnplugged set to true. We only do this check until the battery is first detected. Once it is, we don't check again. 
    // Even if the user later unplugs the battery, it will be considered an LVC condition. 
    if (BatteryUnplugged)
    {
        if (!IsBatteryUnplugged())
        {
            // Ok, battery has been detected
            BatteryUnplugged = false;
            HavePower = true;   // True for now, we will do an LVC check next to make sure the battery has enough voltage
        }
    }
    else
    {   // In this case, we know the battery is (or was) plugged in. 
        // Now check the voltage level
        if (BatteryBelowCutoff() && !LVC)       // The battery has just dropped below the cutoff level. 
        {
            LVC = true;
            HavePower = false;
            if (DEBUG) 
            {
                DebugSerial->print(F("LVC threshold reached! ("));
                DumpLVC_Voltage();
                DebugSerial->println(F("v)"));
            }
            StopEverything();
            LVC_BlinkHandler();                 // Start the special blinker to indicate LVC
        }
        else if (LVC && !BatteryBelowCutoff())
        {   // In this case we were in LVC, but now the battery has gone back up above the cutoff
            // Restore power
            LVC = false;
            HavePower = true;
            if (DEBUG) DebugSerial->println(F("LVC Exit - Voltage Restored"));
        }
    }
}

void EnableRoutineVoltageCheck(void)
{
    LVC_TimerID = timer.setInterval(2000, CheckVoltage);
}

void DisableRoutineVoltageCheck(void)
{
    if (timer.isEnabled(LVC_TimerID)) timer.deleteTimer(LVC_TimerID);
}

boolean IsBatteryUnplugged(void)
{
    // If the voltage reading is really low, the issue probably isn't a dead battery, but more likely the
    // user is running from USB power alone. That is fine, but we still want to know about it. 
    if (ReadVoltage() < 2.0)    // We consider anything below 2 volts to be unplugged
    {
        return true;
    }
    else
    {
        return false;
    }
}

boolean BatteryBelowCutoff(void)
{
uint16_t milliVolts;

    // Read voltage, convert it to millivolts since that's what we're actually going to use to compare to the cutoff
    milliVolts = static_cast<uint16_t>(ReadVoltage() * 1000.0);

    // Check voltage against cutoff. 
    if (milliVolts < eeprom.ramcopy.LVC_Cutoff_mV)
    {
        return true;
    }
    else 
    {   
        return false;
    }
}

float ReadVoltage(void)
{
int voltSense;                  // What we read on the analog pin
float unFilteredVoltage;        // Converted to voltage on pin
static float filteredVoltage;   // Voltage on pin after being run through a simple low-pass filter
float Voltage;                  // Pin voltage converted to battery voltage
static boolean firstPass = true;       

// We are using this voltage divider:
// 
// GND |-----/\/\/\-----------------/\/\/\-------> +V Batt
//            4.7k         |         10k
//                         |
//                      Measure

// The voltage we measure on the pin isn't the actual outside voltage of the battery, it is the battery voltage divided by some number
//multiplier = (4.7 + 10) / 4.7 = 3.1277
const float multiplier = 3.1277;    // Multiply this by our measured voltage and we will have battery voltage

// But wait! Our input polarity protection diode also drops at least 0.3 volts, and up to 0.5 volts at 5A draw (max 0.7 volts but we shouldn't be running that much current through it). 
// (These are specs for the Vishay V12P10-M3/86A)
// Most of the time we will probably be at the low end of that scale, and that is also more conservative for LVC purposes. 
const float vAdj = 0.3;         // Our adjustment factor 

// To reduce the effect of noise, we run our voltage through a low-pass filter.
// With an alpha of 0.9, it is like giving the voltage reading a weight of 10%, and past readings a weight of 90%
const float alpha = 0.9;

    // Ok, here we go: first take an analog reading (will give us a number between 0-1023)
        voltSense = analogRead(pin_BattVoltage);
    //Convert the reading to actual voltage read (0 - 5 Vdc)
        unFilteredVoltage = (voltSense / 1024.0) * 5.0;
    // Now run it through low-pass filter
        if (firstPass) { filteredVoltage = unFilteredVoltage; firstPass = false; }
        filteredVoltage = alpha * filteredVoltage + (1.0 - alpha) * unFilteredVoltage;
    // Now convert the measured voltage to the external battery voltage by accounting for our voltage divider
        Voltage = filteredVoltage * multiplier;
    // Now also add our adjustment factor to account for the voltage drop on the input polarity diode
        Voltage += vAdj;

    // In testing this results in a pretty accurate reading
    // Takes about 700 microSeconds (0.7 mS = 0.0007 seconds)
        //DebugSerial->print("Voltage: ");
        //DebugSerial->println(Voltage,2);

    return Voltage;
}


