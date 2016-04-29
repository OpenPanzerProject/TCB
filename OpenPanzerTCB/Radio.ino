

void StartFailsafe()
{
    if (!Failsafe)    
    {
        StopEverything();                           // Stop every physical movement
        Failsafe = true;                            // Set the failsafe flag
        StartFailsafeLights();                      // Start the failsafe blinking effect
        if (DEBUG) { DebugSerial->println(F("RADIO FAILSAFE")); }
    }
}

void EndFailsafe()
{
    if (Failsafe)
    {
        Failsafe = false;
        StopFailsafeLights();
        if (SAVE_DEBUG) 
        { 
            DebugSerial->print(F("Radio Ready - ")); 
            DebugSerial->println(RadioProtocol(Radio.getProtocol()));
        }    
    }
}


