
void RadioSetup()
{
stick_channel * ch[STICKCHANNELS];
int i;
unsigned long TotThrottlePulse = 0;
unsigned long TotTurnPulse = 0;
unsigned long TotElevationPulse = 0;
unsigned long TotAzimuthPulse = 0;
int ThrottleCount = 0;
int TurnCount = 0;
int ElevationCount = 0;
int AzimuthCount = 0;
float TempFloat;
    //int TypicalPulseCenter = 1500;
    //int MaxPossiblePulse = 2250;
    //int MinPossiblePulse = 750;
int GreenBlinker;                        // We'll use this to create a SimpleTimer for the green led, that flashes to represent which stage we are in.     


    // Acknowledge setup entry with a beep
    //TankSound.ForceBeep();    // Beeps have been removed

    // Save our four stick channels to an array, this makes the code a lot shorter    
    ch[0] = &Radio.Sticks.Throttle;
    ch[1] = &Radio.Sticks.Turn;
    ch[2] = &Radio.Sticks.Elevation;
    ch[3] = &Radio.Sticks.Azimuth;

    // Ok, before we run setup, stop everything. 
    StopEverything();
    
    // Turn the Red LED on for the duration of setup
    GreenLedOff();
    RedLedOn();
        
    DebugSerial->println();
    DebugSerial->println(F("ENTERING RADIO SETUP")); 


    // STAGE 1 = Read max travel values from radio, save to EEPROM
    // ------------------------------------------------------------------------------------------------------------------------------------------>                  
        // Transition to Stage 1:
        // Green LED on steady for two seconds
        delay(2000);
        DebugSerial->println(F("STAGE 1 - STORE MAX TRAVEL VALUES"));
        PrintDebugLine();
        DebugSerial->println(F("Move all controls to maximum values"));
        DebugSerial->println(F("while green LED blinks"));
        DebugSerial->println();
        GreenLedOn();
        delay(2000);
        GreenLedOff();
        delay(2000);

        // Start green LED blinking for stage one: one blink every 1200 ms
        GreenBlinker = StartBlinking_ms(pin_GreenLED, 1, 1200);    // Blip the GreenLED once every 1200ms
        StartWaiting_sec(10);
        DebugSerial->println(F("Reading..."));
        //TankSound.ForceBeep();    // Beeps have been removed

        // We initialize every min and max value to TypicalPulseCenter. In the loop below we will record deviations from the center. 
        for (i=0; i<4; i++)
        {
            ch[i]->Settings->pulseMin = ch[i]->Settings->pulseMax = DEFAULT_PULSE_CENTER;
        }

        // Repeat until StartWaiting timer is up
        do
        {   // Update timer
            timer.run();
            // Read channels while the user moves the sticks to the extremes
            Radio.GetCommands();
            // Only save min/max values if they are greater or less than the last time through the loop. 
            // At the end we should have the true min and max for each channel.
            for (i=0; i<4; i++)
            {
                if (ch[i]->updated && (ch[i]->pulse < ch[i]->Settings->pulseMin)) { ch[i]->Settings->pulseMin = ch[i]->pulse; }
                if (ch[i]->updated && (ch[i]->pulse > ch[i]->Settings->pulseMax)) { ch[i]->Settings->pulseMax = ch[i]->pulse; }
            }
        }
        while (!TimeUp);    // Keep looping until time's up
        StopBlinking(GreenBlinker);
        
        // Sanity check in case something weird happened (like Tx turned off during setup)
        for (i=0; i<4; i++)
        {
            if (ch[i]->Settings->pulseMin < MIN_POSSIBLE_PULSE) {ch[i]->Settings->pulseMin = MIN_POSSIBLE_PULSE; }
            if (ch[i]->Settings->pulseMax > MAX_POSSIBLE_PULSE) {ch[i]->Settings->pulseMax = MAX_POSSIBLE_PULSE; }
        }            

        // Save values to EEPROM
        SaveStickSettingsToEEPROM(); 
        
        DebugSerial->println();
        DebugSerial->println(F("Stage 1 Results"));
        DumpRadioStickMinMax();


        DebugSerial->println();
        DebugSerial->println(F("STAGE 1 COMPLETE"));
        DebugSerial->println();
        DebugSerial->println();
        
     
    // Stage 2 = Set Channel center values (I've found they are not always equal to one half of max travel)
    // ------------------------------------------------------------------------------------------------------------------------------------------>       
        // Transition to Stage 2:
        // Off for two seconds, two slow blinks
        GreenLedOff();
        delay(2000);        
        DebugSerial->println(F("STAGE 2 - STORE CENTER VALUES"));
        PrintDebugLine();
        DebugSerial->println(F("Place both sticks at their NEUTRAL (center) positions."));
        DebugSerial->println();
        GreenBlinkSlow(2);
        delay(2000);

        // Initialize some variables
        TotThrottlePulse = 0;
        TotTurnPulse = 0;                        
        TotElevationPulse = 0;                    
        TotAzimuthPulse = 0;                
        ThrottleCount = 0;
        TurnCount = 0;
        ElevationCount = 0;
        AzimuthCount = 0;


        // Start green LED blinking for stage two: two blinks every 1200 ms
        GreenBlinker = StartBlinking_ms(pin_GreenLED, 2, 1200);    // Blip the GreenLED twice every 1200ms
        StartWaiting_sec(3); // For the first bit of time we don't take any readings, this lets the user get the sticks centered
        DebugSerial->print(F("Reading..."));
        //TankSound.ForceBeeps(2);    // Beeps have been removed
        do
        {
            timer.run();
        }        
        while (!TimeUp);
        StartWaiting_sec(4); // Now for the next four seconds we check the sticks
        do
        {        
            // Refresh the timer
            timer.run();
            // Get Rx commands
            Radio.GetCommands();
            // Add to our readings 
            if (Radio.Sticks.Throttle.updated)  { TotThrottlePulse += Radio.Sticks.Throttle.pulse; ThrottleCount++; }
            if (Radio.Sticks.Turn.updated)      { TotTurnPulse += Radio.Sticks.Turn.pulse; TurnCount++; }
            if (Radio.Sticks.Elevation.updated) { TotElevationPulse += Radio.Sticks.Elevation.pulse; ElevationCount++; }
            if (Radio.Sticks.Azimuth.updated)   { TotAzimuthPulse += Radio.Sticks.Azimuth.pulse; AzimuthCount++; }
        }
        while (!TimeUp);    // Keep looping until time's up
        StopBlinking(GreenBlinker);

        // Finally we record our readings
        TempFloat = (float)TotThrottlePulse / (float)ThrottleCount;
        Radio.Sticks.Throttle.Settings->pulseCenter = (int)lround(TempFloat);
        TempFloat = (float)TotTurnPulse / (float)TurnCount;
        Radio.Sticks.Turn.Settings->pulseCenter = (int)lround(TempFloat);
        TempFloat = (float)TotElevationPulse / (float)ElevationCount;
        Radio.Sticks.Elevation.Settings->pulseCenter = (int)lround(TempFloat);
        TempFloat = (float)TotAzimuthPulse / (float)AzimuthCount;
        Radio.Sticks.Azimuth.Settings->pulseCenter = (int)lround(TempFloat);               

        // Sanity check in case something weird happened (like Tx turned off during setup, or some channels disconnected)
        for (i=0; i<4; i++)
        {
            if ((ch[i]->Settings->pulseCenter < MIN_POSSIBLE_PULSE) || (ch[i]->Settings->pulseCenter > MAX_POSSIBLE_PULSE)) {ch[i]->Settings->pulseCenter = DEFAULT_PULSE_CENTER; }
        }     

        // Save values to EEPROM
        SaveStickSettingsToEEPROM();

        DebugSerial->println();
        DebugSerial->println(F("Stage 2 Results"));
        DumpRadioStickCenters();

        DebugSerial->println();
        DebugSerial->println(F("STAGE 2 COMPLETE"));
        DebugSerial->println();        
        DebugSerial->println();  


    // Stage 3 = Set channel reversing. 
    // ------------------------------------------------------------------------------------------------------------------------------------------>                                 
        // Method used here is to ask the user to move both sticks up and out (Left stick up and to the left, right stick up and to the right)
        // We take a string of readings and average them. We see if the pulse lengths are long or short, and knowing where the sticks are physically, allows
        // use to determine if we need to reverse any channels in software. 
        // NOTE: For channels beyond the basic 4 we do not provide software reversing. 
        //   A) because these are switches, knobs or sliders and it is less clear which direction of movement is the most logical for the user, and 
        //   B) because if the user has a radio with greater than 4 channels they also probably have the ability to do channel reversing at the Transmitter. 

        // Transition to Stage 3:
        GreenLedOff();
        delay(2000);            
        DebugSerial->println(F("STAGE 3 - STORE CHANNEL DIRECTIONS"));
        PrintDebugLine();
        DebugSerial->println(F("Hold throttle stick to upper right corner. Hold turret stick to upper left corner."));
        DebugSerial->println();
        GreenBlinkSlow(3);
        delay(2000);       
        
        // Initialize some variables
        TotThrottlePulse = 0;
        TotTurnPulse = 0;                        
        TotElevationPulse = 0;                    
        TotAzimuthPulse = 0;                
        ThrottleCount = 0;
        TurnCount = 0;
        ElevationCount = 0;
        AzimuthCount = 0;
        
        // Start with no channel reversed
        for (i=0; i<4; i++)
        {
            ch[i]->Settings->reversed = false;    
        }             

        // Start green LED blinking for stage three: three blinks every 1200 ms
        GreenBlinker = StartBlinking_ms(pin_GreenLED, 3, 1200);    // Blip the GreenLED three times every 1200ms
        StartWaiting_sec(3); // For the first bit of time we don't take any readings, this lets the user get the sticks ready
        DebugSerial->print(F("Reading..."));
        //TankSound.ForceBeeps(3);    // Beeps have been removed
        while (!TimeUp)
        { timer.run(); }
        StartWaiting_sec(4); // Now for the next four seconds we check the sticks
        do
        {    
            // Refresh the timer
            timer.run();
            // Get radio commands
            Radio.GetCommands();
            // Add to our readings 
            if (Radio.Sticks.Throttle.updated)  { TotThrottlePulse += Radio.Sticks.Throttle.pulse; ThrottleCount++; }
            if (Radio.Sticks.Turn.updated)      { TotTurnPulse += Radio.Sticks.Turn.pulse; TurnCount++; }
            if (Radio.Sticks.Elevation.updated) { TotElevationPulse += Radio.Sticks.Elevation.pulse; ElevationCount++; }
            if (Radio.Sticks.Azimuth.updated)   { TotAzimuthPulse += Radio.Sticks.Azimuth.pulse; AzimuthCount++; }
        }
        while (!TimeUp);    // Keep looping until time's up
        StopBlinking(GreenBlinker);

        // Get the average of our readings
        TotThrottlePulse /= ThrottleCount;
        TotTurnPulse /= TurnCount;
        TotElevationPulse /= ElevationCount;
        TotAzimuthPulse /= AzimuthCount;
        
        // Throttle stick was held up, should have been long pulse. If not, reverse
        if (TotThrottlePulse < 1200) { Radio.Sticks.Throttle.Settings->reversed = true; }
        // Turn stick was held right, should have been long pulse. If not, reverse
        if (TotTurnPulse < 1200) { Radio.Sticks.Turn.Settings->reversed = true; }
        // Elevation stick was held up, should have been long pulse. If not, reverse
        if (TotElevationPulse < 1200) { Radio.Sticks.Elevation.Settings->reversed = true; }
        // Azimuth stick was held left, should have been short pulse. If not, reverse
        if (TotAzimuthPulse > 1800) { Radio.Sticks.Azimuth.Settings->reversed = true; }
        
        // Save values to EEPROM
        SaveStickSettingsToEEPROM();

        DebugSerial->println();
        DebugSerial->println(F("Stage 3 Results"));
        DumpRadioSticksReversed();

        DebugSerial->println();
        DebugSerial->println(F("STAGE 3 COMPLETE"));
        DebugSerial->println();      
        DebugSerial->println(); 

        // IMPORTANT
        // ------------------------------------------------------------------------------------------------------->
        // If we are using turret stick special positions (such as corners) to trigger functions, we need to adjust 
        // the effective pulse min and max (we increase the min above actual and decrease the max below actual). Then 
        // pulses below the new min or above the new max will be counted as special positions. We do this now, not 
        // earlier, because we don't want these new min/maxes to be saved to eeprom, only to our runtime variables. 
        if (Radio.UsingSpecialPositions) Radio.AdjustTurretStickEndPoints();

        // Transition to Stage End:
        GreenLedOff();
        delay(2000);
        GreenLedOn();
        delay(3000);
        GreenLedOff();

    // End Setup
    DebugSerial->println(F("--- END SETUP ---")); 
    DebugSerial->println();
    RedLedOff();       


    // DUMP DATA TO SERIAL
    // -------------------------------------------------------------------------------------------------------------------------------------------------->
    DumpStickInfo();
    DumpChannelsDetectedUtilized();
}


void SaveStickSettingsToEEPROM(void)
{
    EEPROM.updateBlock(offsetof(_eeprom_data, ThrottleSettings), Radio.Sticks.Throttle.Settings);
    EEPROM.updateBlock(offsetof(_eeprom_data, TurnSettings), Radio.Sticks.Turn.Settings);
    EEPROM.updateBlock(offsetof(_eeprom_data, ElevationSettings), Radio.Sticks.Elevation.Settings);
    EEPROM.updateBlock(offsetof(_eeprom_data, AzimuthSettings), Radio.Sticks.Azimuth.Settings);
}

void DumpRadioStickMinMax(void)
{
    DebugSerial->println(F("Stick: Min - Max pulse values"));
    DebugSerial->print(F("Throttle: "));    DebugSerial->print(Radio.Sticks.Throttle.Settings->pulseMin);    PrintSpaceDash();    DebugSerial->println(Radio.Sticks.Throttle.Settings->pulseMax);
    DebugSerial->print(F("Turn: "));        DebugSerial->print(Radio.Sticks.Turn.Settings->pulseMin);    PrintSpaceDash();    DebugSerial->println(Radio.Sticks.Turn.Settings->pulseMax);
    DebugSerial->print(F("Elevation: "));   DebugSerial->print(Radio.Sticks.Elevation.Settings->pulseMin);    PrintSpaceDash();    DebugSerial->println(Radio.Sticks.Elevation.Settings->pulseMax);
    DebugSerial->print(F("Azimuth: "));     DebugSerial->print(Radio.Sticks.Azimuth.Settings->pulseMin);    PrintSpaceDash();    DebugSerial->println(Radio.Sticks.Azimuth.Settings->pulseMax);
}

void DumpRadioStickCenters(void)
{
    DebugSerial->println(F("Stick: Pulse center value"));
    DebugSerial->print(F("Throttle: "));    DebugSerial->println(Radio.Sticks.Throttle.Settings->pulseCenter);
    DebugSerial->print(F("Turn: "));        DebugSerial->println(Radio.Sticks.Turn.Settings->pulseCenter);
    DebugSerial->print(F("Elevation: "));   DebugSerial->println(Radio.Sticks.Elevation.Settings->pulseCenter);
    DebugSerial->print(F("Azimuth: "));     DebugSerial->println(Radio.Sticks.Azimuth.Settings->pulseCenter);
}

void DumpRadioSticksReversed(void)
{
    DebugSerial->println(F("Stick: Channel reversed"));
    DebugSerial->print(F("Throttle: "));    PrintLnTrueFalse(Radio.Sticks.Throttle.Settings->reversed);
    DebugSerial->print(F("Turn: "));        PrintLnTrueFalse(Radio.Sticks.Turn.Settings->reversed);
    DebugSerial->print(F("Elevation: "));   PrintLnTrueFalse(Radio.Sticks.Elevation.Settings->reversed);
    DebugSerial->print(F("Azimuth: "));     PrintLnTrueFalse(Radio.Sticks.Azimuth.Settings->reversed);
}

