
// IO Ports
// -------------------------------------------------------------------------------------------------------------------------------->>
// Not very sophisticated. But there are only two of them, and the functionality is basic. No need for a whole class

void SetActiveInputFlag(void)
{
    // It is wise to set these to inputs if we are not going to use them, because it is less likely you will accidentally fry a pin that way. 
    // But just because they are set to inputs, doesn't mean we are using them. We are only using them if the input is set as a trigger to some 
    // function. So let's run through the function triggers and see if any match.     

    for (int i=0; i<NUM_IO_PORTS; i++)
    {
        IO_Pin[i].inputActive = false;   // Start off with Active = false
        if (IO_Pin[i].Settings.dataDirection == INPUT)
        {
            for (int t = 0; t <MAX_FUNCTION_TRIGGERS; t++)
            {   
                if (eeprom.ramcopy.SF_Trigger[t].TriggerID >= (trigger_id_multiplier_ports * (i + 1)) && 
                    eeprom.ramcopy.SF_Trigger[t].TriggerID <  (trigger_id_multiplier_ports * (i + 2)))
                {
                    IO_Pin[i].inputActive = true;    // Ok, this is an input, and it is assigned to a function. Set Active to true
                    break;
                }
            }
        }
    }
    //DebugSerial->print(F("A Active Input: "));
    //PrintLnYesNo(IO_Pin[0].inputActive);
    //DebugSerial->print(F("B Active Input: "));
    //PrintLnYesNo(IO_Pin[1].inputActive);
}

void ReadIOPorts()
{
    // No point doing analog reads every loop if these are not set as inputs to some function,
    // so we do the inputActive check first
    if (IO_Pin[IOA].inputActive) PortA_ReadValue();
    if (IO_Pin[IOB].inputActive) PortB_ReadValue();
}

// PORT A - OUTPUT FUNCTIONS (On, Off, Toggle)
boolean isPortA_Output(void)
{   // If this statement is true, PortA is set to Output
    // If false, it is an input
    return (IO_port & _BV(IO_A_bit));
}
void PortA_On()
{   // For safety, we only do anything if this has been set to output
    if (isPortA_Output()) 
    { 
        digitalWrite(pin_IO_A, HIGH);
        IO_Pin[IOA].outputValue = HIGH;
        if (DEBUG) { DebugSerial->println(F("IO Port A Output - On")); }
    }
}
void PortA_Off()
{   // For safety, we only do anything if this has been set to output
    if (isPortA_Output()) 
    { 
        digitalWrite(pin_IO_A, LOW);
        IO_Pin[IOA].outputValue = LOW;
        if (DEBUG) { DebugSerial->println(F("IO Port A Output - Off")); }
    }
}
void PortA_Toggle()
{
    static boolean PortA_State = false;
    PortA_State ? PortA_Off() : PortA_On();
    PortA_State = !PortA_State;
}
void PortA_Pulse()
{
    // This function briefly holds the pin in the opposite direction of its default state.
    // For example if the default is high, the pin is held low briefly. This can be used to 
    // emulate a "button push" to external devices that can accept a logic signal.
    if (IO_Pin[IOA].Settings.dataType)
    {   
        PortA_Off();                                // Go in the opposite direction (dataType == true means default high, so go low)
        timer.setTimeout(IO_PULSE_TIME, PortA_On);  // Wait briefly then revert to default high
    }
    else
    {
        PortA_On();                                 // Go in the opposite direction (dataType == false means default low, so go high)
        timer.setTimeout(IO_PULSE_TIME, PortA_Off); // Wait briefly then revert to default low
    }
}
// PORT A - INPUT FUNCTION 
void PortA_ReadValue(void)
{   int oldVal;
    if (isPortA_Output()==false)
    {
        oldVal = IO_Pin[IOA].inputValue;
        if (IO_Pin[IOA].Settings.dataType)
        {   // If the user set this to "digital" input (dataType = 1), anything over 1/2 counts as 0 (because we have pullups turned on), 
            // and anything below counts as 1. In other words, the input will automatically be off (high). The user has to 
            // specifically tie the input to ground to set the input on (low). 
            analogRead(pin_IO_A) > 512 ? IO_Pin[IOA].inputValue = 0 : IO_Pin[IOA].inputValue = 1;
        }
        else
        {   // In this case we want to save the actual analog reading.
            // We may want to filter this (take an average) to eliminate noise, but that is not yet implemented.
            IO_Pin[IOA].inputValue = analogRead(pin_IO_A);
        }
        // Only set the updated flag if the value has changed. 
        IO_Pin[IOA].inputValue == oldVal ? IO_Pin[IOA].updated = false : IO_Pin[IOA].updated = true;   
    }
}


// PORT B - OUTPUT FUNCTIONS (On, Off, Toggle)
boolean isPortB_Output(void)
{   // If this statement is true, PortB is set to Output
    // If false, it is an input
    return (IO_port & _BV(IO_B_bit));
}
void PortB_On()
{   // For safety, we only do anything if this has been set to output
    if (isPortB_Output()) 
    { 
        digitalWrite(pin_IO_B, HIGH);
        IO_Pin[IOB].outputValue = HIGH;
        if (DEBUG) { DebugSerial->println(F("IO Port B Output - On")); }
    }
}
void PortB_Off()
{   // For safety, we only do anything if this has been set to output
    if (isPortB_Output()) 
    { 
        digitalWrite(pin_IO_B, LOW);
        IO_Pin[IOB].outputValue = LOW;
        if (DEBUG) { DebugSerial->println(F("IO Port B Output - Off")); }
    }
}
void PortB_Toggle()
{
    static boolean PortB_State = false;
    PortB_State ? PortB_Off() : PortB_On();
    PortB_State = !PortB_State;
}
void PortB_Pulse()
{
    // This function briefly holds the pin in the opposite direction of its default state.
    // For example if the default is high, the pin is held low briefly. This can be used to 
    // emulate a "button push" to external devices that can accept a logic signal.
    if (IO_Pin[IOB].Settings.dataType)
    {   
        PortB_Off();                                // Go in the opposite direction (dataType == true means default high, so go low)
        timer.setTimeout(IO_PULSE_TIME, PortB_On);  // Wait briefly then revert to default high
    }
    else
    {
        PortB_On();                                 // Go in the opposite direction (dataType == false means default low, so go high)
        timer.setTimeout(IO_PULSE_TIME, PortB_Off); // Wait briefly then revert to default low
    }
}

// PORT B - INPUT FUNCTION 
void PortB_ReadValue(void)
{   int oldVal;
    if (isPortB_Output()==false) // In other words, do this only if PortB is set to Input
    {
        oldVal = IO_Pin[IOB].inputValue;
        if (IO_Pin[IOB].Settings.dataType)
        {   // If the user set this to "digital" input (dataType = 1), anything over 1/2 counts as 0 (because we have pullups turned on), 
            // and anything below counts as 1. In other words, the input will automatically be off (high). The user has to 
            // specifically tie the input to ground to set the input on (low). 
            analogRead(pin_IO_B) > 512 ? IO_Pin[IOB].inputValue = 0 : IO_Pin[IOB].inputValue = 1;
        }
        else
        {   // In this case we want to save the actual analog reading.
            // We may want to filter this (take an average) to eliminate noise, but that is not yet implemented.
            IO_Pin[IOB].inputValue = analogRead(pin_IO_B);
        }
        // Only set the updated flag if the value has changed. 
        IO_Pin[IOB].inputValue == oldVal ? IO_Pin[IOB].updated = false : IO_Pin[IOB].updated = true;
    }
}


