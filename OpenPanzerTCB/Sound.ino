
void InstantiateSoundObject(void)
{
    // Sanity check - make sure DriveType is valid
    if (eeprom.ramcopy.SoundDevice < SD_FIRST_SD || eeprom.ramcopy.SoundDevice > SD_LAST_SD)
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

void IncreaseVolume(void)
{
    TankSound->IncreaseVolume();
}
void DecreaseVolume(void)
{
    TankSound->DecreaseVolume();
}
void StopVolume(void)
{
    TankSound->StopVolume();
}

void EnableTrackOverlaySounds()
{
    TankSound->EnableTrackOverlay(true);
}

void DisableTrackOverlaySounds()
{
    TankSound->EnableTrackOverlay(false);
}

void TriggerUserSound1()
{
    TankSound->UserSound_Play(1);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("1")); }
}
void UserSound1_Repeat()
{
    TankSound->UserSound_Repeat(1);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("1 - Start Repeating")); }
}
void UserSound1_Stop()
{
    TankSound->UserSound_Stop(1);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("1 - Stop")); }
}

void TriggerUserSound2()
{
    TankSound->UserSound_Play(2);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("2")); }
}
void UserSound2_Repeat()
{
    TankSound->UserSound_Repeat(2);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("2 - Start Repeating")); }
}
void UserSound2_Stop()
{
    TankSound->UserSound_Stop(2);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("2 - Stop")); }
}

void TriggerUserSound3()
{
    TankSound->UserSound_Play(3);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("3")); }
}
void UserSound3_Repeat()
{
    TankSound->UserSound_Repeat(3);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("3 - Start Repeating")); }
}
void UserSound3_Stop()
{
    TankSound->UserSound_Stop(3);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("3 - Stop")); }
}

void TriggerUserSound4()
{
    TankSound->UserSound_Play(4);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("4")); }
}
void UserSound4_Repeat()
{
    TankSound->UserSound_Repeat(4);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("4 - Start Repeating")); }
}
void UserSound4_Stop()
{
    TankSound->UserSound_Stop(4);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("4 - Stop")); }
}

void TriggerUserSound5()
{
    TankSound->UserSound_Play(5);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("5")); }
}
void UserSound5_Repeat()
{
    TankSound->UserSound_Repeat(5);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("5 - Start Repeating")); }
}
void UserSound5_Stop()
{
    TankSound->UserSound_Stop(5);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("5 - Stop")); }
}

void TriggerUserSound6()
{
    TankSound->UserSound_Play(6);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("6")); }
}
void UserSound6_Repeat()
{
    TankSound->UserSound_Repeat(6);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("6 - Start Repeating")); }
}
void UserSound6_Stop()
{
    TankSound->UserSound_Stop(6);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(F("6 - Stop")); }
}

void PrintUserSound()
{
    DebugSerial->print(F("User Sound "));
}

// This routine is only needed for testing during development of compatibility with Benedini TBS Flash v3. //
/*
void TBSTest(void)
{
OP_TBS * TBS;        
TBS = new OP_TBS (&timer);        

    // Ok, before we run this test, stop everything.      
    StopEverything();     

    // Turn the Red LED on for the duration of setup      
    GreenLedOff();        
    RedLedOn();       

    //        
    DebugSerial->println();       
    PrintDebugLine();     
    DebugSerial->println(F("Start TBS Prop3 Test"));       
    PrintDebugLine();     
    delay(2000);

    // Initialize TBS outputs     
    TBS->InitializeOutputs();  // Set all outputs to defaults    

    // Run the test
    TBS->testProp3();

    // Ok, we're done        
    PrintDebugLine();     
    DebugSerial->println(F("End TBS Prop3 Test"));     
    PrintDebugLine();     
    DebugSerial->println();           

    // Blink the Green LED 3 times quickly, then turn all leds off.       
    GreenBlinkFast(3);        
    RedLedOff();    
}
*/


