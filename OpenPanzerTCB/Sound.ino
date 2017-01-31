
void InstantiateSoundObject(void)
{
    // Sanity check - make sure DriveType is valid
    if (eeprom.ramcopy.SoundDevice < SD_FIRST_SD || eeprom.ramcopy.DriveType > SD_LAST_SD)
    {   // Default to TBS Mini if we have some invalid value, and update EEPROM too
        eeprom.ramcopy.SoundDevice = SD_BENEDINI_TBSMINI;
        EEPROM.updateInt(offsetof(_eeprom_data, SoundDevice), SD_BENEDINI_TBSMINI);
    }
  
    switch (eeprom.ramcopy.SoundDevice)
    {
        case SD_BENEDINI_TBSMINI:
            TankSound = new BenediniTBS(&timer);
            RCOutput6_Available = false;                // The Benedini requires all three of these RC outputs, so they are not available for other uses
            RCOutput7_Available = false;
            RCOutput8_Available = false;
            break;
            
        case SD_OP_SOUND_CARD:
            TankSound = new OP_SoundCard(&MotorSerial); // We communicate on the same serial port as we use for serial motor controllers
            RCOutput6_Available = true;                 // Because the OP Sound Card communicates over serial, we have all three of these RC outputs free for general use
            RCOutput7_Available = true;
            RCOutput8_Available = true;
            break;

        case SD_TAIGEN_SOUND:
            TankSound = new OP_TaigenSound();
            TankServos.detach(PROP1);                   // We will be using this output for the Taigen sound card, so make sure it is detached as a servo output (should already be anyway)
            RCOutput6_Available = true;                 // Taigen needs the Prop1 output (Servo 8). But RC outputs 6 and 7 are free for general use. 
            RCOutput7_Available = true;
            RCOutput8_Available = false;                // This one reserved for Taigen comms. 
            break;
            
        case SD_BEIER_USMRC2:
            // Not available yet
            RCOutput6_Available = false;                // Don't know about these but keep them reserved for now. 
            RCOutput7_Available = false;
            RCOutput8_Available = false;            
            break;

        default:
            // We shouldn't end up here but in case we do, we need to define something or else the program will croak at runtime.
            // We set it to TBS Mini and save it to EEPROM so we don't end up here again next time
            eeprom.ramcopy.SoundDevice = SD_BENEDINI_TBSMINI;
            EEPROM.updateInt(offsetof(_eeprom_data, SoundDevice), SD_BENEDINI_TBSMINI);
            TankSound = new BenediniTBS(&timer);
            RCOutput6_Available = false;                // The Benedini requires all three of these RC outputs, so they are not available for other uses
            RCOutput7_Available = false;
            RCOutput8_Available = false;            
    }

    // Now initialize 
    TankSound->begin();     

}

void TBS_Setup()
{
    // When input button held down for 2 seconds and dipswitch 3 is in the off position while dipswitch 4 is in the on position, 
    // we will enter this menu, that allows teaching the TBS our servo positions on its 3 "prop" inputs. 
    // This setup menu could be rendered unnecessary if Benedini releases a TBS update. 
    
    // Temporary pointer to OP_TBS class directly (not through OP_Sound)
    OP_TBS * TBS;
    TBS = new OP_TBS (&timer);
    
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
    TBS->InitializeOutputs();  // Set all outputs to defaults

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
    TBS->TeachEncoder();
    
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
void SetVolume(uint16_t unmapped_level)
{
    static uint8_t lastLevel = 0;

    // TankSound volume adjustment function expects a uint8_t value from 0-100
    uint8_t level = map(unmapped_level, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, 0, 100); 
    
    // Only update if changed by some small amount - this hysterisis prevents us from constantly spamming
    // the serial port as a consequence of a jittery signal. 
    if (abs(level - lastLevel) >  2)    // Apply changes greater than 2 percent
    {
        if (level < 3)  level = 0;      // This helps ensure we can always get an actual Off position 
        if (level > 97) level = 100;    // And this that we can always reliably achieve On
        TankSound->SetVolume(level);
        lastLevel = level; 
        if (DEBUG) { DebugSerial->print(F("Set volume: ")); PrintLnPct(level); } 
    }
}

void TriggerUserSound1()
{
    TankSound->UserSound1();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("1")); }
}
void UserSound1_Repeat()
{
    TankSound->UserSound1_Repeat();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("1 - Start Repeating")); }
}
void UserSound1_Stop()
{
    TankSound->UserSound1_Stop();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("1 - Stop")); }
}

void TriggerUserSound2()
{
    TankSound->UserSound2();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("2")); }
}
void UserSound2_Repeat()
{
    TankSound->UserSound2_Repeat();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("2 - Start Repeating")); }
}
void UserSound2_Stop()
{
    TankSound->UserSound2_Stop();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("2 - Stop")); }
}

void TriggerUserSound3()
{
    TankSound->UserSound3();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("3")); }
}
void UserSound3_Repeat()
{
    TankSound->UserSound3_Repeat();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("3 - Start Repeating")); }
}
void UserSound3_Stop()
{
    TankSound->UserSound3_Stop();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("3 - Stop")); }
}

void TriggerUserSound4()
{
    TankSound->UserSound4();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("4")); }
}
void UserSound4_Repeat()
{
    TankSound->UserSound4_Repeat();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("4 - Start Repeating")); }
}
void UserSound4_Stop()
{
    TankSound->UserSound4_Stop();
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("4 - Stop")); }
}

void PrintUserSound()
{
    DebugSerial->print(F("User Sound "));
}

