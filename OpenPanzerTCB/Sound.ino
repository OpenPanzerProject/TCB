
void InstantiateSoundObject(void)
{
    // Sanity check - make sure Sound Device is valid
    if (eeprom.ramcopy.SoundDevice < SD_FIRST_SD || eeprom.ramcopy.SoundDevice > SD_LAST_SD)
    {   // Default to TBS Mini if we have some invalid value, and update EEPROM too
        eeprom.ramcopy.SoundDevice = SD_BENEDINI_TBSMINI;
        EEPROM.updateInt(offsetof(_eeprom_data, SoundDevice), SD_BENEDINI_TBSMINI);
    }
  
    switch (eeprom.ramcopy.SoundDevice)
    {
        case SD_BENEDINI_TBSMINI:
            TankSound = new BenediniTBS(&timer, false); // false means "not the Micro" but instead this is the Mini
            RCOutput6_Available = false;                // The Benedini Mini requires all three of these RC outputs, so they are not available for other uses
            RCOutput7_Available = false;
            RCOutput8_Available = false;
            break;

        case SD_BENEDINI_TBSMICRO:
            TankSound = new BenediniTBS(&timer, true);  // true means "Micro" (not Mini)
            RCOutput6_Available = true;                 // The Benedini Micro will only use Prop 1 and 2 (RC Outputs 8 and 7), Prop 3 is open for the user, he could program some fancy switches in the Tx and pass them through. 
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

        case SD_ONBOARD:                                // No onboard sound for the MkI TCB - fallthrough to default
        default:
            // We shouldn't end up here but in case we do, we need to define something or else the program will croak at runtime.
            // We set it to TBS Mini and save it to EEPROM so we don't end up here again next time
            eeprom.ramcopy.SoundDevice = SD_BENEDINI_TBSMINI;
            EEPROM.updateInt(offsetof(_eeprom_data, SoundDevice), SD_BENEDINI_TBSMINI);
            TankSound = new BenediniTBS(&timer, false); // false meaning Mini, not Micro
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

void TriggerUserSound(uint8_t s)
{
    TankSound->UserSound_Play(s);
    if (DEBUG) { PrintUserSound(); DebugSerial->println(s); }
}
void UserSound_Repeat(uint8_t s)
{
    TankSound->UserSound_Repeat(s);
    if (DEBUG) { PrintUserSound(); DebugSerial->print(s); DebugSerial->println(F(" - Start Repeating")); }
}
void UserSound_Stop(uint8_t s)
{
    TankSound->UserSound_Stop(s);
    if (DEBUG) { PrintUserSound(); DebugSerial->print(s); DebugSerial->println(F(" - Stop")); }
}
void UserSound_StopAll()
{
    TankSound->UserSound_StopAll();
    if (DEBUG) { DebugSerial->println(F("Stop All User Sounds")); }
}

void PrintUserSound()
{
    DebugSerial->print(F("User Sound "));
}

void SoundBank(soundbank sb, switch_action a)
{
    TankSound->SoundBank(sb, a);    // Send the sound bank command
    if (DEBUG) 
    { 
        PrintSoundBank(sb); 
        switch (a)
        {
            case ACTION_ONSTART:    DebugSerial->println(F("Play/Stop"));       break;
            case ACTION_PLAYNEXT:   DebugSerial->println(F("Play Next"));       break;
            case ACTION_PLAYPREV:   DebugSerial->println(F("Play Previous"));   break;
            case ACTION_PLAYRANDOM: DebugSerial->println(F("Play Random"));     break;
            default:                break;
        }
    }
}

void PrintSoundBank(soundbank sb)
{
    DebugSerial->print(F("Sound Bank ")); 
    switch (sb)
    {
        case SOUNDBANK_A: DebugSerial->print(F("A - ")); break;
        case SOUNDBANK_B: DebugSerial->print(F("B - ")); break;
        default: break;
    }
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


