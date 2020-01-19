
void SetupServo(ESC_POS_t servoNum)
{
    #define sideMin false
    #define sideMax true
//    #define absMax SERVO_OUT_MAXPULSE
//    #define absMin SERVO_OUT_MINPULSE
    // The defines above are 2250 and 750 (OP_Servo.h). But I found many servos don't respond to signals at those extremes. 
    // If you go beyond what a servo is capable of, the setup routine is going to croak. So here I use a
    // more conservative range, although the absolute allowed by the servo library will remain the larger range. 
    #define absMax 2100
    #define absMin 900


    boolean isRecoil = false;
    boolean isSteering = false;
    boolean reversed;
    int16_t pulseMin;
    int16_t pulseMax;
    
    Servo_PAN * servo;

    // Before we run setup, stop everything. 
    StopEverything();
    
    // Turn the Red LED on for the duration of setup
    GreenLedOff();
    RedLedOn();
    
    DebugSerial->println();
    PrintDebugLine();
    switch (servoNum)
    {
        case SERVONUM_TURRETELEVATION:
            // Two possible cases for output 4, but either way: 
                servo = new Servo_PAN(SERVONUM_TURRETELEVATION,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                pulseMin = servo->getMinPulseWidth(SERVONUM_TURRETELEVATION);
                pulseMax = servo->getMaxPulseWidth(SERVONUM_TURRETELEVATION);
                isRecoil = false;            
            // First, this could actually be the turret elevation servo as expected
            if (eeprom.ramcopy.TurretElevationMotor == SERVO_ESC || eeprom.ramcopy.TurretElevationMotor == SERVO_PAN)
            {
                reversed = TurretElevation->isReversed();   // Elevation motor object
                isSteering = false; 
                DebugSerial->println(F("Setup Turret Elevation Servo"));
            }
            // Or it could actually be the steering servo assigned to the turret elevation output
            else if (eeprom.ramcopy.DriveType == DT_HALFTRACK && eeprom.ramcopy.DriveMotors == SERVO_ESC)
            {
                reversed = SteeringServo->isReversed();     // Steering motor object
                isSteering = true;
                DebugSerial->println(F("Setup Steering Servo (Output 4)"));
            }
            break;

        case SERVONUM_RIGHTTREAD:
            // If we see the right tread, we are actually using it for steering servo for cars and haltracks with single rear drive
            servo = new Servo_PAN(SERVONUM_RIGHTTREAD,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
            reversed = SteeringServo->isReversed();
            pulseMin = servo->getMinPulseWidth(SERVONUM_RIGHTTREAD);
            pulseMax = servo->getMaxPulseWidth(SERVONUM_RIGHTTREAD);
            isRecoil = false;
            DebugSerial->println(F("Setup Steering Servo (Output 2)"));
            break;

        case SERVONUM_TURRETROTATION:
            servo = new Servo_PAN(SERVONUM_TURRETROTATION,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
            reversed = TurretRotation->isReversed();
            pulseMin = servo->getMinPulseWidth(SERVONUM_TURRETROTATION);
            pulseMax = servo->getMaxPulseWidth(SERVONUM_TURRETROTATION);
            isRecoil = false;
            DebugSerial->println(F("Setup Turret Rotation Pan Servo"));
            break;
            
        case SERVONUM_RECOIL:
            servo = new Servo_PAN(SERVONUM_RECOIL,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
            reversed = RecoilServo->isReversed();
            pulseMin = RecoilServo->getMinPulseWidth(SERVONUM_RECOIL);
            pulseMax = RecoilServo->getMaxPulseWidth(SERVONUM_RECOIL);
            isRecoil = true;
            DebugSerial->println(F("Setup Recoil Servo"));
            break;

        default: 
            DebugSerial->println(F("INVALID SERVO. NO SETUP."));
            return;
    }
    PrintDebugLine();
    DebugSerial->println(F("Start Values"));
    DebugSerial->print(F("Min Pulse: ")); DebugSerial->println(pulseMin);
    DebugSerial->print(F("Max Pulse: ")); DebugSerial->println(pulseMax);
    DebugSerial->print(F("Reversed:  ")); PrintLnTrueFalse(reversed);
    DebugSerial->println();
    
    servo->begin();

    stick_channel * AdjustChannel;
    AdjustChannel = &Radio.Sticks.Elevation;

    stick_channel * SwapChannel;
    SwapChannel = &Radio.Sticks.Turn;

    stick_channel * ReverseChannel;
    ReverseChannel = &Radio.Sticks.Throttle;
    
    boolean SwapSide;
    boolean canSwap = true;
    boolean doSwap = false;
    unsigned long swapTime; 
    const uint8_t swapDelay = 200;

    boolean canReverse = true;
    unsigned long reverseTime; 
    const uint8_t reverseDelay = 200;    

    int awayFromCenter = 200;
    int16_t pulseNow; 

    // Start at the opposite end of where we're at now, as an indication that we've entered the menu. 
    uint16_t curPulse = servo->getPulseWidth(servoNum);
    if (curPulse <= 1500)
    {
        // Move to max
        servo->setMaxPulseWidth(servoNum, absMax);        
        servo->setPos(pulseMax);
        servo->setMinPulseWidth(servoNum, 1500);
        if (reversed) SwapSide = sideMin;
        else SwapSide = sideMax;
    }
    else
    {   // Move to min
        servo->setMinPulseWidth(servoNum, absMin);
        servo->setPos(pulseMin);
        servo->setMaxPulseWidth(servoNum, 1500);              
        if (reversed) SwapSide = sideMax;
        else SwapSide = sideMin;            
    }

    //if (SwapSide == sideMin) Serial.println("Side Min");
    //else Serial.println("Side Max");

    // Loop until they push the button again
    do
    {
        Radio.GetCommands();            
        if (AdjustChannel->updated) 
        {   
            servo->setSpeed(AdjustChannel->command); 
            pulseNow = servo->getPulseWidth(servoNum);
            if (pulseNow <= 1500) { pulseMin = constrain(pulseNow, absMin, 1500);  } // Serial.print(F("New min: ")); Serial.println(pulseMin); }
            else                  { pulseMax = constrain(pulseNow, 1501, absMax);  } // Serial.print(F("New max: ")); Serial.println(pulseMax); }
        }

        // If the "swap" channel is moved to one end or the other, swap the servo over to its other limit
        // But don't allow a second swap swap until after the stick is first returned to neutral and the delay time has expired
        if (!canSwap && SwapChannel->updated && SwapChannel->pulse > (SwapChannel->Settings->pulseCenter - 50) && SwapChannel->pulse < (SwapChannel->Settings->pulseCenter + 50) && millis() >= swapTime) { canSwap = true; }
        if (canSwap && SwapChannel->updated)
        {   // Here we are swapping to the other side. Only do it if the swap stick is held over far enough. 
            if (SwapChannel->pulse < (SwapChannel->Settings->pulseCenter - awayFromCenter) || SwapChannel->pulse > (SwapChannel->Settings->pulseCenter + awayFromCenter))
            {   
                SwapSide = !SwapSide;
                doSwap = true;
            }
        }

        // This does the swap
        if (doSwap)
        {               
            if (SwapSide == sideMax)
            {   //DebugSerial->println(F("Max Side"));
                // sideMax is always "return" of recoil, reversed will just dictate whether that end is pulse min or pulse max
                if (reversed)
                {
                    servo->setMinPulseWidth(servoNum, absMin);
                    if (isRecoil)
                    {
                        servo->setRampSpeed_mS(servoNum, eeprom.ramcopy.RecoilServo_Return_mS, true);
                        do {PerLoopUpdates();}
                        while (servo->PulseWidth() > pulseMin);
                        servo->stopRamping(servoNum);
                    }
                    servo->setPos(pulseMin);
                    servo->setMaxPulseWidth(servoNum, 1500);                     
                }
                else
                {
                    servo->setMaxPulseWidth(servoNum, absMax);        
                    if (isRecoil)
                    {
                        servo->setRampSpeed_mS(servoNum, eeprom.ramcopy.RecoilServo_Return_mS, false);
                        do {PerLoopUpdates();}
                        while (servo->PulseWidth() < pulseMax);
                        servo->stopRamping(servoNum);    
                    }
                    servo->setPos(pulseMax);
                    servo->setMinPulseWidth(servoNum, 1500);
                }
            }
            else
            {   //DebugSerial->println(F("Min Side"));
                // sideMin is always "recoil" of recoil, reversed will just dictate whether that end is pulse min or pulse max
                // We don't need to ramp to the recoil position - just go directly there as fast as possible, same as for regular servo
                if (reversed)
                {
                    servo->setMaxPulseWidth(servoNum, absMax);         
                    servo->setPos(pulseMax);
                    servo->setMinPulseWidth(servoNum, 1500);
                }
                else
                {
                    servo->setMinPulseWidth(servoNum, absMin);
                    servo->setPos(pulseMin);
                    servo->setMaxPulseWidth(servoNum, 1500);                     
                }                
            }
            // Disable swapping for a short period of time
            canSwap = false;
            swapTime = millis() + swapDelay;            
            // Done swapping
            doSwap = false; 
        }

        // If the "reverse" channel is moved to one end or the other, reverse the servo direction
        if (canReverse && ReverseChannel->updated)
        {   // Here we are reversing or un-reversing. Only do it if the reverse stick is held over far enough. 
            if (ReverseChannel->pulse < (ReverseChannel->Settings->pulseCenter - awayFromCenter) || ReverseChannel->pulse > (ReverseChannel->Settings->pulseCenter + awayFromCenter))
            {   
                reversed = !reversed;

                // Green LED blinks to indicate whether it was reversed or un-reversed
                if (reversed)
                {
                    DebugSerial->println(F("Servo reversed"));
                    GreenBlinkFast(2);
                }
                else 
                {
                    DebugSerial->println(F("Servo un-reversed"));
                    GreenBlinkFast(1);
                }

                // Disable reversing for a short period of time
                canReverse = false; 
                reverseTime = millis() + reverseDelay;           
                
                // Reverse the actual motor object (sets a flag)
                servo->set_Reversed(reversed);
                
                // Whenever we reverse, swap the servo position as well
                doSwap = true;
            }
        }
        // After we reverse, don't allow a second reverse until after the stick is first returned to neutral
        //if (!canReverse && ReverseChannel->updated && ReverseChannel->pulse > (ReverseChannel->Settings->pulseCenter - 50) && ReverseChannel->pulse < (ReverseChannel->Settings->pulseCenter + 50) && millis() >= reverseTime ) { canReverse = true; }
        if (!canReverse && ReverseChannel->updated && ReverseChannel->pulse > (ReverseChannel->Settings->pulseCenter - 50) && ReverseChannel->pulse < (ReverseChannel->Settings->pulseCenter + 50)) { canReverse = true; }
        
        Radio.Update();         // Update radio
        InputButton.read();     // Read the input button
        timer.run();            // Update timer

    } while (!InputButton.wasPressed());

    // Now wait for the release, so it doesn't trigger a release in the main sketch
    do {InputButton.read();}
    while (!InputButton.wasReleased());

    // Now save settings
    switch (servoNum)
    {
        case SERVONUM_TURRETELEVATION:  // This could be elevation or steering
            // Save the global variable
            if (isSteering)
            {
                eeprom.ramcopy.SteeringServo_EPMin = pulseMin;
                eeprom.ramcopy.SteeringServo_EPMax = pulseMax;
                eeprom.ramcopy.SteeringServo_Reversed = reversed;    
                // Update eeprom too so it's permanent
                EEPROM.updateInt(offsetof(_eeprom_data, SteeringServo_EPMin), pulseMin);
                EEPROM.updateInt(offsetof(_eeprom_data, SteeringServo_EPMax), pulseMax);
                EEPROM.updateByte(offsetof(_eeprom_data, SteeringServo_Reversed), reversed);
            }
            else
            {
                eeprom.ramcopy.TurretElevation_EPMin = pulseMin;
                eeprom.ramcopy.TurretElevation_EPMax = pulseMax;
                eeprom.ramcopy.TurretElevation_Reversed = reversed;
                // Update eeprom too so it's permanent
                EEPROM.updateInt(offsetof(_eeprom_data, TurretElevation_EPMin), pulseMin);
                EEPROM.updateInt(offsetof(_eeprom_data, TurretElevation_EPMax), pulseMax);
                EEPROM.updateByte(offsetof(_eeprom_data, TurretElevation_Reversed), reversed);
            }
            // Finally, set the actual end-point limits to the servo class
            servo->setMinPulseWidth(SERVONUM_TURRETELEVATION, pulseMin);
            servo->setMaxPulseWidth(SERVONUM_TURRETELEVATION, pulseMax);
            if (isSteering)
            {
                // And in this case, we use the reversed function of the motor class
                SteeringServo->set_Reversed(reversed);
                // We also want to exit with the servo put back exactly to center
                servo->writeMicroseconds(servoNum, 1500);
                delay(150); // Give it time to get there
                // This is a regular servo. The stop command will put it to center. 
                SteeringServo->stop();                
            }
            else
            {
                // And in this case, we use the reversed function of the motor class
                TurretElevation->set_Reversed(reversed);
                // Set it to the duplicate Barrel object too if we have one of those
                if (eeprom.ramcopy.TurretElevationMotor == SERVO_PAN)
                { 
                    Barrel->set_Reversed(reversed); 
                }
                // We also want to exit with the servo put back exactly to center
                servo->writeMicroseconds(servoNum, 1500);
                delay(150); // Give it time to get there
                if (eeprom.ramcopy.TurretElevationMotor == SERVO_PAN)
                {   // We want to save the position so the barrel stabilizer knows where to start
                    // We start moving just to set the canSetFixedPos flag, then immediately stop
                    // to record the positionm, which should probably still be 1500
                    pulseNow = servo->getPulseWidth(servoNum);
                    if (pulseNow > 1500) TurretElevation->setSpeed(-1);
                    else                 TurretElevation->setSpeed(1);
                    TurretElevation->setSpeed(0);
                }
                else
                {   // This is a regular servo. The stop command will put it to center. 
                    TurretElevation->stop();
                }
            }
            break;

        case SERVONUM_RIGHTTREAD:   // This is the steering servo
            // Save the global variable
            eeprom.ramcopy.SteeringServo_EPMin = pulseMin;
            eeprom.ramcopy.SteeringServo_EPMax = pulseMax;
            eeprom.ramcopy.SteeringServo_Reversed = reversed;
            // Update eeprom too so it's permanent
            EEPROM.updateInt(offsetof(_eeprom_data, SteeringServo_EPMin), pulseMin);
            EEPROM.updateInt(offsetof(_eeprom_data, SteeringServo_EPMax), pulseMax);
            EEPROM.updateByte(offsetof(_eeprom_data, SteeringServo_Reversed), reversed);
            // Finally, set the actual end-point limits to the servo class
            servo->setMinPulseWidth(SERVONUM_RIGHTTREAD, pulseMin);
            servo->setMaxPulseWidth(SERVONUM_RIGHTTREAD, pulseMax);
            // And in this case, we use the reversed function of the motor class
            SteeringServo->set_Reversed(reversed);
            // We also want to exit with the servo put back exactly to center
            servo->writeMicroseconds(servoNum, 1500);
            delay(150); // Give it time to get there
            // This is a regular servo. The stop command will put it to center. 
            SteeringServo->stop();
            break;

        case SERVONUM_TURRETROTATION:
            // Save the global variable
            eeprom.ramcopy.TurretRotation_EPMin = pulseMin;
            eeprom.ramcopy.TurretRotation_EPMax = pulseMax;
            eeprom.ramcopy.TurretRotation_Reversed = reversed;
            // Update eeprom too so it's permanent
            EEPROM.updateInt(offsetof(_eeprom_data, TurretRotation_EPMin), pulseMin);
            EEPROM.updateInt(offsetof(_eeprom_data, TurretRotation_EPMax), pulseMax);
            EEPROM.updateByte(offsetof(_eeprom_data, TurretRotation_Reversed), reversed);
            // Finally, set the actual end-point limits to the servo class
            servo->setMinPulseWidth(SERVONUM_TURRETROTATION, pulseMin);
            servo->setMaxPulseWidth(SERVONUM_TURRETROTATION, pulseMax);
            // And in this case, we use the reversed function of the motor class
            TurretRotation->set_Reversed(reversed);
            // We also want to exit with the servo put back exactly to center
            servo->writeMicroseconds(servoNum, 1500);
            delay(150); // Give it time to get there
            if (eeprom.ramcopy.TurretRotationMotor == SERVO_PAN)
            {   // We want to save the position so the barrel stabilizer knows where to start.
                // EDIT - this is not rotation servo, we have no stabilization on it yet. I am still
                // leaving this bit in for now as it only puts us back to center. 
                // We start moving just to set the canSetFixedPos flag, then immediately stop
                // to record the positionm, which should probably still be 1500
                pulseNow = servo->getPulseWidth(servoNum);
                if (pulseNow > 1500) TurretRotation->setSpeed(-1);
                else                 TurretRotation->setSpeed(1);
                TurretRotation->setSpeed(0);
            }
            else
            {   // This is a regular servo. The stop command will put it to center. 
                TurretRotation->stop();
            }
            break;
            
        case SERVONUM_RECOIL:
            // Save the global variable
            eeprom.ramcopy.RecoilServo_EPMin = pulseMin;
            eeprom.ramcopy.RecoilServo_EPMax = pulseMax;
            eeprom.ramcopy.RecoilReversed = reversed;
            // Update eeprom too so it's permanent
            EEPROM.updateInt(offsetof(_eeprom_data, RecoilServo_EPMin), pulseMin);
            EEPROM.updateInt(offsetof(_eeprom_data, RecoilServo_EPMax), pulseMax);
            EEPROM.updateByte(offsetof(_eeprom_data, RecoilReversed), reversed);
            // Finally, set the actual end-point limits to the servo class
            servo->setMinPulseWidth(SERVONUM_RECOIL, pulseMin);
            servo->setMaxPulseWidth(SERVONUM_RECOIL, pulseMax);
            // The reversed setting needs to be applied both to the motor class (flag) as well as to the servo class (actual recoil movement settings). 
            RecoilServo->set_Reversed(eeprom.ramcopy.RecoilReversed);                       // motor class method
            RecoilServo->setRecoilReversed(SERVONUM_RECOIL, eeprom.ramcopy.RecoilReversed); // servo class method
            // Let's make sure we put the recoil servo back to battery on exit, since it's possible they exited while adjusting the recoil side           
            pulseNow = servo->getPulseWidth(servoNum);
            if ((eeprom.ramcopy.RecoilReversed && pulseNow != servo->getMinPulseWidth(servoNum)) || (!eeprom.ramcopy.RecoilReversed && pulseNow != servo->getMaxPulseWidth(servoNum)))
            {   // The recoil servo is not at battery. Let's return it. 
                if (eeprom.ramcopy.RecoilReversed)
                {
                    servo->setRampSpeed_mS(servoNum, eeprom.ramcopy.RecoilServo_Return_mS, true);
                    do {PerLoopUpdates();}
                    while (servo->PulseWidth() > pulseMin);
                    servo->stopRamping(servoNum);
                    servo->setPos(pulseMin);
                }
                else
                {
                    servo->setRampSpeed_mS(servoNum, eeprom.ramcopy.RecoilServo_Return_mS, false);
                    do {PerLoopUpdates();}
                    while (servo->PulseWidth() < pulseMax);
                    servo->stopRamping(servoNum);    
                    servo->setPos(pulseMax);
                }
            }
            break;
    }
    DebugSerial->println();
    PrintDebugLine();
    switch (servoNum) 
    {   case SERVONUM_TURRETELEVATION:
            if (eeprom.ramcopy.TurretElevationMotor == SERVO_ESC || eeprom.ramcopy.TurretElevationMotor == SERVO_PAN) DebugSerial->println(F("End Elevation Servo Setup")); 
            else if (eeprom.ramcopy.DriveType == DT_HALFTRACK && eeprom.ramcopy.DriveMotors == SERVO_ESC)             DebugSerial->println(F("End Steering Servo Setup"));
            break;
        case SERVONUM_RIGHTTREAD:       DebugSerial->println(F("End Steering Servo Setup")); break;
        case SERVONUM_TURRETROTATION:   DebugSerial->println(F("End Rotation Servo Setup")); break;
        case SERVONUM_RECOIL:           DebugSerial->println(F("End Recoil Servo Setup"));   break;      
        default:                                                                             break;
    }
    PrintDebugLine();
    DebugSerial->println(F("End Values"));
    DebugSerial->print(F("Min Pulse: ")); DebugSerial->println(pulseMin);
    DebugSerial->print(F("Max Pulse: ")); DebugSerial->println(pulseMax);
    DebugSerial->print(F("Reversed:  ")); PrintLnTrueFalse(reversed);
    DebugSerial->println();
    
    RedLedOff();
}


