

void TBS_Setup()
{
    // Ok, before we run setup, stop everything. 
    StopEverything();
    
    // Turn the Red LED on for the duration of setup
    GreenLedOff();
    RedLedOn();
    
    // 
    DebugSerial->println(F("Start TBS Setup"));
    
    // Initialize TBS outputs
    TankSound.IdleEngine();         // Initialize PROP1: set engine speed to idle
    TankSound.PROP2_OFF();          // Initialize PROP2: set to middle position (1500 PWM). That way TBS knows we have a 3-way switch.
    TankSound.StopSpecialSounds();  // Initialize PROP3: set to off

    // Tell user to press TBS Button. TBS will record neutral positions
    DebugSerial->println(F("Press TBS Button"));
    DebugSerial->println(F("After TBS beeps, press button on TCB board"));

    // Now we will wait for the user to press the button on the TCB board one more time. 
    // However, this code has run so fast they are probably still holding it down from when they entered the menu. 
    // Wait for them to release
    do { delay(10); InputButton.read(); } while (!InputButton.wasReleased());              

    // Ok, now while we're waiting, blink the green LED fast
    GreenLedOn();
    SetTimeUp();
    do
    {   if (TimeUp) 
        {   GreenBlink(); 
            StartWaiting_mS(100);
        }
        timer.run();
        InputButton.read();
    } while (!InputButton.wasReleased());

    // Now teach the sound unit throttle speeds and encoder positions
    TankSound.TeachEncoder();
    
    // Ok, we're done
    DebugSerial->println(F("All Done"));

    // To let them know it's over, blink the Green LED 3 times quickly, then turn all leds off. 
    GreenBlinkFast(3);
    RedLedOff();
    
    // Now we return to the main loop
}


// We need local functions here in the sketch so we can assign these to our
// special function callbacks in LoadFunctionTriggers() in the ObjectSetup tab
void TriggerUserSound1()
{
    TankSound.UserSound1();
    if (DEBUG) DebugSerial->println(F("User Sound 1")); 
}
void UserSound1_Repeat()
{
    TankSound.UserSound1_Repeat();
    if (DEBUG) DebugSerial->println(F("User Sound 1 - Start Repeating")); 
}
void UserSound1_Stop()
{
    TankSound.UserSound1_Stop();
    if (DEBUG) DebugSerial->println(F("User Sound 1 - Stop")); 
}

void TriggerUserSound2()
{
    TankSound.UserSound2();
    if (DEBUG) DebugSerial->println(F("User Sound 2"));     
}
void UserSound2_Repeat()
{
    TankSound.UserSound2_Repeat();
    if (DEBUG) DebugSerial->println(F("User Sound 2 - Start Repeating"));     
}
void UserSound2_Stop()
{
    TankSound.UserSound2_Stop();
    if (DEBUG) DebugSerial->println(F("User Sound 2 - Stop"));     
}

