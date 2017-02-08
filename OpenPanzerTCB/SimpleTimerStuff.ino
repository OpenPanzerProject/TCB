
// This gets called each time through the main loop. If we have
// anything that needs to be continuously polled, put it here. 
void PerLoopUpdates(void)
{
    InputButton.read();     // Read the input button
    SetActiveCommPort();    // Check Dipswitch #5 and set the communication port to USB (switch On) or Serial 1 (switch Off)
    UpdateSimpleTimers();   // Update timers
}

void UpdateSimpleTimers()
{
    timer.run();                        // Our simple timer object, used all over the place including by various libraries.  
    Radio.Update();                     // Radio update (polls SBus and iBus)
    TankEngine.UpdateTimer();           // Engine timer (see OP_Driver library)
    TankTransmission.UpdateTimer();     // Transmission timer (see OP_Driver library)
    
    // Now we also update the four motor objects. The motor update() routines will only do something if the motor type is a serial controller. 
    // We can use this to force serial commands be sent at set intervals even if the command hasn't changed; this keeps us from tripping the serial 
    // watchdog that for example the Scout ESC implements. 
    switch (eeprom.ramcopy.DriveType)
    {
        case DT_TANK:       { RightTread->update(); LeftTread->update();     } break;
        case DT_HALFTRACK:  { RightTread->update(); LeftTread->update();     } break;
        case DT_CAR:        { DriveMotor->update();                          } break;
        case DT_DKLM:       { DriveMotor->update(); SteeringMotor->update(); } break;
        default:                                                               break;
    }    

    TurretRotation->update();
    TurretElevation->update();

    // We also update the smoker object because it can have special effects that require polling
    Smoker->update();
}


int StartBlinking_ms(int LED, int BlinkTimes, int ms)
{
    switch (BlinkTimes)
    {
        case 1:
            if (LED == pin_GreenLED) { return timer.setInterval(ms, GreenBlinkOne); }
            if (LED == pin_RedLED)   { return timer.setInterval(ms, RedBlinkOne);   }
            break;
        case 2:
            if (LED == pin_GreenLED) { return timer.setInterval(ms, GreenBlinkTwo); }
            if (LED == pin_RedLED)   { return timer.setInterval(ms, RedBlinkTwo);   }
            break;
        case 3:
            if (LED == pin_GreenLED) { return timer.setInterval(ms, GreenBlinkThree); }
            if (LED == pin_RedLED)   { return timer.setInterval(ms, RedBlinkThree);   }
            break;
        default:
            break;        
    }
}

void StopBlinking(int TimerID)
{
    timer.deleteTimer(TimerID);
}

int StartWaiting_mS(long mS)
{
    TimeUp = false;
    return timer.setTimeout(mS, SetTimeUp);    // will call function once after ms duration
}

int StartWaiting_sec(long seconds)
{
    return StartWaiting_mS(seconds*1000);
}


void SetTimeUp()
{
    TimeUp = true;
}

