// Special Functions - functions that can be mapped by the user to one of the numerous triggers

// The function definitions are not on this tab, but spread about. This is simply where they are assigned to the SF_Callback array.
// We also have "fake" functions for some that re-direct to the real function but without any parameters passed. 


// ----------------------------------------------------------------------------------------------------------------------------------------------->>
// LOAD SPECIAL FUNCTIONS INTO SF_CALLBACK ARRAY
// ----------------------------------------------------------------------------------------------------------------------------------------------->>
void LoadFunctionTriggers()
{
    // Here we setup the special functions list. The user can create up to MAX_FUNCTION_TRIGGERS (40 for now) pairs of Triggers-to-Functions.
    // Here we save the callback function address for each trigger. 
    for (int i = 0; i <MAX_FUNCTION_TRIGGERS; i++)
    {   // A valid function-trigger will have a function number and a TriggerID > 0
        if (eeprom.ramcopy.SF_Trigger[i].specialFunction != SF_NULL_FUNCTION && eeprom.ramcopy.SF_Trigger[i].TriggerID > 0)
        {
            //DebugSerial->print(F("TriggerID: "));
            //DebugSerial->print(eeprom.ramcopy.SF_Trigger[i].TriggerID);
            //DebugSerial->print(F(" Function #: "));
            //DebugSerial->println(eeprom.ramcopy.SF_Trigger[i].specialFunction);
            
            // Assign the callback function to the same index in the FunctionTrigger array
            // that the function definition occupies in our eeprom.ramcopy.SF_Trigger array
            switch (eeprom.ramcopy.SF_Trigger[i].specialFunction)
            {
                case SF_ENGINE_TOGGLE:              SF_Callback[i] = &SF_EngineToggle;                              break;
                case SF_ENGINE_ON:                  SF_Callback[i] = &SF_EngineOn;                                  break; 
                case SF_ENGINE_OFF:                 SF_Callback[i] = &SF_EngineOff;                                 break;
                case SF_TRANS_TOGGLE:               SF_Callback[i] = &SF_TransmissionToggle;                        break;
                case SF_TRANS_ON:                   SF_Callback[i] = &SF_TransmissionEngage;                        break;
                case SF_TRANS_OFF:                  SF_Callback[i] = &SF_TransmissionDisengage;                     break;
                case SF_CANNON_FIRE:                SF_Callback[i] = &SF_FireCannon;                                break;
                case SF_MECH_BARREL:                SF_Callback[i] = &SF_MechBarrel;                                break;
                case SF_RECOIL_SERVO:               SF_Callback[i] = &SF_RecoilServo;                               break;
                case SF_HI_FLASH:                   SF_Callback[i] = &SF_HiFlash;                                   break;
                case SF_MG_FIRE:                    SF_Callback[i] = &SF_MG_Start;                                  break;
                case SF_MG_OFF:                     SF_Callback[i] = &SF_MG_Stop;                                   break;
                case SF_MG2_FIRE:                   SF_Callback[i] = &SF_MG2_Start;                                 break;
                case SF_MG2_OFF:                    SF_Callback[i] = &SF_MG2_Stop;                                  break;
                case SF_BARREL_ENABLE:              SF_Callback[i] = &SF_MechBarrel_Enable;                         break;
                case SF_BARREL_DISABLE:             SF_Callback[i] = &SF_MechBarrel_Disable;                        break;
                case SF_BARREL_TOGGLE:              SF_Callback[i] = &SF_MechBarrel_Toggle;                         break;
                case SF_IR_ENABLE:                  SF_Callback[i] = &SF_IR_Enable;                                 break;
                case SF_IR_DISABLE:                 SF_Callback[i] = &SF_IR_Disable;                                break;
                case SF_IR_TOGGLE:                  SF_Callback[i] = &SF_IR_Toggle;                                 break;
                case SF_LIGHT1_TOGGLE:              SF_Callback[i] = &SF_Light1Toggle;                              break;
                case SF_LIGHT1_ON:                  SF_Callback[i] = &SF_Light1On;                                  break;
                case SF_LIGHT1_OFF:                 SF_Callback[i] = &SF_Light1Off;                                 break;
                case SF_LIGHT2_TOGGLE:              SF_Callback[i] = &SF_Light2Toggle;                              break;
                case SF_LIGHT2_ON:                  SF_Callback[i] = &SF_Light2On;                                  break;
                case SF_LIGHT2_OFF:                 SF_Callback[i] = &SF_Light2Off;                                 break;                
                case SF_RUNNINGLIGHTS_TOGGLE:       SF_Callback[i] = &SF_RunningLightsToggle;                       break;
                case SF_RUNNINGLIGHTS_ON:           SF_Callback[i] = &SF_RunningLightsOn;                           break;
                case SF_RUNNINGLIGHTS_OFF:          SF_Callback[i] = &SF_RunningLightsOff;                          break;
                case SF_AUXOUT_TOGGLE:              SF_Callback[i] = &SF_AuxOutputToggle;                           break;
                case SF_AUXOUT_ON:                  SF_Callback[i] = &SF_AuxOutputOn;                               break;
                case SF_AUXOUT_OFF:                 SF_Callback[i] = &SF_AuxOutputOff;                              break;
                case SF_AUXOUT_LEVEL:               SF_Callback[i] = &AuxOutput_SetLevel;                           break; // Analog function
                case SF_AUXOUT_PRESETDIM:           SF_Callback[i] = &SF_AuxOutput_PresetDim;                       break;
                case SF_AUXOUT_TOGGLEDIM:           SF_Callback[i] = &SF_AuxOutput_ToggleDim;                       break;
                case SF_AUXOUT_FLASH:               SF_Callback[i] = &SF_AuxOutputFlash;                            break;
                case SF_AUXOUT_INV_FLASH:           SF_Callback[i] = &SF_AuxOutputInverseFlash;                     break;
                case SF_AUXOUT_BLINK:               SF_Callback[i] = &SF_AuxOutputBlink;                            break;
                case SF_AUXOUT_TOGGLEBLINK:         SF_Callback[i] = &SF_AuxOutputToggleBlink;                      break;
                case SF_AUXOUT_REVOLVE:             SF_Callback[i] = &SF_AuxOutputRevolve;                          break;
                case SF_AUXOUT_TOGGLEREVOLVE:       SF_Callback[i] = &SF_AuxOutputToggleRevolve;                    break;                    
                case SF_SET_VOLUME:                 SF_Callback[i] = &SetVolume;                                    break; // Analog function
                case SF_OVERLAY_ENABLE:             SF_Callback[i] = &SF_OverlayEnable;                             break;
                case SF_OVERLAY_DISABLE:            SF_Callback[i] = &SF_OverlayDisable;                            break;
                case SF_USER_SOUND1_ONCE:           SF_Callback[i] = &SF_TriggerUserSound1;                         break;
                case SF_USER_SOUND1_RPT:            SF_Callback[i] = &SF_UserSound1_Repeat;                         break;
                case SF_USER_SOUND1_OFF:            SF_Callback[i] = &SF_UserSound1_Stop;                           break;
                case SF_USER_SOUND2_ONCE:           SF_Callback[i] = &SF_TriggerUserSound2;                         break;
                case SF_USER_SOUND2_RPT:            SF_Callback[i] = &SF_UserSound2_Repeat;                         break;
                case SF_USER_SOUND2_OFF:            SF_Callback[i] = &SF_UserSound2_Stop;                           break;
                case SF_USER_SOUND3_ONCE:           SF_Callback[i] = &SF_TriggerUserSound3;                         break;
                case SF_USER_SOUND3_RPT:            SF_Callback[i] = &SF_UserSound3_Repeat;                         break;
                case SF_USER_SOUND3_OFF:            SF_Callback[i] = &SF_UserSound3_Stop;                           break;
                case SF_USER_SOUND4_ONCE:           SF_Callback[i] = &SF_TriggerUserSound4;                         break;
                case SF_USER_SOUND4_RPT:            SF_Callback[i] = &SF_UserSound4_Repeat;                         break;
                case SF_USER_SOUND4_OFF:            SF_Callback[i] = &SF_UserSound4_Stop;                           break;
                case SF_USER_SOUND5_ONCE:           SF_Callback[i] = &SF_TriggerUserSound5;                         break;
                case SF_USER_SOUND5_RPT:            SF_Callback[i] = &SF_UserSound5_Repeat;                         break;
                case SF_USER_SOUND5_OFF:            SF_Callback[i] = &SF_UserSound5_Stop;                           break;
                case SF_USER_SOUND6_ONCE:           SF_Callback[i] = &SF_TriggerUserSound6;                         break;
                case SF_USER_SOUND6_RPT:            SF_Callback[i] = &SF_UserSound6_Repeat;                         break;
                case SF_USER_SOUND6_OFF:            SF_Callback[i] = &SF_UserSound6_Stop;                           break;
                case SF_USER_SOUND7_ONCE:           SF_Callback[i] = &SF_TriggerUserSound7;                         break;
                case SF_USER_SOUND7_RPT:            SF_Callback[i] = &SF_UserSound7_Repeat;                         break;
                case SF_USER_SOUND7_OFF:            SF_Callback[i] = &SF_UserSound7_Stop;                           break;
                case SF_USER_SOUND8_ONCE:           SF_Callback[i] = &SF_TriggerUserSound8;                         break;
                case SF_USER_SOUND8_RPT:            SF_Callback[i] = &SF_UserSound8_Repeat;                         break;
                case SF_USER_SOUND8_OFF:            SF_Callback[i] = &SF_UserSound8_Stop;                           break;
                case SF_USER_SOUND9_ONCE:           SF_Callback[i] = &SF_TriggerUserSound9;                         break;
                case SF_USER_SOUND9_RPT:            SF_Callback[i] = &SF_UserSound9_Repeat;                         break;
                case SF_USER_SOUND9_OFF:            SF_Callback[i] = &SF_UserSound9_Stop;                           break;
                case SF_USER_SOUND10_ONCE:          SF_Callback[i] = &SF_TriggerUserSound10;                        break;
                case SF_USER_SOUND10_RPT:           SF_Callback[i] = &SF_UserSound10_Repeat;                        break;
                case SF_USER_SOUND10_OFF:           SF_Callback[i] = &SF_UserSound10_Stop;                          break;
                case SF_USER_SOUND11_ONCE:          SF_Callback[i] = &SF_TriggerUserSound11;                        break;
                case SF_USER_SOUND11_RPT:           SF_Callback[i] = &SF_UserSound11_Repeat;                        break;
                case SF_USER_SOUND11_OFF:           SF_Callback[i] = &SF_UserSound11_Stop;                          break;
                case SF_USER_SOUND12_ONCE:          SF_Callback[i] = &SF_TriggerUserSound12;                        break;
                case SF_USER_SOUND12_RPT:           SF_Callback[i] = &SF_UserSound12_Repeat;                        break;
                case SF_USER_SOUND12_OFF:           SF_Callback[i] = &SF_UserSound12_Stop;                          break;
                case SF_USER_SOUND_ALL_OFF:         SF_Callback[i] = &SF_UserSound_Stop_All;                        break;
                case SF_SBA_PLAYSTOP:               SF_Callback[i] = &SF_Soundbank_A_PlayStop;                      break;
                case SF_SBA_NEXT:                   SF_Callback[i] = &SF_Soundbank_A_Next;                          break;
                case SF_SBA_PREVIOUS:               SF_Callback[i] = &SF_Soundbank_A_Previous;                      break;
                case SF_SBA_RANDOM:                 SF_Callback[i] = &SF_Soundbank_A_Random;                        break;
                case SF_SBB_PLAYSTOP:               SF_Callback[i] = &SF_Soundbank_B_PlayStop;                      break;
                case SF_SBB_NEXT:                   SF_Callback[i] = &SF_Soundbank_B_Next;                          break;
                case SF_SBB_PREVIOUS:               SF_Callback[i] = &SF_Soundbank_B_Previous;                      break;
                case SF_SBB_RANDOM:                 SF_Callback[i] = &SF_Soundbank_B_Random;                        break;                
                case SF_OUTPUT_A_TOGGLE:            SF_Callback[i] = &SF_PortA_Toggle;                              break;
                case SF_OUTPUT_A_ON:                SF_Callback[i] = &SF_PortA_On;                                  break; 
                case SF_OUTPUT_A_OFF:               SF_Callback[i] = &SF_PortA_Off;                                 break; 
                case SF_OUTPUT_A_PULSE:             SF_Callback[i] = &SF_PortA_Pulse;                               break;
                case SF_OUTPUT_A_BLINK:             SF_Callback[i] = &SF_PortA_Blink;                               break;
                case SF_OUTPUT_B_TOGGLE:            SF_Callback[i] = &SF_PortB_Toggle;                              break;
                case SF_OUTPUT_B_ON:                SF_Callback[i] = &SF_PortB_On;                                  break; 
                case SF_OUTPUT_B_OFF:               SF_Callback[i] = &SF_PortB_Off;                                 break; 
                case SF_OUTPUT_B_PULSE:             SF_Callback[i] = &SF_PortB_Pulse;                               break;                                
                case SF_OUTPUT_B_BLINK:             SF_Callback[i] = &SF_PortB_Blink;                               break;                
                case SF_ACCEL_LEVEL:                SF_Callback[i] = &SF_SetAccelRampFreq;                          break;         
                case SF_DECEL_LEVEL:                SF_Callback[i] = &SF_SetDecelRampFreq;                          break;
                case SF_TURNMODE_1:                 SF_Callback[i] = &SF_TurnMode1;                                 break;                    
                case SF_TURNMODE_2:                 SF_Callback[i] = &SF_TurnMode2;                                 break;                    
                case SF_TURNMODE_3:                 SF_Callback[i] = &SF_TurnMode3;                                 break;                    
                case SF_SMOKER:                     SF_Callback[i] = &Smoker_ManualControl;                         break; // Analog function for controlling speed of smoker output (could be a motor or light or whatever)
                case SF_SMOKER_ON:                  SF_Callback[i] = &SF_Smoker_ManualOn;                           break; // Manual control of smoker output
                case SF_SMOKER_OFF:                 SF_Callback[i] = &SF_Smoker_ManualOff;                          break; // Manual control of smoker output
                case SF_SMOKER_MANTOGGLE:           SF_Callback[i] = &SF_Smoker_ManualToggle;                       break; // Manual control of smoker output
                case SF_SMOKE_PREHEAT_ON:           SF_Callback[i] = &SF_Smoker_PreheatEnable;                      break;
                case SF_SMOKE_PREHEAT_OFF:          SF_Callback[i] = &SF_Smoker_PreheatDisable;                     break;
                case SF_SMOKE_PREHEAT_TOGGLE:       SF_Callback[i] = &SF_Smoker_PreheatToggle;                      break;
                case SF_MOTOR_A:                    SF_Callback[i] = &MotorA_ManualControl;                         break; // Analog function
                case SF_MOTOR_B:                    SF_Callback[i] = &MotorB_ManualControl;                         break; // Analog function
                case SF_MOTOR_A_ON:                 SF_Callback[i] = &MotorA_On;                                    break;
                case SF_MOTOR_A_OFF:                SF_Callback[i] = &MotorA_Off;                                   break;
                case SF_MOTOR_A_TOGGLE:             SF_Callback[i] = &MotorA_Toggle;                                break;
                case SF_MOTOR_B_ON:                 SF_Callback[i] = &MotorB_On;                                    break;
                case SF_MOTOR_B_OFF:                SF_Callback[i] = &MotorB_Off;                                   break;
                case SF_MOTOR_B_TOGGLE:             SF_Callback[i] = &MotorB_Toggle;                                break;                
                case SF_RC1_PASS:                   SF_Callback[i] = &RC_Passthrough_1;                             break; // Analog function
                case SF_RC2_PASS:                   SF_Callback[i] = &RC_Passthrough_2;                             break; // Analog function
                case SF_RC3_PASS:                   SF_Callback[i] = &RC_Passthrough_3;                             break; // Analog function
                case SF_RC4_PASS:                   SF_Callback[i] = &RC_Passthrough_4;                             break; // Analog function // No passthrough for 5 - that is always reserved for recoil
                case SF_RC6_PASS:                   SF_Callback[i] = &RC_Passthrough_6;                             break; // Analog function
                case SF_RC7_PASS:                   SF_Callback[i] = &RC_Passthrough_7;                             break; // Analog function
                case SF_RC8_PASS:                   SF_Callback[i] = &RC_Passthrough_8;                             break; // Analog function
                case SF_RC1_PASS_PAN:               SF_Callback[i] = &RC_PanServo_1;                                break; // Analog function
                case SF_RC2_PASS_PAN:               SF_Callback[i] = &RC_PanServo_2;                                break; // Analog function
                case SF_RC3_PASS_PAN:               SF_Callback[i] = &RC_PanServo_3;                                break; // Analog function
                case SF_RC4_PASS_PAN:               SF_Callback[i] = &RC_PanServo_4;                                break; // Analog function  // No pan for 5 - that is always reserved for recoil
                case SF_RC6_PASS_PAN:               SF_Callback[i] = &RC_PanServo_6;                                break; // Analog function
                case SF_RC7_PASS_PAN:               SF_Callback[i] = &RC_PanServo_7;                                break; // Analog function
                case SF_RC8_PASS_PAN:               SF_Callback[i] = &RC_PanServo_8;                                break; // Analog function
                case SF_USER_FUNC_1:                SF_Callback[i] = &SF_UserFunc1;                                 break;
                case SF_USER_FUNC_2:                SF_Callback[i] = &SF_UserFunc2;                                 break;                    
                case SF_USER_ANLG_1:                SF_Callback[i] = &User_Analog_Function1;                        break; // Analog function
                case SF_USER_ANLG_2:                SF_Callback[i] = &User_Analog_Function2;                        break; // Analog function
                case SF_DUMP_DEBUG:                 SF_Callback[i] = &SF_DumpDebug;                                 break;
                case SF_NT_ENABLE:                  SF_Callback[i] = &SF_NT_Enable;                                 break;
                case SF_NT_DISABLE:                 SF_Callback[i] = &SF_NT_Disable;                                break;
                case SF_NT_TOGGLE:                  SF_Callback[i] = &SF_NT_Toggle;                                 break;
                case SF_DRIVEPROFILE_1:             SF_Callback[i] = &SF_DriveProfile_1;                            break;
                case SF_DRIVEPROFILE_2:             SF_Callback[i] = &SF_DriveProfile_2;                            break;
                case SF_DRIVEPROFILE_TOGGLE:        SF_Callback[i] = &SF_DriveProfile_Toggle;                       break;
                case SF_SPEED_75:                   SF_Callback[i] = &SF_ReduceSpeed_75;                            break;
                case SF_SPEED_50:                   SF_Callback[i] = &SF_ReduceSpeed_50;                            break;
                case SF_SPEED_25:                   SF_Callback[i] = &SF_ReduceSpeed_25;                            break;
                case SF_SPEED_RESTORE:              SF_Callback[i] = &SF_RestoreSpeed;                              break;
                case SF_SMOKER_ENABLE:              SF_Callback[i] = &SF_Smoker_Enable;                             break; // Control of auto smoker function
                case SF_SMOKER_DISABLE:             SF_Callback[i] = &SF_Smoker_Disable;                            break; // Control of auto smoker function
                case SF_SMOKER_TOGGLE:              SF_Callback[i] = &SF_Smoker_Toggle;                             break; // Control of auto smoker function
                case SF_INCR_VOLUME:                SF_Callback[i] = &SF_IncreaseVolume;                            break;
                case SF_DECR_VOLUME:                SF_Callback[i] = &SF_DecreaseVolume;                            break;
                case SF_STOP_VOLUME:                SF_Callback[i] = &SF_StopVolume;                                break;
                case SF_MANTRANS_FWD:               SF_Callback[i] = &SF_ManualTransForward;                        break;
                case SF_MANTRANS_REV:               SF_Callback[i] = &SF_ManualTransReverse;                        break;
                case SF_MANTRANS_NEUTRAL:           SF_Callback[i] = &SF_ManualTransNeutral;                        break;
            }
        }
    }
}

uint8_t CountTriggers(void)
{
    uint8_t tCount = 0;
    
    // A valid function-trigger will have a function number and a TriggerID > 0
    for (int i = 0; i <MAX_FUNCTION_TRIGGERS; i++)
    {   
        if (eeprom.ramcopy.SF_Trigger[i].specialFunction != SF_NULL_FUNCTION && eeprom.ramcopy.SF_Trigger[i].TriggerID > 0) tCount += 1;
    }
    return tCount; 
}

boolean isFunctionAssigned(_special_function f)
{
    boolean does_exist = false; 

    // We want to know if the passed function f is being used, meaning,it exists in the special function array and a valid trigger has been assigned to it. 
    for (int i = 0; i <MAX_FUNCTION_TRIGGERS; i++)
    {   
        if (eeprom.ramcopy.SF_Trigger[i].specialFunction == f && eeprom.ramcopy.SF_Trigger[i].TriggerID > 0) 
        {
            does_exist = true;
            break;
        }
    }
    return does_exist;
}

// ----------------------------------------------------------------------------------------------------------------------------------------------->>
// CONVENIENCE FUNCTION - GET TRIGGER NAME FROM TRIGGER ID
// ----------------------------------------------------------------------------------------------------------------------------------------------->>
void PrintTriggerDescription(_special_function sf, uint16_t TriggerID)
{
    // We add spaces at the end of each description so they will all always equal the same number of characters.
    // With a fixed space font this makes them easier to read when printed. Set this constant to the length of
    // the longest decription, and any description shorter than that will have the appropriate number of spaces added. 
    const uint8_t PadLength = 29;
    
    uint8_t TriggerAction = 0;

    // Is the selected function digital or analog:
    boolean sf_digital = isSpecialFunctionDigital(sf);
    
    // Turret stick triggers
    if (TriggerID > 0 && TriggerID <= MAX_SPEC_POS)
    {   
        // Trigger action in this case is equal to the Trigger ID
        DebugSerial->print(F("Turret Stick - "));
        DebugSerial->print(TurretStickPosition(TriggerID));
        PrintSpaces(PadLength - (TS_PositionString_Length(TurretStickPositionArrayNumFromTriggerID(TriggerID)) + 15));   // TS_PositionString_Length defined in OP_Radio.h. TurretStickPositionArrayNumFromTriggerID is defined below. 
    }
    // External I/O source. The Trigger Source is the integer value of the division by the ports multiplier (discard remainder)
    else if (TriggerID >= trigger_id_multiplier_ports && TriggerID < trigger_id_multiplier_auxchannel)
    {
        int portNum = TriggerID / trigger_id_multiplier_ports;
        DebugSerial->print(F("External Input "));
        switch (portNum)
        {   case 1: DebugSerial->print(F("A")); break;
            case 2: DebugSerial->print(F("B")); break;
        }
        PrintSpaceDash();
        // Up to this point we now have something like "External Input A - "
        if (sf_digital)
        {
            // The remainder is the action (on/off if digital, or variable if analog)
            TriggerAction = getTriggerActionFromTriggerID(TriggerID);
            if (TriggerAction == 0) { DebugSerial->print(F("Off")); PrintSpaces(PadLength - 22); }
            if (TriggerAction == 1) { DebugSerial->print(F("On"));  PrintSpaces(PadLength - 21); }
        }
        else                         
        {
            DebugSerial->print(F("Variable"));
            PrintSpaces(PadLength - 27);
        }
    }
    // Aux RC Channel Trigger
    else if (TriggerID >= trigger_id_multiplier_auxchannel && TriggerID < trigger_id_adhoc_start)
    {
        int channelNum = TriggerID / trigger_id_multiplier_auxchannel;
        DebugSerial->print(F("Aux Channel "));
        if (channelNum < 10) PrintSpace();
        DebugSerial->print(channelNum, DEC);
        PrintSpaceDash();
        // Up to this point we now have something like "Aux Channel  4 - "
        if (sf_digital)
        {
            // But we still need to append the position. 
            TriggerAction = getTriggerActionFromTriggerID(TriggerID);
            int numPositions = getNumPositionsFromTriggerID(TriggerID);
            DebugSerial->print(F("Pos "));
            DebugSerial->print(TriggerAction);
            DebugSerial->print(F(" (of "));
            DebugSerial->print(numPositions);
            DebugSerial->print(F(")"));
            /*
            switch (numPositions)
            {
                case 2:
                    // The only two valid positions in this case are really just 1 & 5, but
                    // we split the difference at 3 for robustness
                    if (TriggerAction <= 3) DebugSerial->print(F("1 (of 2)"));
                    if (TriggerAction >  3) DebugSerial->print(F("2 (of 2)"));
                    PrintSpaces(PadLength - 29);    
                    break;
                case 3:
                    // Again here the only valid positions are 1,3 and 5 but this works as well
                    if (TriggerAction <  3) DebugSerial->print(F("1 (of 3)"));
                    if (TriggerAction == 3) DebugSerial->print(F("2 (of 3)"));
                    if (TriggerAction >  3) DebugSerial->print(F("3 (of 3)"));
                    PrintSpaces(PadLength - 29);
                    break;
                case 5:
                    DebugSerial->print(TriggerAction, DEC);
                    DebugSerial->print(F(" (of 5)"));
                    PrintSpaces(PadLength - 29);
                    break;
            } */
        }
        else
        {
            DebugSerial->print(F("Variable"));
            PrintSpaces(PadLength - 25);
        }
    }
    // Ad-Hoc Triggers
    else if (TriggerID >= trigger_id_adhoc_start && TriggerID < (trigger_id_adhoc_start + trigger_id_adhoc_range))
    {   // The only way to do these is hand-code them, that is why they are called "ad-hoc"
        switch (TriggerID)
        {   // The line should equal 29 characters
            case ADHOC_TRIGGER_BRAKES_APPLIED:      DebugSerial->print(F("Brakes Applied"));    PrintSpaces(15); break;
            case ADHOC_TRIGGER_CANNON_HIT:          DebugSerial->print(F("Cannon Hit"));        PrintSpaces(19); break;
            case ADHOC_TRIGGER_VEHICLE_DESTROYED:   DebugSerial->print(F("Vehicle Destroyed")); PrintSpaces(12); break;
            case ADHOC_TRIGGER_CANNON_RELOADED:     DebugSerial->print(F("Cannon Reloaded"));   PrintSpaces(14); break;
            case ADHOC_TRIGGER_ENGINE_START:        DebugSerial->print(F("Engine Start"));      PrintSpaces(17); break;
            case ADHOC_TRIGGER_ENGINE_STOP:         DebugSerial->print(F("Engine Stop"));       PrintSpaces(18); break;
            case ADHOC_TRIGGER_MOVE_FORWARD:        DebugSerial->print(F("Moving Forward"));    PrintSpaces(15); break;
            case ADHOC_TRIGGER_MOVE_REVERSE:        DebugSerial->print(F("Moving Reverse"));    PrintSpaces(15); break;
            case ADHOC_TRIGGER_VEHICLE_STOP:        DebugSerial->print(F("Vehicle Stopped"));   PrintSpaces(14); break;
            case ADHOC_TRIGGER_RIGHT_TURN:          DebugSerial->print(F("Right Turn Started"));PrintSpaces(11); break;
            case ADHOC_TRIGGER_LEFT_TURN:           DebugSerial->print(F("Left Turn Started")); PrintSpaces(12); break;
            case ADHOC_TRIGGER_NO_TURN:             DebugSerial->print(F("Turn Stopped"));      PrintSpaces(17); break;
        }        
    }
    // Vehicle speed triggers - increasing speed
    else if (TriggerID >= trigger_id_speed_increase && TriggerID < (trigger_id_speed_increase + trigger_id_speed_range))        
    {
        uint8_t triggerSpeed = TriggerID - trigger_id_speed_increase;  // The remainder is the percent we want to check against
        DebugSerial->print(F("Speed increases above "));
        if (triggerSpeed < 10) PrintSpace(); 
        PrintPct(triggerSpeed); 
        PrintSpaces(4);
    }
    // Vehicle speed triggers - decreasing speed
    else if (TriggerID >= trigger_id_speed_decrease && TriggerID < (trigger_id_speed_decrease + trigger_id_speed_range))        
    {
        uint8_t triggerSpeed = TriggerID - trigger_id_speed_decrease;  // The remainder is the percent we want to check against
        DebugSerial->print(F("Speed decreases below "));
        if (triggerSpeed < 10) PrintSpace(); 
        PrintPct(triggerSpeed); 
        PrintSpaces(4);
    }
    // Throttle command (variable)
    else if (TriggerID == trigger_id_throttle_command)
    {
        DebugSerial->print(F("Throttle Command"));
        PrintSpaces(PadLength - 16);
    }
    // Engine speed (variable)
    else if (TriggerID == trigger_id_engine_speed)
    {
        DebugSerial->print(F("Engine Speed"));
        PrintSpaces(PadLength - 12);
    }    
    // Vehicle speed (variable)
    else if (TriggerID == trigger_id_vehicle_speed)
    {
        DebugSerial->print(F("Vehicle Speed"));
        PrintSpaces(PadLength - 13);
    }
    // Steering command (variable)
    else if (TriggerID == trigger_id_steering_command)
    {
        DebugSerial->print(F("Steering Command"));
        PrintSpaces(PadLength - 16);
    }    
    // Turret rotation command (variable)
    else if (TriggerID == trigger_id_rotation_command)
    {
        DebugSerial->print(F("Turret Rotation Command"));
        PrintSpaces(PadLength - 23);
    }    
    // Barrel elevation command (variable)
    else if (TriggerID == trigger_id_elevation_command)
    {
        DebugSerial->print(F("Barrel Elevation Command"));
        PrintSpaces(PadLength - 24);
    }    
}

// The Trigger ID for the 9 turret stick positions are not numbers 1-9, 
// but for purposes of printing out the description we have the names in
// an array from 1-9 so sometimes it is useful to convert the Trigger ID
// to that range
uint8_t TurretStickPositionArrayNumFromTriggerID(uint8_t TID)
{
    switch (TID)
    {
        case 0:  return 0; break;
        case TL: return 1; break;
        case TC: return 2; break;
        case TR: return 3; break;
        case ML: return 4; break;
        case MC: return 5; break;
        case MR: return 6; break;
        case BL: return 7; break;
        case BC: return 8; break;
        case BR: return 9; break;
        default: return 0; break;
    }
}

// How many switch positions does this trigger have (External Input or Digital Aux only)
uint8_t getNumPositionsFromTriggerID(uint16_t TriggerID)
{
    // Turret stick trigger - num positions is not relevant
    if (TriggerID >0 && TriggerID <= MAX_SPEC_POS)
        return 0;

    // External input trigger - if digital, these are only 2-position switches:
    // On (line connected to ground) or Off (line disconnected/held high by internal pullups)
    if (TriggerID >= trigger_id_multiplier_ports && TriggerID < trigger_id_multiplier_auxchannel)
        return 2;

    // Aux Channel switches - if digital, these can be 2, 3 or 5 position switches (for now)
    if (TriggerID >= trigger_id_multiplier_auxchannel)
    {
        // We will walk through an example as we calculate these values.
        // Assume the Trigger ID is 4035
        // channelNum will equal 4 (Aux Channel 4)
        int channelNum = TriggerID / trigger_id_multiplier_auxchannel;
        // NumPos_CurPos will equal 35. 3 is the number of positions the switch has, 5 is the trigger position number
        // Recall that for a 3-position switch, the 3 possible positions are actually 1, 3, and 5
        int NumPos_CurPos = TriggerID - (channelNum * trigger_id_multiplier_auxchannel);
        // NumPositions will equal 3. This is the number of positions the switch is capable of
        int NumPositions = NumPos_CurPos / switch_pos_multiplier;
        return NumPositions;
        // Trigger Action will equal 5, we don't need it in this function though
        // int TriggerAction = NumPos_CurPos - (NumPositions * switch_pos_multiplier);
    }

    // Any other case, return 0
        return 0;
}

uint8_t getTriggerActionFromTriggerID(uint16_t TriggerID)
{   // Remember also, for analog triggers, there is no "action"

    int TriggerAction = 0;

    // Turret stick trigger - action *is* the trigger ID
    if (TriggerID >0 && TriggerID <= MAX_SPEC_POS)
        return TriggerID;   // should auto-cast to uint8_t. Anyway it doesn't matter, we won't be looking up TriggerActions for turret stick

    // External input trigger - if digital, these are only 2-position switches:
    // On (line connected to ground) or Off (line disconnected/held high by internal pullups)
    if (TriggerID >= trigger_id_multiplier_ports && TriggerID < trigger_id_multiplier_auxchannel)
    {
        int portNum = TriggerID / trigger_id_multiplier_ports;

        // The remainder is the action (1/0)
        TriggerAction = TriggerID - (portNum * trigger_id_multiplier_ports);
        return TriggerAction;
    }

    // Aux Channel switches - this is the tricky case that we have this function for in the first place
    if (TriggerID >= trigger_id_multiplier_auxchannel)
    {
        // We will walk through an example as we calculate these values.
        // Assume the Trigger ID is 4035
        // channelNum will equal 4 (Aux Channel 4)
        int channelNum = TriggerID / trigger_id_multiplier_auxchannel;
        // NumPos_CurPos will equal 35. "3" is the number of positions the switch has, "5" is the trigger action we want.
        // It is equal to "position 3" because for a three-position switch, the three positions are actually defined as action 1,3,5
        int NumPos_CurPos = TriggerID - (channelNum * trigger_id_multiplier_auxchannel);
        // NumPositions will equal 3. This is the number of positions the switch is capable of
        int NumPositions = NumPos_CurPos / switch_pos_multiplier;
        // Trigger Action will equal 5
        TriggerAction = NumPos_CurPos - (NumPositions * switch_pos_multiplier);
        return TriggerAction;
    }

    // Any other case, return 0
    return 0;
}

uint8_t getAuxChannelNumberFromTriggerID(uint16_t TriggerID)
{
    if (TriggerID >= trigger_id_multiplier_auxchannel)
    {
        return TriggerID / trigger_id_multiplier_auxchannel;
    }
    else
    {
        return 0;
    }
}


// ----------------------------------------------------------------------------------------------------------------------------------------------->>
// RE-DIRECT FUNCTIONS - FOR FUNCTIONS THAT DON'T NEED PARAMETERS
// ----------------------------------------------------------------------------------------------------------------------------------------------->>
// These functions don't need a variable passed, but we need a function with a uint16_t 
// for our function pointer. So to make using these functions elsewhere in the code
// easier, we create SF_functions that take an (ignored) parameter and then just call
// the function we actually want. 
void SF_EngineToggle(uint16_t ignoreMe)         { EngineToggle();           }
void SF_EngineOn(uint16_t ignoreMe)             { EngineOn();               }
void SF_EngineOff(uint16_t ignoreMe)            { EngineOff();              }
void SF_TransmissionToggle(uint16_t ignoreMe)   { TransmissionToggle();     }
void SF_TransmissionEngage(uint16_t ignoreMe)   { TransmissionEngage();     }
void SF_TransmissionDisengage(uint16_t ignoreMe){ TransmissionDisengage();  }
void SF_FireCannon(uint16_t ignoreMe)           { FireCannon();             }
void SF_MechBarrel(uint16_t ignoreMe)           { MechBarrel();             }
void SF_RecoilServo(uint16_t ignoreMe)          { TriggerServoRecoil();     }
void SF_HiFlash(uint16_t ignoreMe)              { MuzzleFlash();            }
void SF_MG_Start(uint16_t ignoreMe)             { MG_Start();               }
void SF_MG_Stop(uint16_t ignoreMe)              { MG_Stop();                }
void SF_MG2_Start(uint16_t ignoreMe)            { MG2_Start();              }
void SF_MG2_Stop(uint16_t ignoreMe)             { MG2_Stop();               }
void SF_MechBarrel_Enable(uint16_t ignoreMe)    { MechBarrel_Enable();      }
void SF_MechBarrel_Disable(uint16_t ignoreMe)   { MechBarrel_Disable();     }
void SF_MechBarrel_Toggle(uint16_t ignoreMe)    { MechBarrel_Toggle();      }
void SF_IR_Enable(uint16_t ignoreMe)            { IR_Enable();              }
void SF_IR_Disable(uint16_t ignoreMe)           { IR_Disable();             }
void SF_IR_Toggle(uint16_t ignoreMe)            { IR_Toggle();              }
void SF_Light1Toggle(uint16_t ignoreMe)         { Light1Toggle();           }
void SF_Light1On(uint16_t ignoreMe)             { Light1On();               }
void SF_Light1Off(uint16_t ignoreMe)            { Light1Off();              }
void SF_Light2Toggle(uint16_t ignoreMe)         { Light2Toggle();           }
void SF_Light2On(uint16_t ignoreMe)             { Light2On();               }
void SF_Light2Off(uint16_t ignoreMe)            { Light2Off();              }
void SF_RunningLightsToggle(uint16_t ignoreMe)  { RunningLightsToggle();    }
void SF_RunningLightsOn(uint16_t ignoreMe)      { RunningLightsOn();        }
void SF_RunningLightsOff(uint16_t ignoreMe)     { RunningLightsOff();       }
void SF_AuxOutputToggle(uint16_t ignoreMe)      { AuxOutputToggle();        }
void SF_AuxOutputOn(uint16_t ignoreMe)          { AuxOutputOn();            }
void SF_AuxOutputOff(uint16_t ignoreMe)         { AuxOutputOff();           }
void SF_AuxOutput_PresetDim(uint16_t ignoreMe)  { AuxOutput_PresetDim();    }
void SF_AuxOutput_ToggleDim(uint16_t ignoreMe)  { AuxOutputToggleDim();     }
void SF_AuxOutputFlash(uint16_t ignoreMe)       { AuxOutputFlash();         }
void SF_AuxOutputInverseFlash(uint16_t ignoreMe){ AuxOutputInverseFlash();  }
void SF_AuxOutputBlink(uint16_t ignoreMe)       { AuxOutputBlink();         }
void SF_AuxOutputToggleBlink(uint16_t ignoreMe) { AuxOutputToggleBlink();   }
void SF_AuxOutputRevolve(uint16_t ignoreMe)     { AuxOutputRevolve();       }
void SF_AuxOutputToggleRevolve(uint16_t ignoreMe){ AuxOutputToggleRevolve();}
void SF_OverlayEnable(uint16_t ignoreMe)        { EnableTrackOverlaySounds();}
void SF_OverlayDisable(uint16_t ignoreMe)       { DisableTrackOverlaySounds();}
void SF_TriggerUserSound1(uint16_t ignoreMe)    { TriggerUserSound(1);      }
void SF_UserSound1_Repeat(uint16_t ignoreMe)    { UserSound_Repeat(1);      }
void SF_UserSound1_Stop(uint16_t ignoreMe)      { UserSound_Stop(1);        }
void SF_TriggerUserSound2(uint16_t ignoreMe)    { TriggerUserSound(2);      }
void SF_UserSound2_Repeat(uint16_t ignoreMe)    { UserSound_Repeat(2);      }
void SF_UserSound2_Stop(uint16_t ignoreMe)      { UserSound_Stop(2);        }
void SF_TriggerUserSound3(uint16_t ignoreMe)    { TriggerUserSound(3);      }
void SF_UserSound3_Repeat(uint16_t ignoreMe)    { UserSound_Repeat(3);      }
void SF_UserSound3_Stop(uint16_t ignoreMe)      { UserSound_Stop(3);        }
void SF_TriggerUserSound4(uint16_t ignoreMe)    { TriggerUserSound(4);      }
void SF_UserSound4_Repeat(uint16_t ignoreMe)    { UserSound_Repeat(4);      }
void SF_UserSound4_Stop(uint16_t ignoreMe)      { UserSound_Stop(4);        }
void SF_TriggerUserSound5(uint16_t ignoreMe)    { TriggerUserSound(5);      }
void SF_UserSound5_Repeat(uint16_t ignoreMe)    { UserSound_Repeat(5);      }
void SF_UserSound5_Stop(uint16_t ignoreMe)      { UserSound_Stop(5);        }
void SF_TriggerUserSound6(uint16_t ignoreMe)    { TriggerUserSound(6);      }
void SF_UserSound6_Repeat(uint16_t ignoreMe)    { UserSound_Repeat(6);      }
void SF_UserSound6_Stop(uint16_t ignoreMe)      { UserSound_Stop(6);        }
void SF_TriggerUserSound7(uint16_t ignoreMe)    { TriggerUserSound(7);      }
void SF_UserSound7_Repeat(uint16_t ignoreMe)    { UserSound_Repeat(7);      }
void SF_UserSound7_Stop(uint16_t ignoreMe)      { UserSound_Stop(7);        }
void SF_TriggerUserSound8(uint16_t ignoreMe)    { TriggerUserSound(8);      }
void SF_UserSound8_Repeat(uint16_t ignoreMe)    { UserSound_Repeat(8);      }
void SF_UserSound8_Stop(uint16_t ignoreMe)      { UserSound_Stop(8);        }
void SF_TriggerUserSound9(uint16_t ignoreMe)    { TriggerUserSound(9);      }
void SF_UserSound9_Repeat(uint16_t ignoreMe)    { UserSound_Repeat(9);      }
void SF_UserSound9_Stop(uint16_t ignoreMe)      { UserSound_Stop(9);        }
void SF_TriggerUserSound10(uint16_t ignoreMe)   { TriggerUserSound(10);     }
void SF_UserSound10_Repeat(uint16_t ignoreMe)   { UserSound_Repeat(10);     }
void SF_UserSound10_Stop(uint16_t ignoreMe)     { UserSound_Stop(10);       }
void SF_TriggerUserSound11(uint16_t ignoreMe)   { TriggerUserSound(11);     }
void SF_UserSound11_Repeat(uint16_t ignoreMe)   { UserSound_Repeat(11);     }
void SF_UserSound11_Stop(uint16_t ignoreMe)     { UserSound_Stop(11);       }
void SF_TriggerUserSound12(uint16_t ignoreMe)   { TriggerUserSound(12);     }
void SF_UserSound12_Repeat(uint16_t ignoreMe)   { UserSound_Repeat(12);     }
void SF_UserSound12_Stop(uint16_t ignoreMe)     { UserSound_Stop(12);       }
void SF_UserSound_Stop_All(uint16_t ignoreMe)   { UserSound_StopAll();      }
void SF_Soundbank_A_PlayStop(uint16_t ignoreMe) { SoundBank(SOUNDBANK_A, ACTION_ONSTART); }
void SF_Soundbank_A_Next(uint16_t ignoreMe)     { SoundBank(SOUNDBANK_A, ACTION_PLAYNEXT); }
void SF_Soundbank_A_Previous(uint16_t ignoreMe) { SoundBank(SOUNDBANK_A, ACTION_PLAYPREV); }
void SF_Soundbank_A_Random(uint16_t ignoreMe)   { SoundBank(SOUNDBANK_A, ACTION_PLAYRANDOM); }
void SF_Soundbank_B_PlayStop(uint16_t ignoreMe) { SoundBank(SOUNDBANK_B, ACTION_ONSTART); }
void SF_Soundbank_B_Next(uint16_t ignoreMe)     { SoundBank(SOUNDBANK_B, ACTION_PLAYNEXT); }
void SF_Soundbank_B_Previous(uint16_t ignoreMe) { SoundBank(SOUNDBANK_B, ACTION_PLAYPREV); }
void SF_Soundbank_B_Random(uint16_t ignoreMe)   { SoundBank(SOUNDBANK_B, ACTION_PLAYRANDOM); }
void SF_PortA_Toggle(uint16_t ignoreMe)         { PortA_Toggle();           }
void SF_PortA_On(uint16_t ignoreMe)             { PortA_On();               }
void SF_PortA_Off(uint16_t ignoreMe)            { PortA_Off();              }
void SF_PortA_Pulse(uint16_t ignoreMe)          { PortA_Pulse();            }
void SF_PortA_Blink(uint16_t ignoreMe)          { PortA_Blink();            }
void SF_PortB_Toggle(uint16_t ignoreMe)         { PortB_Toggle();           }
void SF_PortB_On(uint16_t ignoreMe)             { PortB_On();               }
void SF_PortB_Off(uint16_t ignoreMe)            { PortB_Off();              }
void SF_PortB_Pulse(uint16_t ignoreMe)          { PortB_Pulse();            }
void SF_PortB_Blink(uint16_t ignoreMe)          { PortB_Blink();            }
void SF_UserFunc1(uint16_t ignoreMe)            { UserFunction1();          }
void SF_UserFunc2(uint16_t ignoreMe)            { UserFunction2();          }
void SF_DumpDebug(uint16_t ignoreMe)            { DumpSysInfo();            }
void SF_DriveProfile_1(uint16_t ignoreMe)       { SetDrivingProfile(1);     }
void SF_DriveProfile_2(uint16_t ignoreMe)       { SetDrivingProfile(2);     }
void SF_DriveProfile_Toggle(uint16_t ignoreMe)  { ToggleDrivingProfile();   }
void SF_ReduceSpeed_75(uint16_t ignoreMe)       { ReduceSpeed(75);          }
void SF_ReduceSpeed_50(uint16_t ignoreMe)       { ReduceSpeed(50);          }
void SF_ReduceSpeed_25(uint16_t ignoreMe)       { ReduceSpeed(25);          }
void SF_RestoreSpeed(uint16_t ignoreMe)         { ReduceSpeed(100);         }
void SF_Smoker_Enable(uint16_t ignoreMe)        { EnableSmoker();           }
void SF_Smoker_Disable(uint16_t ignoreMe)       { DisableSmoker();          }
void SF_Smoker_Toggle(uint16_t ignoreMe)        { ToggleSmoker();           }
void SF_Smoker_ManualOn(uint16_t ignoreMe)      { Smoker_ManualOn();        }
void SF_Smoker_ManualOff(uint16_t ignoreMe)     { Smoker_ManualOff();       }
void SF_Smoker_ManualToggle(uint16_t ignoreMe)  { Smoker_ManualToggle();    }
void SF_Smoker_PreheatEnable(uint16_t ignoreMe) { Smoker_PreheatEnable();   }
void SF_Smoker_PreheatDisable(uint16_t ignoreMe){ Smoker_PreheatDisable();  }
void SF_Smoker_PreheatToggle(uint16_t ignoreMe) { Smoker_PreheatToggle();   }
void SF_IncreaseVolume(uint16_t ignoreMe)       { IncreaseVolume();         }
void SF_DecreaseVolume(uint16_t ignoreMe)       { DecreaseVolume();         }
void SF_StopVolume(uint16_t ignoreMe)           { StopVolume();             }
void SF_ManualTransForward(uint16_t ignoreMe)   { ManualTransmission(GEAR_FORWARD);}
void SF_ManualTransReverse(uint16_t ignoreMe)   { ManualTransmission(GEAR_REVERSE);}
void SF_ManualTransNeutral(uint16_t ignoreMe)   { ManualTransmission(GEAR_NEUTRAL);}


// ----------------------------------------------------------------------------------------------------------------------------------------------->>
// INPUT PARAMETER SCALING
// ----------------------------------------------------------------------------------------------------------------------------------------------->>
// So-called "analog" special functions must all accept a uint16_t (two byte unsigned integer) that can range from 0-1023 (10-bit number),
// however the "analog" inputs may be a different range from that. 
// Analog inputs (on I/O port A or B) use analogRead which actually does return a value from 0-1023 on the Atmega2560, so we don't need to scale those. 
// But an RC channel will have a range from 1000 - 2000 approximately (more specifically, it will have a value from pulseMin to pulseMax)
// This function converts this RC channel scale to the expected scale for analog special functions
uint16_t ScaleAuxChannelPulse_to_AnalogInput(int chan)
{
    // Just to be sure we don't end up with anything out of bounds: 
    long val = constrain(Radio.AuxChannel[chan].pulse, Radio.AuxChannel[chan].Settings->pulseMin, Radio.AuxChannel[chan].Settings->pulseMax);
    // Map pulse within the pulse range to a new value within our analog special function range: 
    val = map(val, Radio.AuxChannel[chan].Settings->pulseMin, Radio.AuxChannel[chan].Settings->pulseMax, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL);
    return val;
}

// This scales our global MOTOR_MAX_REVSPEED and MOTOR_MAX_FWDSPEED to the analog range. These two are used widely throughout the project and are defined as -255 and 255.
// Radio commands and most motor speeds are set to those values. This function so far is only used to scale throttle command, engine speed, and vehicle speed as variable 
// inputs to analog functions, so we want the speed command to be absolute (0-255) where no command/speed = zero, rather than allowing negative speeds. 
uint16_t ScaleSpeed_to_AnalogInput_Abs(int16_t speed)
{
    return map(abs(speed), 0, MOTOR_MAX_FWDSPEED, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL);
}
// This is a similar case for other RC inputs used as variable triggers such as the steering channel and turret rotation/elevation channels. In this case we do account for 
// both negative and postivie values (to account for left/right or up/down, etc...)
uint16_t ScaleSpeed_to_AnalogInput_Signed(int16_t speed)
{
    return map(speed, MOTOR_MAX_REVSPEED, MOTOR_MAX_FWDSPEED, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL);
}

// ----------------------------------------------------------------------------------------------------------------------------------------------->>
// RE-DIRECT FUNCTIONS - FOR ANALOG FUNCTIONS WITH PARAMETER
// ----------------------------------------------------------------------------------------------------------------------------------------------->>

// Acceleration and Deceleration constraint adjustment
// ----------------------------------------------------------------------------------------------------------------------------------------------->>
// This function maps the 0-1023 analog input to a range from 1-MAX_SKIP_NUM expected by the two 
// drive functions used to change the acceleration/deceleration constraints
uint8_t SkipNum_from_AnalogInput(uint16_t val)
{
    long skipNum = constrain(val, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL);
    skipNum = map(skipNum, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, 1, MAX_SKIP_NUM);
    return skipNum; 
}
// These re-direct functions take care of mapping the analog value to the value expected by the accel/decel functions
void SF_SetAccelRampFreq(uint16_t val)
{
    if ((DrivingProfile == 1 && eeprom.ramcopy.AccelRampEnabled_1) || (DrivingProfile == 2 && eeprom.ramcopy.AccelRampEnabled_2))
    { 
        uint8_t sn = SkipNum_from_AnalogInput(val); 
        // Only update if changed
        if (sn != Driver.getAccelRampFrequency())
        {
            Driver.setAccelRampFrequency(sn); 
            if (DEBUG) { DebugSerial->print(F("Set Accel Ramp Level: ")); DebugSerial->println(sn); } 
        }
    }
}
void SF_SetDecelRampFreq(uint16_t val)
{ 
    if ((DrivingProfile == 1 && eeprom.ramcopy.DecelRampEnabled_1) || (DrivingProfile == 2 && eeprom.ramcopy.DecelRampEnabled_2))
    { 
        uint8_t sn = SkipNum_from_AnalogInput(val); 
        // Only update if changed
        if (sn != Driver.getDecelRampFrequency())
        {
            Driver.setDecelRampFrequency(sn); 
            if (DEBUG) { DebugSerial->print(F("Set Decel Ramp Level: ")); DebugSerial->println(sn); }
        }
    }
}


// RC Output pass-through functions
// ----------------------------------------------------------------------------------------------------------------------------------------------->>
void RC_Passthrough_1(uint16_t level)
{   // Recall, all analog special functions must receive values from 0-1023. But instead of using the map function here, we just set the external range
    // of the servo object to that range and the motor class will take care of it (which we did when we created this object)
    if (RCOutput1_Available) RCOutput1->setPos(level);    
}
void RC_Passthrough_2(uint16_t level)
{
    if (RCOutput2_Available) RCOutput2->setPos(level);
}
void RC_Passthrough_3(uint16_t level)
{
    if (RCOutput3_Available) RCOutput3->setPos(level);
}
void RC_Passthrough_4(uint16_t level)
{
    if (RCOutput4_Available) RCOutput4->setPos(level);
}
void RC_Passthrough_6(uint16_t level)
{
    if (RCOutput6_Available) RCOutput6->setPos(level);
}
void RC_Passthrough_7(uint16_t level)
{
    if (RCOutput7_Available) RCOutput7->setPos(level);
}
void RC_Passthrough_8(uint16_t level)
{
    if (RCOutput8_Available) RCOutput8->setPos(level);
}
// These are the pan servo variants. Here we set the pan speed instead of the actual position
void RC_PanServo_1(uint16_t level)
{
    if (RCOutput1_Available) ServoOutput1->setSpeed(level);
}
void RC_PanServo_2(uint16_t level)
{
    if (RCOutput2_Available) ServoOutput2->setSpeed(level);
}
void RC_PanServo_3(uint16_t level)
{
    if (RCOutput3_Available) ServoOutput3->setSpeed(level);
}
void RC_PanServo_4(uint16_t level)
{
    if (RCOutput4_Available) ServoOutput4->setSpeed(level);
}
void RC_PanServo_6(uint16_t level)
{
    if (RCOutput6_Available) ServoOutput6->setSpeed(level);
}
void RC_PanServo_7(uint16_t level)
{
    if (RCOutput7_Available) ServoOutput7->setSpeed(level);
}
void RC_PanServo_8(uint16_t level)
{
    if (RCOutput8_Available) ServoOutput8->setSpeed(level);
}


// Onbord motor pass-through functions - if not used for drive or turret control
// ----------------------------------------------------------------------------------------------------------------------------------------------->>
// If these exist, we already set the Motor A & B external range to ANALOG_SPECFUNCTION_MIN_VAL to ANALOG_SPECFUNCTION_MAX_VAL for the motor object in InstantiateMotorObjects() on the ObjectSetup tab.
// So no scaling needs to be done to level. 
void MotorA_ManualControl(uint16_t level)
{
    if (MotorA_Available) MotorA->setSpeed(level);
}
void MotorA_On(uint16_t IgnoreMe)
{
    if (MotorA_Available) MotorA->setSpeed(ANALOG_SPECFUNCTION_MAX_VAL);
}
void MotorA_Off(uint16_t IgnoreMe)
{
    if (MotorA_Available) MotorA->stop();
}
void MotorA_Toggle(uint16_t IgnoreMe)
{
    static boolean ison = false;

    if (MotorA_Available) 
    {
        ison ? MotorA_Off(0) : MotorA_On(0);
        ison = !ison;
    }
}

void MotorB_ManualControl(uint16_t level)
{
    if (MotorB_Available) MotorB->setSpeed(level);
}
void MotorB_On(uint16_t IgnoreMe)
{
    if (MotorB_Available) MotorB->setSpeed(ANALOG_SPECFUNCTION_MAX_VAL);
}
void MotorB_Off(uint16_t IgnoreMe)
{
    if (MotorB_Available) MotorB->stop();
}
void MotorB_Toggle(uint16_t IgnoreMe)
{
    static boolean ison = false;

    if (MotorB_Available) 
    {
        ison ? MotorB_Off(0) : MotorB_On(0);
        ison = !ison;
    }
}

// ----------------------------------------------------------------------------------------------------------------------------------------------->>
// SAVE ADJUSTMENTS
// ----------------------------------------------------------------------------------------------------------------------------------------------->>
// The user is able to modify several variables on the fly that also have default values in EEPROM. Although we don't want to update EEPROM
// every time an adjustment is made, because that would wear out the EEPROM, we do want to save them in certain cases. 
// A) The user presses the input button - this causes a save before we dump the system info. 
// That's the only case for now... but we might think of others. 
void SaveAdjustments()
{
    // Turn Mode - single byte
    eeprom.ramcopy.TurnMode = Driver.getTurnMode();
    EEPROM.updateByte(offsetof(_eeprom_data, TurnMode), eeprom.ramcopy.TurnMode);

    // Accel/Decel Level
    if (DrivingProfile == 1)
    {
        eeprom.ramcopy.AccelSkipNum_1 = Driver.getAccelRampFrequency();
        EEPROM.updateByte(offsetof(_eeprom_data, AccelSkipNum_1), eeprom.ramcopy.AccelSkipNum_1);
        eeprom.ramcopy.DecelSkipNum_1 = Driver.getDecelRampFrequency();
        EEPROM.updateByte(offsetof(_eeprom_data, DecelSkipNum_1), eeprom.ramcopy.DecelSkipNum_1);
    }
    else
    {
        eeprom.ramcopy.AccelSkipNum_2 = Driver.getAccelRampFrequency();
        EEPROM.updateByte(offsetof(_eeprom_data, AccelSkipNum_2), eeprom.ramcopy.AccelSkipNum_2);
        eeprom.ramcopy.DecelSkipNum_2 = Driver.getDecelRampFrequency();
        EEPROM.updateByte(offsetof(_eeprom_data, DecelSkipNum_2), eeprom.ramcopy.DecelSkipNum_2);
    }
    
    // Aux Output level
    EEPROM.updateByte(offsetof(_eeprom_data, AuxLightPresetDim), eeprom.ramcopy.AuxLightPresetDim);

    // ANY OTHER SPECIAL FUNCTIONS THAT ADJUST PROGRAM SETTINGS STORED IN EEPROM, BE SURE TO ADD HERE
    // ...
}







