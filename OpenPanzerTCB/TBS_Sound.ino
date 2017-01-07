

void TBS_Setup()
{
    // Ok, before we run setup, stop everything. 
    StopEverything();
    
    // Turn the Red LED on for the duration of setup
    GreenLedOff();
    RedLedOn();
    
    // 
    DebugSerial->println();
    PrintDebugLine();
    DebugSerial->println(F("Start TBS Setup"));
    PrintDebugLine();
    
    // Initialize TBS outputs
    TankSound.InitializeOutputs();  // Set all outputs to defaults    

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
    PrintDebugLine();
    DebugSerial->println(F("End TBS Setup"));
    PrintDebugLine();
    DebugSerial->println();    

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
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("1")); }
}
void UserSound1_Repeat()
{
    TankSound.UserSound1_Repeat();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("1 - Start Repeating")); }
}
void UserSound1_Stop()
{
    TankSound.UserSound1_Stop();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("1 - Stop")); }
}

void TriggerUserSound2()
{
    TankSound.UserSound2();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("2")); }
}
void UserSound2_Repeat()
{
    TankSound.UserSound2_Repeat();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("2 - Start Repeating")); }
}
void UserSound2_Stop()
{
    TankSound.UserSound2_Stop();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("2 - Stop")); }
}

void PrintUserSound()
{
    DebugSerial->print(F("User Sound "));
}

