

void StartFailsafe()
{
    if (!Failsafe)    
    {
        StartFailsafeLights();                      // Start the failsafe blinking effect
        StopEverything();                           // Stop every physical movement
        Failsafe = true;                            // Set the failsafe flag
        if (DEBUG) { DebugSerial->println(F("RADIO FAILSAFE")); }
    }
}

void EndFailsafe()
{
    if (Failsafe)
    {
        Failsafe = false;
        StopFailsafeLights();
        if (DEBUG) 
        { 
            DebugSerial->print(F("Radio Ready - ")); 
            DebugSerial->println(RadioProtocol(Radio.getProtocol()));
        }    
    }
}


