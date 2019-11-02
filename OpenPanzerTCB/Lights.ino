
// LIGHT #1 - HEADLIGHTS 
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// Light 1 also has the option of triggering a headlight sound
void Light1On()
{
    digitalWrite(pin_Light1, HIGH);    
    TankSound->HeadlightSound(); // The sound object will automatically ignore this if the headlight sound was disabled
    if (DEBUG) { DebugSerial->println(F("Light 1 On")); }
}

void Light1Off()
{
    digitalWrite(pin_Light1, LOW);
    TankSound->HeadlightSound(); // The sound object will automatically ignore this if the headlight sound was disabled
    if (DEBUG) { DebugSerial->println(F("Light 1 Off")); }
}

void Light1Toggle()
{
    static boolean Light1State = false;
    Light1State ? Light1Off() : Light1On();
    Light1State = !Light1State;
}


// LIGHT #2 - 
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void Light2On()
{
    digitalWrite(pin_Light2, HIGH);
    TankSound->HeadlightSound2(); // The sound object will automatically ignore this if the headlight 2 sound was disabled
    if (DEBUG) { DebugSerial->println(F("Light 2 On")); }
}

void Light2Off()
{   // Turn off 
    digitalWrite(pin_Light2, LOW);
    
    // Only play the headlight 2 sound if we are turning off the light, not if we are ending the machine gun
    if (Light2Blinking == false) TankSound->HeadlightSound2(); // The sound object will automatically ignore this if the headlight 2 sound was disabled

    // Delete blinking timer
    if (timer.isEnabled(Light2TimerID)) timer.deleteTimer(Light2TimerID);

    if (DEBUG)
    {
        if (Light2Blinking) { DebugSerial->println(F("2nd MG Stop"));} 
        else                { DebugSerial->println(F("Light 2 Off")); }
    }

    // Set the blink flag to false, we're done
    Light2Blinking = false;
}

void Light2Toggle()
{
    static boolean Light2State = false;
    Light2State ? Light2Off() : Light2On();
    Light2State = !Light2State;
}

void Light2TempState(boolean s)
{
    // We use this one internally when we want to change the output but don't necessarily want to stop 
    // a blinking effect or to display a debug message.
    digitalWrite(pin_Light2, s);
}

void Light2Blink()
{
    static boolean Light2State = false;

    if (!Light2Blinking)
    {
        Light2Blinking = true;
        if (DEBUG) {DebugSerial->println(F("2nd MG Start"));}
    }
        
    // Set the light state
    Light2TempState(Light2State);       // Use the "temp" pin control function otherwise we would turn off the blinker

    // Now set a timer to come back here and toggle it after the correct length of time has passed.
    Light2TimerID = timer.setTimeout(eeprom.ramcopy.SecondMGLightBlink_mS, Light2Blink);

    // Flip the state for next time
    Light2State = !Light2State;
}


// BRAKE LIGHTS - not a "special function" - the behavior of the brake light is handled automatically during braking
// -------------------------------------------------------------------------------------------------------------------------------------------------->   
void BrakeLightsOn()
{
    BrakeLightsActive = true;
    digitalWrite(pin_Brakelights, HIGH);
}

void BrakeLightsOff()
{
    BrakeLightsActive = false;

    // But just because we are turning off the brake lights, doesn't necessarily mean we are turning off the *running* lights
    if (RunningLightsActive == false)
    {   // If the running lighs aren't active, we can turn these off
        digitalWrite(pin_Brakelights, LOW);                
    }
    else
    {   // Otherwise set them to the dim level
        analogWrite(pin_Brakelights, RunningLightsDimLevel);
    }
}

// RUNNING LIGHTS
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// These are the same as the brake lights, only dim. When the brake lights come on, they will switch from dim to full bright. 
void RunningLightsOn()
{   
    RunningLightsActive = true;
    if (BrakeLightsActive == false)
    {   // But we only set the dim level if the brake lights aren't on
        analogWrite(pin_Brakelights, RunningLightsDimLevel);
    }
    if (DEBUG) { DebugSerial->println(F("Running Lights On")); }
}

void RunningLightsOff()
{   
    RunningLightsActive = false;
    
    // But just because we want to turn off the running lights, doesn't necessarily mean we want the brake lights off
    if (BrakeLightsActive == false)
    {   // Only turn the lights off if the brakes aren't on either
        digitalWrite(pin_Brakelights, LOW);
    }
    if (DEBUG) { DebugSerial->println(F("Running Lights Off")); }
}

void RunningLightsToggle()
{
    static boolean RunningLightsState = false;
    RunningLightsState ? RunningLightsOff() : RunningLightsOn();
    RunningLightsState = !RunningLightsState;
}

// AUX OUTPUT: could be lights, or anything else
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void AuxOutputOn()
{
    digitalWrite(pin_AuxOutput, HIGH);    
    if (DEBUG && !AuxOutputBlinking) { DebugSerial->println(F("Aux Output On")); }
}
void AuxOutputOff()
{
    // Turn off
    if (timer.isEnabled(AuxOutputTimerID)) timer.deleteTimer(AuxOutputTimerID);
    digitalWrite(pin_AuxOutput, LOW);
    if (DEBUG)
    {
        if (AuxOutputBlinking)          { DebugSerial->println(F("Stop Aux Output Blinking")); }
        else if (AuxOutputRevolving)    { DebugSerial->println(F("Stop Revolving Aux Light")); }
        else if (!Failsafe)             { DebugSerial->println(F("Aux Output Off")); }  // Don't need to print anything if the reason we turned it off is because of radio failsafe
    }
    // Now these are also false
    AuxOutputBlinking = false;
    AuxOutputRevolving = false;
}
void AuxOutputTempOff()
{
    // We use this one internally when we want the output to go off but we don't necessarily want to stop 
    // a blinking or revolving effect, or to display a debug message.
    digitalWrite(pin_AuxOutput, LOW);
}
void AuxOutputToggle()
{
    static boolean AuxOutputState = false;
    // Toggle the light state
    AuxOutputState ? AuxOutputOff() : AuxOutputOn();
    AuxOutputState = !AuxOutputState;
}
void AuxOutput_PresetDim()
{
    analogWrite(pin_AuxOutput, eeprom.ramcopy.AuxLightPresetDim);
    if (DEBUG) DebugSerial->println(F("Aux Output Preset Dim"));
}
void AuxOutputToggleDim()
{
    static boolean AuxDimState = false;
    // Toggle the light state from dim/off
    AuxDimState ? AuxOutputOff() : AuxOutput_PresetDim();
    AuxDimState = !AuxDimState;
}
void AuxOutput_SetLevel(uint16_t level)
{   
uint8_t auxLevel;
    
    // Recall, all analog special functions must expect values from 0-1023.
    // Since analogWrite requires values from 0-255, we need to use the map function: 
    analogWrite(pin_AuxOutput, map(level, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, 0, 255));

    // We also save the change to our PresetDim variable, in case we decide to write this change to EEPROM
    auxLevel = map(level, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, 0, 100);
    if (auxLevel != eeprom.ramcopy.AuxLightPresetDim)    
    {
        eeprom.ramcopy.AuxLightPresetDim = auxLevel;
        // This gets to be too much, don't bother printing it. 
        //if (DEBUG) 
        //{ 
            //DebugSerial->print(F("Set Aux Output Level: "));
            //PrintLnPct(eeprom.ramcopy.AuxLightPresetDim); 
        //}
    }   
}
void AuxOutputFlash()
{
    // This is sort of like a one-time blink. It can be used to create a second cannon flash,
    // or it can be used to emulate a button press to some external device. This is the regular
    // version, where the pin is normally held low, but during flash it is brought high briefly. 
    // First we turn the output on (flash)
    AuxOutputOn();  
    // Then we set a timer to turn it off after the flash length of time has passed
    AuxOutputTimerID = timer.setTimeout(eeprom.ramcopy.AuxLightFlashTime_mS, AuxOutputOff);

    if (DEBUG) DebugSerial->println(F("Aux Output Flashed"));
}
void AuxOutputInverseFlash()
{
    // This is sort of like a one-time blink. It can be used to create a second cannon flash,
    // or it can be used to emulate a button press to some external device. This is the inverse
    // version, where the pin is normally held high, but during flash it is dropped low briefly.
    // First we turn the output off (flash)
    AuxOutputOff();  
    // Then we set a timer to turn it on after the flash length of time has passed
    AuxOutputTimerID = timer.setTimeout(eeprom.ramcopy.AuxLightFlashTime_mS, AuxOutputOn);

    if (DEBUG) DebugSerial->println(F("Aux Output Inverse Flashed"));
}
void AuxOutputBlink()
{
    static boolean AuxOutputState = false;
    
    // Start blinking
    if (!AuxOutputBlinking) 
    {   // Don't do blinking and revolving at the same time
        AuxOutputOff(); // This clears any other effects first
        AuxOutputBlinking = true;
        if (DEBUG) { DebugSerial->println(F("Start Aux Output Blinking")); }
    }

    // Toggle the light state
    AuxOutputState ? AuxOutputTempOff() : AuxOutputOn();    // Use the "temp" off version here otherwise we would turn off the blinker
    AuxOutputState = !AuxOutputState;                       // Flop states

    // Now set a timer to come back here and toggle it after the correct length of time has passed.
    // The time can be different for the on portion and the off portion:
    if (AuxOutputState) AuxOutputTimerID = timer.setTimeout(eeprom.ramcopy.AuxLightBlinkOnTime_mS, AuxOutputBlink);
    else AuxOutputTimerID = timer.setTimeout(eeprom.ramcopy.AuxLightBlinkOffTime_mS, AuxOutputBlink);
}
void AuxOutputToggleBlink()
{
    if (AuxOutputBlinking)
    {   
        AuxOutputOff();
        AuxOutputBlinking = false;
        if (timer.isEnabled(AuxOutputTimerID)) timer.deleteTimer(AuxOutputTimerID);
        if (DEBUG) { DebugSerial->println(F("Stop Aux Output Blinking")); }
    }
    else
    {
        AuxOutputBlink();
    }
}
void AuxOutputRevolve()
{   // This starts the "revolving" effect, which frankly, doesn't look that great. 
    if (!AuxOutputRevolving)
    {   // Turn off light to start
        digitalWrite(pin_AuxOutput, LOW);
        // Don't do blinking and revolving at the same time, clear any other junk going on
        AuxOutputBlinking = false;
        if (timer.isEnabled(AuxOutputTimerID)) timer.deleteTimer(AuxOutputTimerID);
        // Ok, we are starting the revolver
        AuxOutputRevolving = true;
        if (DEBUG) { DebugSerial->println(F("Start Revolving Aux Light")); }        
        AuxOutputRevolver();
    }
}
void AuxOutputToggleRevolve()
{
    if (AuxOutputRevolving)
    {
        AuxOutputRevolving = false;
        digitalWrite(pin_AuxOutput, LOW);
        if (timer.isEnabled(AuxOutputTimerID)) timer.deleteTimer(AuxOutputTimerID);
        if (DEBUG) { DebugSerial->println(F("Stop Revolving Aux Light")); }
    }
    else
    {
        AuxOutputRevolve();
    }
}
void AuxOutputRevolver()
{
    const uint8_t flashLength_mS = 120;
    const uint8_t rotatingUpdateRate_mS = 25;
    static uint8_t revolvingLightDelay_mS;
    static uint8_t revolvingLightLevel = 0;
    static boolean revolvingLightDirection = HIGH;
       
    analogWrite(pin_AuxOutput, revolvingLightLevel);
    revolvingLightDelay_mS = rotatingUpdateRate_mS; // Delay for the rotating light

    if (revolvingLightDirection == HIGH)            // Brighter cycle
        revolvingLightLevel += 3;
    else
        revolvingLightLevel -= 3;                   // Dimmer cycle

    if (revolvingLightLevel > 150 && revolvingLightDirection == HIGH)
    {
        revolvingLightLevel = 255;                  // Full on for the flash
    } 
    else if (revolvingLightLevel > 220) 
    {
        revolvingLightLevel = 150;                  // Exit the flash
        revolvingLightDelay_mS = flashLength_mS;    // This is how long the flash will last
    }
 
    if (revolvingLightLevel == 255)
        revolvingLightDirection = LOW;
    if (revolvingLightLevel <= 25)
        revolvingLightDirection = HIGH;

    // Come back here and do the next bit after delay
    AuxOutputTimerID = timer.setTimeout(revolvingLightDelay_mS, AuxOutputRevolver);
        
}


// MACHINE GUN LIGHT - not a "special function": the machine gun effect is handled by the OP_Tank object. However we may
// want to directly manipulate it (for example as a notification) so these are added for convenience. 
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void MGLightOn()
{
    // Machine Gun LED must be set directly. We "or" the port pin bit with 1 to set it to 1, this turns it on
    MG_PORT |= (1 << MG_PORTPIN);
}

void MGLightOff()
{
    // Machine Gun LED must be set directly. We "and-not" the port pin bit with 1 to set it to 0, this turns it off
    MG_PORT &= ~(1 << MG_PORTPIN);    
}


// ONBOARD LED LIGHTS (Green and Red)
// -------------------------------------------------------------------------------------------------------------------------------------------------->   
void LED(int WhatLed, byte Status)
{
    digitalWrite(WhatLed, Status);
}

void RedLedOn()
{
    digitalWrite(pin_RedLED, HIGH);
}
void RedLedOff()
{
    digitalWrite(pin_RedLED, LOW);
}
void GreenLedOn()
{
    digitalWrite(pin_GreenLED, HIGH);
}
void GreenLedOff()
{
    digitalWrite(pin_GreenLED, LOW);
}

void GreenBlink()
{
    static boolean GreenState; 
    GreenState ? GreenLedOn() : GreenLedOff();
    GreenState = !GreenState;
}

void RedBlink()
{
    static boolean RedState; 
    RedState ? RedLedOn() : RedLedOff();
    RedState = !RedState;
}

void GreenBlinkSlow(int HowMany)
{   // This uses Delays and blocks all code!
    for (int i=1; i<=HowMany; i++)
    {
        GreenLedOn();
        delay(750);
        GreenLedOff();
        if (i < HowMany)
        { delay(500);  }
    }
}

void RedBlinkSlow(int HowMany)
{   // This uses Delays and blocks all code!
    for (int i=1; i<=HowMany; i++)
    {
        RedLedOn();
        delay(750);
        RedLedOff();
        if (i < HowMany)
        { delay(500);  }
    }
}

void BothBlinkSlow(int HowMany)
{   // This uses Delays and blocks all code!
    for (int i=1; i<=HowMany; i++)
    {
        GreenLedOn();
        RedLedOn();
        delay(750);
        GreenLedOff();
        RedLedOff();
        if (i < HowMany)
        { delay(500);  }
    }
}

void RedBlinkFast(int HowMany)
{   // This uses Delays and blocks all code!
    for (int i=1; i<=HowMany; i++)
    {
        RedLedOn();
        delay(100);
        RedLedOff();
        if (i < HowMany)
        { delay(90); }
    }
}

void GreenBlinkFast(int HowMany)
{   // This uses Delays and blocks all code!
    for (int i=1; i<=HowMany; i++)
    {
        GreenLedOn();
        delay(100);
        GreenLedOff();
        if (i < HowMany)
        { delay(90); }
    }
}

void GreenBlinkOne()
{ GreenBlinkFast(1); }

void GreenBlinkTwo()
{ GreenBlinkFast(2); }

void GreenBlinkThree()
{ GreenBlinkFast(3); }

void RedBlinkOne()
{ RedBlinkFast(1); }

void RedBlinkTwo()
{ RedBlinkFast(2); }

void RedBlinkThree()
{ RedBlinkFast(3); }


void AllLightsOff()
{
    digitalWrite(pin_RedLED, LOW);
    digitalWrite(pin_GreenLED, LOW);
    digitalWrite(pin_Light1, LOW);
    digitalWrite(pin_Light2, LOW);
    digitalWrite(pin_Brakelights, LOW);   
    digitalWrite(pin_HitNotifyLEDs, LOW);
    // We don't include the Aux output because we don't know if it's a light or what the user is using it for. 
    MGLightOff();
}

void RandomLights()
{
static boolean Toggle = true;
    // This routine turns some lights on, and some lights off. Each time it is called, the on lights go off, and the off lights go on. 
    // We don't include the Aux output, because the user may have something other than a light plugged into it. 
    digitalWrite(pin_RedLED, Toggle);
    digitalWrite(pin_GreenLED, !Toggle);
    digitalWrite(pin_Light1, Toggle);
    digitalWrite(pin_Light2, !Toggle);
    digitalWrite(pin_Brakelights, !Toggle);   
    digitalWrite(pin_HitNotifyLEDs, Toggle);
    Toggle ? MGLightOff() : MGLightOn();
    // Swap for next time
    Toggle = !Toggle;
}

void RandomLightsOnboardOnly()
{
static boolean Toggle = true;
    // Same thing as RandomLights, but we only toggle onboard LEDs
    digitalWrite(pin_RedLED, Toggle);
    digitalWrite(pin_GreenLED, !Toggle);
    // Swap for next time
    Toggle = !Toggle;
}


void StartFailsafeLights()
{
const int FailsafeBlinkRate = 50;
    
    // Start the failsafe blinker if it isn't already
    if (timer.isEnabled(RxSignalLostTimerID) == false)   
    {
        // Start with all lights off. This keeps off any outputs that we aren't going to include in the blinking effect (ie Aux output which may not be driving a light anyway)
        AllLightsOff();
            
        // The user has the option of blinking all the lights if radio signal is lost, but if not, 
        // we still blink the on-board LEDs (green and red)
        if (eeprom.ramcopy.FlashLightsWhenSignalLost) 
        {
            // Blink all lights
            RxSignalLostTimerID = timer.setInterval(FailsafeBlinkRate, RandomLights);
        }
        else 
        {   
            // Blink only the onboard LEDs
            RxSignalLostTimerID = timer.setInterval(FailsafeBlinkRate, RandomLightsOnboardOnly);
        }
    }
}

void StopFailsafeLights()
{
    // Quit the blinking
    if (timer.isEnabled(RxSignalLostTimerID))
    {
        timer.deleteTimer(RxSignalLostTimerID);
        RxSignalLostTimerID = 0;
        ResetAllLights();
    }
    // Also restore the running lights if the user wants them always on
    if (eeprom.ramcopy.RunningLightsAlwaysOn) RunningLightsOn();    
}

void BlinkLightsTankDestroyed()
{
    // This routine is to draw attention to the fact we have been destroyed. It blinks all the lights on the tank at a slow rate.
    RandomLights();
}

void ResetAllLights()
{   // This resets all the lights blinked above, otherwise when we exit failsafe some may be left on
    digitalWrite(pin_RedLED, LOW);
    digitalWrite(pin_GreenLED, LOW);
    digitalWrite(pin_Light1, LOW);
    digitalWrite(pin_Light2, LOW);
    digitalWrite(pin_Brakelights, LOW);   
    digitalWrite(pin_HitNotifyLEDs, LOW);
    MGLightOff();
}




// LOW VOLTAGE CUTOFF LIGHTS
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// In LVC mode, the lights blink in a pattern that starts very fast and then slows down. This is to indicate the tank is losing power. For an example
// of an opposite effect, see Cannon_RepairLights() in OP_Tank.cpp (lights blink slowly and then increase in speed). 
void LVC_BlinkHandler(void)
{   // Only start/continue the blinking effect if we are still in LVC
    if (LVC) 
    {
        RampBlinkDown(); 
    }
    else 
    {   // Stop the LVC blinking
        if (timer.isEnabled(LVC_BlinkTimerID)) timer.deleteTimer(LVC_BlinkTimerID);
        ResetAllLights();
    }
}

void RampBlinkDown()
{
    static boolean effectStarted = false;
    const int MaxInterval = 1000;
    
    const int Knee1 = 22;
    const int Knee2 = 143;
    const int Knee3 = 349;
    const int Knee4 = 550;

    const uint8_t AC_0 = 0;
    const uint8_t AC_1 = 1;
    const uint8_t AC_2 = 2;
    const uint8_t AC_3 = 4;
    const uint8_t AC_4 = 70;
    
    static int Interval;
    static int Add;

    if (!effectStarted) 
    {
        Interval = 0;
        Add = 1;
        effectStarted = true;
    }
    else
    {
        if      (Interval < Knee1)          Add += AC_0;
        else if (Interval < Knee2)          Add += AC_1;
        else if (Interval < Knee3)          Add += AC_2;
        else if (Interval < Knee4)          Add += AC_3;
        else if (Interval < MaxInterval)    Add += AC_4;
        else                                Add = 0;
    }

    // Toggle the light state
    RandomLights();

    if (Interval > MaxInterval)
    {
        effectStarted = false;
        ResetAllLights();
        // Go back to LVC_BlinkHandler after a brief delay. It will restart the blinking effect if appropriate. 
        LVC_BlinkTimerID = timer.setTimeout(1000,LVC_BlinkHandler);
    }
    else
    {
        // Now set a timer to come back here and toggle it after the correct length of time has passed.
        LVC_BlinkTimerID = timer.setTimeout(Interval, RampBlinkDown);
    }

    // For next time
    Interval += Add;    
}





