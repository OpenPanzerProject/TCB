/* OP_EEPROM.cpp    Open Panzer EEPROM - a library functions related to EEPROM usage
 * Source:          openpanzer.org              
 * Authors:         Luke Middleton
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */ 


#include "OP_EEPROM.h"


// Static variables must be declared outside the class
    _eeprom_data OP_EEPROM::ramcopy;


//------------------------------------------------------------------------------------------------------------------------>>
// CONSTRUCTOR, BEGIN, and FIRST STUFF
//------------------------------------------------------------------------------------------------------------------------>>

// Constructor
OP_EEPROM::OP_EEPROM(void) { }

// Begin
boolean OP_EEPROM::begin(void)
{
    // Check if EEPROM has ever been initalized, if not, do so
    long Temp = EEPROM.readLong(offsetof(_eeprom_data, InitStamp));// Get our EEPROM initialization stamp code
    if(Temp != EEPROM_INIT)                                         // EEPROM_INIT is set at the top of OP_EEPROM.h. It must be changed to a new number  
    {   Initialize_EEPROM();                                        // if changes have been made to the OP_EEPROM_Struct.h file. 
        return true;        // If we initialized EEPROM, return true
    }
    else
    {
        // In this case, the values in EEPROM are what we want. We load them all to RAM
        loadRAMcopy();
        return false;
    }

}

// This takes all variables from the eeprom struct, and puts them into the RAM copy struct
void OP_EEPROM::loadRAMcopy(void)
{
    EEPROM.readBlock(EEPROM_START_ADDRESS, ramcopy);
}
    



//------------------------------------------------------------------------------------------------------------------------>>
// READING FROM EEPROM / RAM Copy
//------------------------------------------------------------------------------------------------------------------------>>    
// This function isn't really used for general reading from EEPROM, for that we would just use EEPROM.readByte/Int/Long, etc...
// Instead this is used when communicating with the PC for retrieving a value from EEPROM by unique ID, and sending it
// out the comm port as serial data. First we retrieve the value, then we convert it to a string of ASCII characters that 
// are stored in a character array passed by reference. 
boolean OP_EEPROM::readSerialEEPROM_byID(uint16_t ID, char *chrArray, uint8_t bufflen, uint8_t &stringlength)
{
    _storage_var_info svi;
    uint16_t arrayPos;  
    String str;  // Yes, we are using the dreaded string object

    // Get the data info for this variable, it is stored in svi if successful
    arrayPos = findStorageVarInfo(svi, ID);
    
    // TESTING:
/*    if (ID == 1015)
    {
    Serial1.print(F("ID: "));
    Serial1.println(ID);
    Serial1.print(F("Array pos: "));
    Serial1.println(arrayPos);
    Serial1.print(F("Var Type: "));
    Serial1.println(printVarType(svi.varType));
    Serial1.print(F("Offset: "));
    Serial1.println(svi.varOffset);
    }
*/
    
    if (arrayPos == 0)
    {
        return false;
    }
    else
    {
        switch (svi.varType)
        {
            case varNULL:
                return false;
                break;
                
            case varBOOL:
            case varCHAR:
            case varUINT8:  // Unsigned 8 bit
                {
                    uint8_t myUint8 = EEPROM.readByte(svi.varOffset);
                    str = String(myUint8);
                    str.toCharArray(chrArray, bufflen);
                    stringlength = str.length();    // Return the actual string length
                }
                break;
            
            case varINT8:   // Signed 8 bit
                {
                    int8_t myInt8 = EEPROM.readByte(svi.varOffset);
                    str = String(myInt8);
                    str.toCharArray(chrArray, bufflen); 
                    stringlength = str.length();    // Return the actual string length
                }
                break;
                
            
            case varINT16:  // Signed 16 bit
                {
                    int16_t myInt16 = (int16_t)EEPROM.readInt(svi.varOffset);
                    str = String(myInt16);
                    str.toCharArray(chrArray, bufflen);
                    stringlength = str.length();    // Return the actual string length
                }
                break;          
            
            case varUINT16: // Unsigned 16 bit
                {
                    uint16_t myUint16 = (uint16_t)EEPROM.readInt(svi.varOffset);
                    str = String(myUint16, DEC);
                    str.toCharArray(chrArray, bufflen);
                    stringlength = str.length();    // Return the actual string length
                }
                break;

            case varINT32:  // Signed 32 bit
                {
                    int32_t myInt32 = (int32_t)EEPROM.readLong(svi.varOffset);
                    str = String(myInt32, DEC);
                    str.toCharArray(chrArray, bufflen);
                    stringlength = str.length();    // Return the actual string length
                }
                break;

            case varUINT32: // Unsigned 32 bit
                {
                    uint32_t myUint32 = (uint32_t)EEPROM.readLong(svi.varOffset);
                    str = String(myUint32, DEC);
                    str.toCharArray(chrArray, bufflen);
                    stringlength = str.length();    // Return the actual string length
                }
                break;

            default:
                return false;
        }
    }

    return 1;

}

//------------------------------------------------------------------------------------------------------------------------>>
// WRITING TO EEPROM / RAM Copy
//------------------------------------------------------------------------------------------------------------------------>>    
// Writes value "Value" to the eeprom variable with ID "ID"
// Again, this is only really used for communicating with the PC. Otherwise we could just write directly with the variable name, ie
// EEPROM.writeInt(offsetof(_eeprom_data, SmokerMaxSpeed), 255);
boolean OP_EEPROM::updateEEPROM_byID(uint16_t ID, uint32_t Value)
{
    _storage_var_info svi;
    uint16_t arrayPos;

    // Get the data info for this variable, it is stored in svi if successful
    arrayPos = findStorageVarInfo(svi, ID);
    
    if (arrayPos == 0)
    {
        return false;
    }
    else
    {
        switch (svi.varType)
        {
            case varNULL:
                return false;
                break;
                
            case varBOOL:
            case varCHAR:
            case varINT8:
            case varUINT8:
                // This will return false if the value we are updating is the same as the value we're sending.
                // That doesn't mean an error occured, it just means there was nothing to update. So we just
                // return true regardless
                EEPROM.updateByte(svi.varOffset, (uint8_t)Value);
                return true;
                break;
            
            case varINT16:
            case varUINT16:
                EEPROM.updateInt(svi.varOffset, (uint16_t)Value);
                return true;
                break;

            case varINT32:
            case varUINT32:
                EEPROM.updateLong(svi.varOffset, (uint32_t)Value);
                return true;
                break;

            default:
                return false;
        }
            
        // If we want to update the same variable in our RAM copy struct, the following (untested)
        // might work. 
        /*
        int *base;
        int *element;
        _eeprom_data *rc; 
        rc = &ramcopy;
        
        base = (int *)rc;                       // starting address of ramcopy struct
        element = (int *)(base+svi.varOffset);  // Pointer to starting byte of the member we want to update
        for (int i=svi.byteCount; i-->0;)        
        {
            *element = (Value >> (8*(i-1))) & 0xff;     // Write byte to the current pointer location (* dereference)
            element += 8;                               // Increment our pointer to the next byte (no * dereference)
        }
        */
        // However, instead of bothering with that, we will just update the eeprom. Later we can copy the entire eeprom
        // struct to our RAM copy using loadRAMcopy()
        
    }
}



//------------------------------------------------------------------------------------------------------------------------>>
// EEPROM / RAM COPY UTILITIES
//------------------------------------------------------------------------------------------------------------------------>>    
uint16_t OP_EEPROM::findStorageVarInfo(_storage_var_info &svi, uint16_t findID)
{
    static uint16_t lastArrayPos = 0;
    int i;
    boolean found = false;
    
    // Start searching from the position after the last one we found
    for (i=(lastArrayPos+1); i<NUM_STORED_VARS; i++)
    {   
        // Back when we had STORAGEVARS (see OP_EEPROM_VarInfo.h) in regular progmem, we could do this: 
        // if (findID == pgm_read_word_near(&(STORAGEVARS[i].varID)))
        // Note we could reference any element of the struct array using typical array syntax ([i]) and we could also access the 
        // the struct members directly by name (in this case varID). 
        
        // When we moved it to PROGMEM_FAR (out beyond the first 64k of program memory) we could no longer
        // address it with an 8-bit pointer. Instead we use the "pgm_get_far_address" macro
        // to return a 32-bit pointer to the start address of the struct. This precludes us from obtaining individual 
        // elements of the array in the traditional manner, or the struct members likewise. Here we get the starting address, 
        // then to get the first word of the i-th struct we multiply i by 5 which is the number of bytes in each struct, or 
        // in other words, the number of bytes for each element of the array. See below for other machinations to get 
        // struct members other than the first one (varID is the first member of the _storage_var_info struct)
        if (findID == pgm_read_word_far(pgm_get_far_address(STORAGEVARS) + (i*5)))
        {
            found = true;
            break;
        }
    }
    
    // If that didn't find it, wrap around back to the beginning and 
    // search the first half too. Position 0 doesn't count. 
    if (!found)
    {
        for (i=1; i<=lastArrayPos; i++)
        {   // Old method: 
            // if (findID == pgm_read_word_near(&(STORAGEVARS[i].varID)))
            
            // Far method: 
            if (findID == pgm_read_word_far(pgm_get_far_address(STORAGEVARS) + (i*5)))
            {
                found = true;
                break;
            }
        }
    }

    // If found, fill in the _storage_var struct that was passed, and return
    // the array position
    if (found)
    {   // Old method: 
        //svi.varOffset = pgm_read_word_near(&(STORAGEVARS[i].varOffset));        // read_word is for reading a two-byte value (int16 for example)
        //svi.varType = pgm_read_byte_near(&(STORAGEVARS[i].varType));            // read_byte is for reading a single byte value (int8 for example)
        
        // Far method: 
        // Here we use again i*5 to get us the i-th element of the array. But we also add some more bytes to reach the second and third members of the struct
        svi.varOffset = pgm_read_word_far(pgm_get_far_address(STORAGEVARS) + (i*5) + 2);    // varOffset comes after varID which is 2 bytes, so we need to add 2 bytes to reach it. varOffset is 2 bytes as well so we use read_word.
        svi.varType = pgm_read_byte_far(pgm_get_far_address(STORAGEVARS) + (i*5) + 4);      // varType comes after varID and varOffset so we have to add 4 bytes to reach it. varType is only 1 byte so we use read_byte to read it. 
        lastArrayPos = i;   // remember where we were for next time
        return i+1;         // return the position. Add 1 since the array is zero-based, but zero doesn't count (our null "FirstVal")
    }
    else
    {
        // If not found, return 0
        lastArrayPos = 0;
        return 0;
    }

}

boolean OP_EEPROM::getStorageVarInfo(_storage_var_info & svi, uint16_t arrayPos)
{
    if (arrayPos > 0 && arrayPos <= NUM_STORED_VARS)
    {   // Old method: 
        //svi.varOffset = pgm_read_word_near(&(STORAGEVARS[arrayPos].varOffset));         
        //svi.varType = pgm_read_byte_near(&(STORAGEVARS[arrayPos].varType));       
        
        // Far method: see the discussion above for what we're doing. 
        svi.varOffset = pgm_read_word_far(pgm_get_far_address(STORAGEVARS) + (arrayPos*5) + 2);
        svi.varType = pgm_read_byte_far(pgm_get_far_address(STORAGEVARS) + (arrayPos*5) + 4);
        return true;
    }
    else
    {
        return false;   
    }
}




//------------------------------------------------------------------------------------------------------------------------>>
// INITIALIZE EEPROM
//------------------------------------------------------------------------------------------------------------------------>>
void OP_EEPROM::factoryReset(void)
{
    Initialize_EEPROM();
}

void OP_EEPROM::Initialize_EEPROM(void) 
{   
    // The way we do this is set the values in our ramcopy struct, then write the entire struct to EEPROM (actually "update" instead of "write")
    Initialize_RAMcopy();                               // Set RAM variables to sensible defaults
    ramcopy.InitStamp = EEPROM_INIT;                    // Set the InitStamp
    EEPROM.updateBlock(EEPROM_START_ADDRESS, ramcopy);  // Now write it all to EEPROM. We use the "update" function so as not to 
                                                        // unnecessarily writebytes that haven't changed. 
}

// THIS IS WHERE EEPROM DEFAULT VALUES ARE SET
void OP_EEPROM::Initialize_RAMcopy(void) 
{   // If EEPROM has not been used before, we initialize to some sensible, yet conservative, default values.
    
    // FirstVar
        ramcopy.FirstVar = 0;                           // Don't mess with this one
    
    // Write 4 channel settings. 
        stick_channel_settings DefaultChSettings;
        DefaultChSettings.pulseMin = 1000;
        DefaultChSettings.pulseMax = 2000;
        DefaultChSettings.pulseCenter = 1500;
        DefaultChSettings.deadband = DEFAULT_DEADBAND;  // Setting in OP_RadioDefines.h
        DefaultChSettings.reversed = false;             // Default to not reversed
        
        //We assume Radio has channel order RETA (2/4/3/1), but the user can change this in the menu
        DefaultChSettings.channelNum = 2;
        ramcopy.ThrottleSettings = DefaultChSettings;
        
        DefaultChSettings.channelNum = 4;
        ramcopy.TurnSettings = DefaultChSettings;
        
        DefaultChSettings.channelNum = 3;
        ramcopy.ElevationSettings = DefaultChSettings;
        
        DefaultChSettings.channelNum = 1;
        ramcopy.AzimuthSettings = DefaultChSettings;

    // Write aux channel settings. Default to digital inputs, 2-position switch, not reversed. 
    // channel order in the PPM stream of 5,6,7,8,9,10,etc..
        uint8_t firstChanNum = 5;
        for (int a=0; a<AUXCHANNELS; a++)
        {
            ramcopy.Aux_ChannelSettings[a].channelNum = firstChanNum + a;
            ramcopy.Aux_ChannelSettings[a].pulseMin = 1000;
            ramcopy.Aux_ChannelSettings[a].pulseMax = 2000;
            ramcopy.Aux_ChannelSettings[a].pulseCenter = 1500;
            ramcopy.Aux_ChannelSettings[a].Digital = true;
            ramcopy.Aux_ChannelSettings[a].numPositions = 2;
            ramcopy.Aux_ChannelSettings[a].reversed = false;
        }
    
    // External I/O ports
        ramcopy.PortA_Settings.dataDirection = 0;   // input
        ramcopy.PortA_Settings.dataType = 1;        // Assume digital input (on/off only)
        ramcopy.PortB_Settings.dataDirection = 0;   // input
        ramcopy.PortB_Settings.dataType = 1;        // Assume digital input (on/off only)
        ramcopy.IOBlinkOnTime_mS = 650;             // 650 mS
        ramcopy.IOBlinkOffTime_mS = 650;            // 650 mS

    // Special function triggers - We init a few common ones to the turret stick (nothing on aux channels)
    // The SF_ numbers are defined in OP_Radio.h
        // First, let's clear all 
        for (int i=0; i<MAX_FUNCTION_TRIGGERS; i++)
        {
            ramcopy.SF_Trigger[i].TriggerID = 0;
            ramcopy.SF_Trigger[i].specialFunction = SF_NULL_FUNCTION;
        }   
        // Now, we just set the few we want to default to
        ramcopy.SF_Trigger[0].TriggerID = TL;
        ramcopy.SF_Trigger[0].specialFunction = SF_ENGINE_TOGGLE;
        ramcopy.SF_Trigger[1].TriggerID = BL;
        ramcopy.SF_Trigger[1].specialFunction = SF_TRANS_TOGGLE;
        ramcopy.SF_Trigger[2].TriggerID = TR;
        ramcopy.SF_Trigger[2].specialFunction = SF_CANNON_FIRE;
        ramcopy.SF_Trigger[3].TriggerID = BR;
        ramcopy.SF_Trigger[3].specialFunction = SF_LIGHT1_TOGGLE;
        
    // 4 main motor drive types. We default motors to the OP Scout ESC and turret to onboard
        ramcopy.DriveMotors = OP_SCOUT;
        ramcopy.TurretRotationMotor = ONBOARD;
        ramcopy.TurretElevationMotor = ONBOARD;

    // Elevation motor min/max/reversed
        ramcopy.TurretElevation_EPMin = 1000;
        ramcopy.TurretElevation_EPMax = 2000;
        ramcopy.TurretElevation_Reversed = false;
        ramcopy.TurretElevation_MaxSpeedPct = 100;
        ramcopy.TurretRotation_MaxSpeedPct = 100;
        ramcopy.TurretRotation_EPMin = 1000;
        ramcopy.TurretRotation_EPMax = 2000;
        ramcopy.TurretRotation_Reversed = false;

    // Steering servo adjustments
        ramcopy.SteeringServo_EPMin = 1000;
        ramcopy.SteeringServo_EPMax = 2000;
        ramcopy.SteeringServo_Reversed = false;

    // Mechanical Barrel and Recoil Servo settings
        ramcopy.Airsoft = false;                // Default to mechanical recoil, because Airsoft selection will prevent some things from working if the user doesn't actually connect an airsoft unit
        ramcopy.MechanicalBarrelWithCannon = true;
        ramcopy.RecoilDelay = 0;                // Default to no delay between recoil action, and flash/sound
        ramcopy.RecoilReversed = true;          // We default to reversed because this works with the Taigen Tiger 1 combination airsoft/servo recoil unit
        ramcopy.ServoRecoilWithCannon = true;
        ramcopy.RecoilServo_Recoil_mS = 200;    // These default times work well with the Taigen Tiger 1 unit
        ramcopy.RecoilServo_Return_mS = 800;
        ramcopy.RecoilServo_EPMin = 1000;
        ramcopy.RecoilServo_EPMax = 2000;
        ramcopy.RecoilServo_PresetNum = RS_PRESET_NONE;  // Not presently implemented
        
    // On board smoker output
        ramcopy.SmokerControlAuto = true;
        ramcopy.SmokerIdleSpeed = 89;           // 35 percent
        ramcopy.SmokerFastIdleSpeed = 128;      // 50 percent
        ramcopy.SmokerMaxSpeed = 255;           // 100 percent
        ramcopy.SmokerDestroyedSpeed = 255;     // 100 percent
        ramcopy.SmokerDeviceType = SMOKERTYPE_ONBOARD_STANDARD; // Standard onboard smoker with single output for combined fan & heat (ie, Heng Long/Taigen types)
        ramcopy.SmokerPreHeat_Sec = 0;          // How long to preheat the heating element before engine start, 0 to disable
        ramcopy.SmokerHeatIdleAmt = 255;
        ramcopy.SmokerHeatFastIdleAmt = 255;
        ramcopy.SmokerHeatMaxAmt = 255;
        ramcopy.HotStartTimeout_Sec = 0;        // If the engine is started again within this amount of time since the last shutdown, it will be considered a hot-start and the pre-heat will be skipped. 

    // Driving adjustments
        ramcopy.AccelRampEnabled_1 = true;      // Profile 1 settings - mild
        ramcopy.AccelSkipNum_1 = 4;
        ramcopy.AccelPreset_1 = 0;
        ramcopy.DecelRampEnabled_1 = true;
        ramcopy.DecelSkipNum_1 = 4;
        ramcopy.DecelPreset_1 = 0;
        ramcopy.AccelRampEnabled_2 = false;     // Profile 2 settings - nothing
        ramcopy.AccelSkipNum_2 = 1;
        ramcopy.AccelPreset_2 = 0;
        ramcopy.DecelRampEnabled_2 = false;
        ramcopy.DecelSkipNum_2 = 1;
        ramcopy.DecelPreset_2 = 0;          
        ramcopy.BrakeSensitivityPct = 40;       // NOT PRESENTLY IMPLEMENTED
        ramcopy.TimeToShift_mS = 1000;          // Default to 1 second
        ramcopy.EnginePauseTime_mS = 1000;      // Default to 1 second
        ramcopy.TransmissionDelay_mS =2000;     // Default to 2 seconds
        ramcopy.NeutralTurnAllowed = true;
        ramcopy.NeutralTurnPct = 50;            // Default to 50 percent of max forward speed for neutral turn max speed
        ramcopy.TurnMode = 2;                   // Default Turn Mode = 2. See MixSteering function in OP_Driver.cpp for specific mode definitions. 
        ramcopy.DriveType = DT_TANK;            // Default to Tank
        ramcopy.MaxForwardSpeedPct = 100;       // Default to 100 percent
        ramcopy.MaxReverseSpeedPct = 70;        // Default to 70 percent
        ramcopy.HalftrackTreadTurnPct = 50;     // Default to 50 pct turn command applied to halftrack treads (in halftrack mode)
        ramcopy.EngineAutoStart = false;        // Default to engine start with trigger instead of throttle. 
        ramcopy.EngineAutoStopTime_mS = 0;      // Default to auto-stop disabled (time = 0)
        ramcopy.MotorNudgePct = 0;              // Default to disabled (when 0, nudge effect disabled)
        ramcopy.NudgeTime_mS = 250;             // Default to 1/4 second
        ramcopy.DragInnerTrack = false;         // Default to false (off)
        ramcopy.EnableTrackRecoil = false;      // Default to false (off)
        ramcopy.TrackRecoilKickbackSpeed = 100; // Initial motor kickback speed as percent 0-100
        ramcopy.TrackRecoilDecelerateFactor = 100; // Deceleration factor applied to the original kick-back speed, or, in simple mode this becomes the recoil duration in mS
		

    // "Physics"
        ramcopy.EnableBarrelStabilize = false;  // If an accelerometer is present, and turret elevation motor is type SERVO_PAN, this will stabilize the barrel
        ramcopy.BarrelSensitivity = 50;         // Sensitivity number from 1 - 100
        ramcopy.EnableHillPhysics = false;      // If an accelerometer is present, this will cause the tank to slow down on uphill climbs, and speed up on downhill. 
        ramcopy.HillSensitivity = 50;           // Sensitivity number from 1 - 100

    // Turret stick adjustments
        ramcopy.IgnoreTurretDelay_mS = 350;     // Turret stick delay defaults to 350ms
        
    // Sound settings
        ramcopy.SoundDevice = SD_BENEDINI_TBSMINI;  
        ramcopy.Squeak1_MinInterval_mS = 1500;
        ramcopy.Squeak1_MaxInterval_mS = 4000;
        ramcopy.Squeak2_MinInterval_mS = 2000;
        ramcopy.Squeak2_MaxInterval_mS = 5000;
        ramcopy.Squeak3_MinInterval_mS = 3000;
        ramcopy.Squeak3_MaxInterval_mS = 8000;
        ramcopy.Squeak1_Enabled = false;
        ramcopy.Squeak2_Enabled = false;
        ramcopy.Squeak3_Enabled = false;
        ramcopy.MinSqueakSpeedPct = 25;                 // Percent of full movement before squeaks are activated (if squeaks are even enabled)
        ramcopy.HeadlightSound_Enabled = true;
        ramcopy.TurretSound_Enabled = true;
        ramcopy.BarrelSound_Enabled = true;   
        ramcopy.Squeak4_MinInterval_mS = 1000;
        ramcopy.Squeak4_MaxInterval_mS = 2000;
        ramcopy.Squeak5_MinInterval_mS = 3000;
        ramcopy.Squeak5_MaxInterval_mS = 4000;
        ramcopy.Squeak6_MinInterval_mS = 5000;
        ramcopy.Squeak6_MaxInterval_mS = 6000;
        ramcopy.Squeak4_Enabled = false;       
        ramcopy.Squeak5_Enabled = false;       
        ramcopy.Squeak6_Enabled = false;       
        ramcopy.VolumeEngine = 50;
        ramcopy.VolumeTrackOverlay = 50;
        ramcopy.VolumeEffects = 50;
        ramcopy.HeadlightSound2_Enabled = false;
        ramcopy.SoundBankA_Loop = false;
        ramcopy.SoundBankB_Loop = false;
        
    // Battle settings
        ramcopy.IR_FireProtocol = IR_TAMIYA;            // Default to Tamiya IR battle protocol
        ramcopy.IR_HitProtocol_2 = IR_UNKNOWN;          // Default to no second protocol
        ramcopy.IR_RepairProtocol = IR_RPR_CLARK;       // Default to Clark repair protocol
        ramcopy.IR_MGProtocol = IR_UNKNOWN;             // Default to no machine gun protocol
        ramcopy.Use_MG_Protocol = false;                // Default to false
        ramcopy.Accept_MG_Damage = false;               // Default to false
        ramcopy.DamageProfile = TAMIYA_DAMAGE;          // Default to stock Tamiya damage profile
        ramcopy.CustomClassSettings.reloadTime = 1500;  // User custom weight class: Default to Ultralight (1.5 seconds reload time)
        ramcopy.CustomClassSettings.recoveryTime  = 8000; // User custom weight class: Default to Ultralight (8 seconds invulnerability after recovery)
        ramcopy.CustomClassSettings.maxHits = 1;        // User custom weight class: Default to Ultralight (1 hit and vehicle destroyed)
        ramcopy.CustomClassSettings.maxMGHits = 10;     // User custom weight class: Default to 10 hits with MG and vehicle destroyed (only if Accept_MG_Damage = true)
        ramcopy.SendTankID = false;                     // Default to not sending the tank ID
        ramcopy.TankID = 1;                             // Tank ID number, default to 1
        ramcopy.IR_Team = IR_TEAM_NONE;                 // Default to NO team (default protocol of Tamiya doesn't have teams)

    // Board settings
        ramcopy.USBSerialBaud = USB_BAUD_RATE;
        ramcopy.AuxSerialBaud = 115200;                 // Default Aux to 115200
        ramcopy.MotorSerialBaud = DEFAULTBAUDRATE;
        ramcopy.Serial3TxBaud = DEFAULTBAUDRATE;
        ramcopy.LVC_Enabled = false;
        ramcopy.LVC_Cutoff_mV = 6400;

    // Light settings
        ramcopy.RunningLightsAlwaysOn = false;
        ramcopy.RunningLightsDimLevelPct = 30;          // percent
        ramcopy.BrakesAutoOnAtStop = false;
        ramcopy.AuxLightFlashTime_mS = 30;              // 
        ramcopy.AuxLightBlinkOnTime_mS = 30;            // milliseconds
        ramcopy.AuxLightBlinkOffTime_mS = 30;           // milliseconds
        ramcopy.AuxLightPresetDim = 30;                 // percent
        ramcopy.MGLightBlink_mS = 30;                   // milliseconds
        ramcopy.FlashLightsWhenSignalLost = true;   
        ramcopy.HiFlashWithCannon = true;   
        ramcopy.AuxFlashWithCannon = false;             
        ramcopy.SecondMGLightBlink_mS = 30;             // milliseconds        
        ramcopy.CannonReloadBlink = true;

    // Scout settings
        ramcopy.ScoutCurrentLimit = 12;                 // Default to 12 amps

    // Program settings
        ramcopy.PrintDebug = true;                      // Default to debugging on
}



//------------------------------------------------------------------------------------------------------------------------>>
// MISC
//------------------------------------------------------------------------------------------------------------------------>>
// Little function to help us print out helpful var type names, rather than numbers. 
// To use, call something like this:  Serial.print(printVarType(my_vartypes));
const __FlashStringHelper *printVarType(_vartype Type) 
{    
    if(Type>LAST_VARTYPE) Type=varNULL;
    const __FlashStringHelper *Names[LAST_VARTYPE+1]={F("NULL"),F("Boolean"),F("Char"),F("INT 8"),F("UINT 8"),F("INT 16"),F("UINT 16"),F("INT 32"),F("UINT 32")};
    return Names[Type];
};
