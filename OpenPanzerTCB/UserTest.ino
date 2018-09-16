
// These are generic special functions that the user can fill in to control whatever they want. 

// Here are two "digital" functions - meaning they can only be triggered by a digital trigger, and they have no paramaters
// ------------------------------------------------------------------------------------------------------------------------------------->>
void UserFunction1(void)
{
    // Do something here

    if (DEBUG) DebugSerial->println(F("User Function 1")); 
}


void UserFunction2(void)
{
    // Do something here

    if (DEBUG) DebugSerial->println(F("User Function 2")); 
}



// Here are two "analog" functions - meaning they can only be triggered by an analog input, and the parameter "level" 
// will be a number from 0-1023 (or ANALOG_SPECFUNCTION_MIN_VAL to ANALOG_SPECFUNCTION_MAX_VAL) representing the 
// position of the analog input
// ------------------------------------------------------------------------------------------------------------------------------------->>
void User_Analog_Function1(uint16_t level)
{
    static uint16_t lastLevel;

    if (lastLevel != level)
    {
        lastLevel = level;
        
        // Do something here with "level"    
        
        if (DEBUG) { DebugSerial->print(F("Analog User Function 1 - ")); DebugSerial->println(level); }
    }
}

void User_Analog_Function2(uint16_t level)
{
    static uint16_t lastLevel;

    if (lastLevel != level)
    {
        lastLevel = level;
        
        // Do something here with "level"    
        
        if (DEBUG) { DebugSerial->print(F("Analog User Function 2 - ")); DebugSerial->println(level); }
    }
}



