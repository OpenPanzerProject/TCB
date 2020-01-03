/* OP_Radio.cpp     Open Panzer Radio - radio class for converting inputs (from various radio types) into more useful channel objects. 
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
 
#include "OP_Radio.h"

boolean                     OP_Radio::didWeBegin;                   
int                         OP_Radio::ChannelsUtilized;
boolean                     OP_Radio::UsingSpecialPositions;
boolean                     OP_Radio::InFailsafe;                   // Are we in failsafe due to some radio problem?            
stick_channels              OP_Radio::Sticks;                       // Creates a collection of linear channels named Throttle, Turn, Elevation, Azimuth
aux_channels                OP_Radio::AuxChannel[AUXCHANNELS];      // Create AUXCHANNELS number of type aux_channels
sf_channel                  OP_Radio::SpecialStick;                 // This holds information about the turret stick, and whether it is being held in a position to indicate a special command
PPMDecode                 * OP_Radio::PPMDecoder;                   // PPM Decoder object       
SBusDecode                * OP_Radio::SBusDecoder;                  // SBus Decoder object
iBusDecode                * OP_Radio::iBusDecoder;                  // iBus Decoder object
boolean                     OP_Radio::PPMFailed;
boolean                     OP_Radio::SBusFailed;
boolean                     OP_Radio::iBusFailed;       
RADIO_PROTOCOL              OP_Radio::Protocol;                     // Which protocol detected
OP_SimpleTimer            * OP_Radio::radioTimer;
uint8_t                     OP_Radio::channelCount;
common_channel_settings     OP_Radio::ptrCommonChannelSettings[(STICKCHANNELS + AUXCHANNELS)];    // This array of pointers to common channel settings for all channels allows us to loop through them quickly, see GetPPMFrame() in RadioInputs tab. 
int16_t                     OP_Radio::ignoreTurretDelay_mS;
int                         OP_Radio::WatchdogTimerID;



// Returns a pointer to a flash-stored character string that is the name of the turret stick position
const __FlashStringHelper *TurretStickPosition(uint8_t TSP) 
{
    if ( TSP > MAX_SPEC_POS) TSP = 0;
    const __FlashStringHelper *Names[SPECIALPOSITIONS+1]={F("Unknown"),F("Top Left"),F("Top Center"),F("Top Right"), 
                                                          F("Middle Left"),F("Middle Center"),F("Middle Right"),
                                                          F("Bottom Left"),F("Bottom Center"),F("Bottom Right")};
    switch (TSP)
    {
        case 0:  return Names[0]; break;
        case TL: return Names[1]; break;
        case TC: return Names[2]; break;
        case TR: return Names[3]; break;
        case ML: return Names[4]; break;
        case MC: return Names[5]; break;
        case MR: return Names[6]; break;
        case BL: return Names[7]; break;
        case BC: return Names[8]; break;
        case BR: return Names[9]; break;
        default: return Names[0]; break;
    }
}

// Returns a pointer to a flash-stored character string that is the name of the radio protocol
const __FlashStringHelper *RadioProtocol(RADIO_PROTOCOL RP)          
{
    if ( RP > LAST_RADIOPROTOCOL) RP = PROTOCOL_NONE;
    const __FlashStringHelper *Names[LAST_RADIOPROTOCOL+1]={F("None Detected"),F("PPM"),F("SBus"),F("iBus")};
    return Names[RP];
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
// CONSTRUCT AND BEGIN (SETUPS)
// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
// Constructor
OP_Radio::OP_Radio()
{
    Protocol = PROTOCOL_NONE;   // We don't know yet what protocol we will detect
    didWeBegin = false;         // We have not yet gotten to the begin() function yet.
    ChannelsUtilized = 0;
    channelCount = 0;
    UsingSpecialPositions = false;
    InFailsafe = false;
    
    PPMFailed = false;
    SBusFailed = false;
    iBusFailed = false;
}


void OP_Radio::saveTimer(OP_SimpleTimer * t)
{   // All we do here is save a pointer to the sketch's SimpleTimer
    radioTimer = t;
}

void OP_Radio::detect(void)
{
// This function tries to detect a radio signal (PPM, SBus, or iBus). It tries one, if it fails, it tries the next,
// and keeps going until it tries the last one, and then returns to the first and starts over. It keeps trying so long as it's called, 
// so the calling routine needs to also be checking OP_Radio.Status(). When status returns READY_state, then the calling routine knows a protocol
// has been successfully detected. At that time the calling routine can check OP_Radio.getProtocol() to find out which one we found. 

static RADIO_PROTOCOL tryProtocol = PROTOCOL_SBUS;     // Whatever you set here, will be the first one checked
static boolean started = false;

    // START TRYING A PROTOCOL
    // ---------------------------------------------------------------------------->>
    if (started == false)
    {
        switch (tryProtocol)
        {
            
            case PROTOCOL_PPM:
                if (!PPMFailed)
                {   // Try to detect PPM
                    PPMDecoder = new PPMDecode;
                    PPMDecoder->begin();
                    // Start a try timer
                    radioTimer->setTimeout(PPM_TRY_TIME, failPPM);
                    started = true;
                    //Serial.println(F("Searching for PPM..."));
                }
                break;
                
            case PROTOCOL_SBUS:
                if (!SBusFailed)
                {   // See if we can detect SBus
                    SBusDecoder = new SBusDecode;
                    SBusDecoder->begin();
                    // Start a try timer
                    radioTimer->setTimeout(SBUS_TRY_TIME, failSBus);
                    started = true;
                    //Serial.println(F("Searching for SBus..."));
                }
                break;

            case PROTOCOL_iBUS:
                if (!iBusFailed)
                {   // See if we can detect iBus
                    iBusDecoder = new iBusDecode;
                    iBusDecoder->begin();
                    // Start a try timer
                    radioTimer->setTimeout(iBUS_TRY_TIME, failiBus);
                    started = true;
                    //Serial.println(F("Searching for iBus..."));
                }
                break;
        }
    }
    // KEEP TRYING A PROTOCOL
    // ---------------------------------------------------------------------------->>
    else
    {
        switch (tryProtocol)
        {
            case PROTOCOL_PPM:
                if (!PPMFailed && PPMDecoder->getState() == READY_state) Protocol = PROTOCOL_PPM;   // Set protocol to PPM
                break;
                
            case PROTOCOL_SBUS:
                if (!SBusFailed)
                {   // SBus needs to be polled
                    SBusDecoder->update();
                    if (SBusDecoder->getState() == READY_state) { Protocol = PROTOCOL_SBUS; }       // Set protocol to SBUS
                }
                break;
            
            case PROTOCOL_iBUS:
                if (!iBusFailed)
                {   // iBus needs to be polled
                    iBusDecoder->update();
                    if (iBusDecoder->getState() == READY_state) { Protocol = PROTOCOL_iBUS; }       // Set protocol to iBUS
                }
                break;
        }
    }

    // FAIL A PROTOCOL, AND TRY THE OTHER ONE INSTEAD
    // ---------------------------------------------------------------------------->>
    switch (tryProtocol)
    {
        case PROTOCOL_PPM:
            if (PPMFailed)
            {
                // Shutdown and deconstruct object
                PPMDecoder->shutdown();
                delete PPMDecoder;      

                // Try the next protocol - SBus
                SBusFailed = started = false;
                tryProtocol = PROTOCOL_SBUS;
            }
            break;
            
        case PROTOCOL_SBUS:
            if (SBusFailed) 
            {
                // Shutdown and deconstruct object
                SBusDecoder->shutdown();
                delete SBusDecoder;             
                
                // Try the next protocol - iBus
                iBusFailed = started = false;
                tryProtocol = PROTOCOL_iBUS;    
            }
            break;

        case PROTOCOL_iBUS:
            if (iBusFailed) 
            {
                // Shutdown and deconstruct object
                iBusDecoder->shutdown();
                delete iBusDecoder;             
                
                // Try the next protocol - PPM
                PPMFailed = started = false;
                tryProtocol = PROTOCOL_PPM;   
            }
            break;
    }
    
    // Every time this function is called, update the timer.
    // We may be calling this from the PCComm class outside of the sketch, 
    // so we can't count on the main loop updating it
    radioTimer->run();
}

void OP_Radio::failPPM(void)
{
    PPMFailed = true;
}

void OP_Radio::failSBus(void)
{
    SBusFailed = true;
}

void OP_Radio::failiBus(void)
{
    iBusFailed = true;
}


// Begin
// This initializes our channels to the settings saved in eeprom. 
void OP_Radio::begin(_eeprom_data *storage)
{
    // Run this so we know how many channels the radio even has
    getChannelCount();
    
    // We assume the radio has at least 4 channels for the two sticks, and we are going to use them
    ChannelsUtilized = 4;   // Later we will add any aux channels utilized as well

    // Initialize all channels as present = false
	Sticks.Throttle.present = false;
	Sticks.Turn.present = false;
	Sticks.Elevation.present = false;
	Sticks.Azimuth.present = false;
	for (uint8_t a=0; a<AUXCHANNELS; a++)
	{
		AuxChannel[a].present = false; 
	}
    
    // All channels are initalized to present = false. If the actual channel number is within the number of channels
    // detected in the PPM stream, we change the present flag to true. 
    // Load stick settings
        Sticks.Throttle.Settings = &storage->ThrottleSettings;
        Sticks.Throttle.ignore = false;
        if (Sticks.Throttle.Settings->channelNum <= channelCount) { Sticks.Throttle.present = true; }
        Sticks.Turn.Settings = &storage->TurnSettings;
        Sticks.Turn.ignore = false;
        if (Sticks.Turn.Settings->channelNum <= channelCount) { Sticks.Turn.present = true; }
        Sticks.Elevation.Settings = &storage->ElevationSettings;
        Sticks.Elevation.ignore = false;
        if (Sticks.Elevation.Settings->channelNum <= channelCount) { Sticks.Elevation.present = true; }
        Sticks.Azimuth.Settings = &storage->AzimuthSettings;
        Sticks.Azimuth.ignore = false;
        if (Sticks.Azimuth.Settings->channelNum <= channelCount) { Sticks.Azimuth.present = true; }

    // Another adjustment that needs to be made - if we are using the turret stick for special commands (stick held to corners and such), then 
    // we need to adjust our pulse min/max values for those channels. The reason being, at some point close to the stick extreme 
    // (TURRETSTICK_PULSESUBTRACT - defined in OP_h) we quit counting the stick position as a turret movement command, and start counting
    // it as a special position command. The way we do this is set the max pulse equal to the actual max pulse minus the PULSESUBTRACT amount.
    // This gives us the correct linear scale to turret movement commands in the GetStickCommand() function. And, by checking if our actual pulse
    // is greater than the saved pulse max, we can also determine if we are at a special position in the GetSpecialPosition() function. 
        UsingSpecialPositions = false;
        for (uint8_t t=0; t<MAX_FUNCTION_TRIGGERS; t++)
        {   // Check our triggers for any that match a turret stick position
            if (storage->SF_Trigger[t].TriggerID > 0 && storage->SF_Trigger[t].TriggerID <= MAX_SPEC_POS) { UsingSpecialPositions = true; break; }
        }
        // Ok, we have a special function assigned to a turret stick special position. We will adjust the linear
        // portion of the stick by modifying the pulse min and max 
        if (UsingSpecialPositions) AdjustTurretStickEndPoints();

        // Regardless we save the ignoreTurretDelay_mS setting locally. 
        // But, caveat - if the turret stick has been detached from the turret motor control, we set the delay to 0 (disable) regardless of what it might default or be set to otherwise
        if (storage->TurretRotationMotor == DRIVE_DETACHED && storage->TurretElevationMotor == DRIVE_DETACHED)
        {
            ignoreTurretDelay_mS = 0;
        }
        else
        {
            ignoreTurretDelay_mS = storage->IgnoreTurretDelay_mS;
        }

    // Initialize our abstract "special stick"
        SpecialStick.Position = MC;                  // Initialize to stick centered
    
    // Load aux channel settings, and determine the total number of "utilized" channels
        for (uint8_t a=0; a<AUXCHANNELS; a++)
        {
            AuxChannel[a].Settings = &storage->Aux_ChannelSettings[a];
            if (AuxChannel[a].Settings->channelNum > 0 && AuxChannel[a].Settings->channelNum <= channelCount) 
            { 
                AuxChannel[a].present = true; 
                ChannelsUtilized++; 
            }
        }
    
    // The number of utilized channels is not necessarily the same as the number of channels detected, but for sure it can't be 
    // greater than the number of channels detected, nor can it be greater than the maximum allowed number (COUNT_OP_CHANNELS)
    // Constrain our utilized channels just to be safe
    if (ChannelsUtilized > COUNT_OP_CHANNELS) ChannelsUtilized = COUNT_OP_CHANNELS;

    // Now we start off by initializing each channel to "not updated"
        ClearAllChannelUpdates();
    
    // Finally we create an array of pointers to a few common variables for each channel. This is a lot of code here, 
    // but it only gets run once. It will make reading the radio stream much quicker and shorter (see GetFrame)
    ptrCommonChannelSettings[0].pulse = &Sticks.Throttle.pulse;
    ptrCommonChannelSettings[0].channelNum = &Sticks.Throttle.Settings->channelNum;
    ptrCommonChannelSettings[0].updated = &Sticks.Throttle.updated;
    ptrCommonChannelSettings[0].started = &Sticks.Throttle.started;
    ptrCommonChannelSettings[0].present = &Sticks.Throttle.present;
   
    ptrCommonChannelSettings[1].pulse = &Sticks.Turn.pulse;
    ptrCommonChannelSettings[1].channelNum = &Sticks.Turn.Settings->channelNum;
    ptrCommonChannelSettings[1].updated = &Sticks.Turn.updated;
    ptrCommonChannelSettings[1].started = &Sticks.Turn.started;
    ptrCommonChannelSettings[1].present = &Sticks.Turn.present;
    
    ptrCommonChannelSettings[2].pulse = &Sticks.Elevation.pulse;
    ptrCommonChannelSettings[2].channelNum = &Sticks.Elevation.Settings->channelNum;
    ptrCommonChannelSettings[2].updated = &Sticks.Elevation.updated;
    ptrCommonChannelSettings[2].started = &Sticks.Elevation.started;
    ptrCommonChannelSettings[2].present = &Sticks.Elevation.present;
    
    ptrCommonChannelSettings[3].pulse = &Sticks.Azimuth.pulse;
    ptrCommonChannelSettings[3].channelNum = &Sticks.Azimuth.Settings->channelNum;
    ptrCommonChannelSettings[3].updated = &Sticks.Azimuth.updated;
    ptrCommonChannelSettings[3].started = &Sticks.Azimuth.started;
    ptrCommonChannelSettings[3].present = &Sticks.Azimuth.present;
    
    for (uint8_t i = 0; i < (ChannelsUtilized - 4); i++)
    {
        ptrCommonChannelSettings[4 + i].pulse = &AuxChannel[i].pulse;
        ptrCommonChannelSettings[4 + i].channelNum = &AuxChannel[i].Settings->channelNum;
        ptrCommonChannelSettings[4 + i].updated = &AuxChannel[i].updated;
        ptrCommonChannelSettings[4 + i].present = &AuxChannel[i].present;
    }
    
    
    // The begin() function is done - set a flag so we know we don't have to come here again. We do this becasue multiple objects have access to this class, 
    // namely the sketch and the OP_PCComm class. We won't know which one gets to it first, but whichever comes second will check to see if this flag has been
    // set, and if so, won't call the begin function again. 
    didWeBegin = true;
}

void OP_Radio::AdjustTurretStickEndPoints(void)
{
    // We want to adjust the linear portion of the stick by increasing the pulse min and decreasing the pulse max. 
    // Then pulses above these new min/maxes will be counted as special position triggers. 
    Sticks.Elevation.Settings->pulseMin += TURRETSTICK_PULSESUBTRACT;
    Sticks.Elevation.Settings->pulseMax -= TURRETSTICK_PULSESUBTRACT;
    Sticks.Azimuth.Settings->pulseMin += TURRETSTICK_PULSESUBTRACT;
    Sticks.Azimuth.Settings->pulseMax -= TURRETSTICK_PULSESUBTRACT;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
// GET COMMANDS - HIGH LEVEL RADIO READING
// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
boolean OP_Radio::GetCommands()
{
    boolean RxReady;
    static int IgnoreElevationTimerID = 0;
    static int IgnoreAzimuthTimerID = 0;    

    // If we're using it, the SBusDecoder needs to be polled
    pollSBus(); 

    // If we're using it, the iBusDecoder needs to be polled
    polliBus();     

    if (Status() == READY_state)
    {   // We have a lock on the Rx. 
        RxReady = true;

        // Has a new frame of data arrived yet? 
        if (NewFrame() == true)
        {
            restartWatchdog();      // Re-start the watchdog timer
            InFailsafe = false;     // If we were in failsafe, we aren't now

            // Get all channel pulses. 
            GetFrame();
    
            // Drive stick commands        
            if (Sticks.Throttle.updated)    { GetStickCommand(Sticks.Throttle);  }
            if (Sticks.Turn.updated)        { GetStickCommand(Sticks.Turn);      }
    
            // Turret stick regular commands
            if (Sticks.Elevation.updated)   { GetStickCommand(Sticks.Elevation); }
            if (Sticks.Azimuth.updated)     { GetStickCommand(Sticks.Azimuth);   }
            
            // Turret stick special position commands
            if (UsingSpecialPositions && (Sticks.Elevation.updated || Sticks.Azimuth.updated)) { GetSpecialPosition(SpecialStick); } 
            
            // If we are using the turret stick for special position commands, we may want to put a slight delay in the linear portion of the 
            // stick movement. If the stick is held over to a special position quickly, before the delay is up, the turret motors won't move - 
            // which they were not intended to in that case. 
            // The drawback of course is that the delay is present when we *do* want the turret motors to move. The longer the delay, the 
            // easier it is to trigger special commands at extreme stick movement without nudging the turret, but the worse the result is
            // when we do want to move the turret. 
            // It is a compromise no matter what. The only way to avoid it completely is put all special functions on auxillary switches, if your
            // radio has them. If you do so, this will be automatically disabled by setting UsingSpecialPositions = false.
            if (UsingSpecialPositions) 
            {
                if (SpecialStick.Position != MC) 
                {   // MC (middle center) covers the stick centered position but also the entire analog range of movement up to the actual corner limits. 
                    // So if SpecialStick is _not_ MC, that means it is at one of the extremes, and regardless of whether we have any ignoreTurretDelay set, 
                    // we want to ignore the stick movement at these points, and apply only the special stick command. 
                    Sticks.Elevation.ignore = Sticks.Azimuth.ignore = true; 
                }
                else if (ignoreTurretDelay_mS > 0)  // The rest only applies to stick movements right when they begin to leave center, and we only ignore them briefly if ignoreTurretDelay_mS is not 0
                {
                    // We do each stick separately, this is elevation
                    if (Sticks.Elevation.started) // If the stick was at center, but is now moving towards one of the edges
                    {
                        Sticks.Elevation.ignore = true;  // Ignore the commands
                        // Keep ignoring them until the timer is up
                        if (radioTimer->isEnabled(IgnoreElevationTimerID) == false) { IgnoreElevationTimerID = radioTimer->setTimeout(ignoreTurretDelay_mS, EnableElevationStick); }
                    }
                    else
                    {
                        // In this case we don't want to ignore it, so clear the ignore flag. 
                        if (radioTimer->isEnabled(IgnoreElevationTimerID) == false) { Sticks.Elevation.ignore = false; }
                    }

                    // This is azimuth
                    if (Sticks.Azimuth.started) // If the stick was at center, but is now moving towards one of the edges
                    {
                        Sticks.Azimuth.ignore = true;  // Ignore the commands
                        // Keep ignoring them until the timer is up
                        if (radioTimer->isEnabled(IgnoreAzimuthTimerID) == false) { IgnoreAzimuthTimerID = radioTimer->setTimeout(ignoreTurretDelay_mS, EnableAzimuthStick); }
                    }
                    else
                    {
                        // In this case we don't want to ignore it, so clear the ignore flag. 
                        if (radioTimer->isEnabled(IgnoreAzimuthTimerID) == false) { Sticks.Azimuth.ignore = false; }
                    }    
                }
                else
                {   // In this case special stick position is MC so we don't have to worry about that, and the ignoreTurretDelay_mS is 0 so we
                    // don't do any delay ignoring either. 
                    Sticks.Elevation.ignore = false;
                    Sticks.Azimuth.ignore = false;
                }
            }
    
            // Aux channel commands
            for (int a=0; a<AUXCHANNELS; a++)
            {   // If this aux channel is an "analog" input, then we already have the pulse, which is all we need. 
                // But if it is a digital "switch" input, we need to convert the pulse into a switch position.
                // In addition to calculating the switch postion, GetSwitchPosition also sets the updated flag to false if the
                // switch position hasn't changed, regardless of whether the pulse did change. Small changes in pulse do not necessarily
                // mean a new switch position, and we don't want to be triggering things unecessarily. 
                if (AuxChannel[a].present && AuxChannel[a].Settings->Digital && AuxChannel[a].updated) { GetSwitchPosition(a); }

            }
        }
        else
        {
            // We already read this frame. No update
            ClearAllChannelUpdates();
            
            // Start the watchdog timer, in case it hasn't been already
            startWatchdog();
        }
    }
    else
    {   
        // The receiver is not ready. Most likely another interrupt interrupted the PPM stream and the decoder thought the channel count got off. 
        // It will clear up next frame. 
        
        // No updates
        ClearAllChannelUpdates();
        
        // Come back later
        RxReady = false;
    }

    return RxReady;
}

void OP_Radio::startWatchdog()
{
    // Start the watchdog timer
    if (!radioTimer->isEnabled(WatchdogTimerID))
    {
        WatchdogTimerID = radioTimer->setTimeout(RADIO_FAILSAFE_MS, SetChannelsFailSafe);  
    }
}

void OP_Radio::restartWatchdog()
{
    if (radioTimer->isEnabled(WatchdogTimerID))
    {   
        // Re-set the timer to the beginning
        radioTimer->restartTimer(WatchdogTimerID);
    }
    else
    {   // In this case the timer hasn't been created yet, so do it now. 
        startWatchdog();
    }
}


// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
// GET FRAME - LOW LEVEL RADIO READING
// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
void OP_Radio::GetFrame()
{
    int16_t Pulse;                         // Temp var to save typing
    int16_t NewPulse[ChannelsUtilized];    // Temporary array of pulses

    // Fill our NewPulse array with the new frame of data
    switch (Protocol)
    {
        case PROTOCOL_PPM:
            PPMDecoder->GetPPM_Frame(NewPulse, ChannelsUtilized);
            break;
            
        case PROTOCOL_SBUS:
            SBusDecoder->GetSBus_Frame(NewPulse, ChannelsUtilized);
            break;      

        case PROTOCOL_iBUS:
            iBusDecoder->GetiBus_Frame(NewPulse, ChannelsUtilized);
            break;      
        
        case PROTOCOL_NONE:
        default:
            // Bad news bears, hope we don't end up here
            return;
    }
    
    // Run through the number of channels being utilized
    for (uint8_t i=0; i<ChannelsUtilized; i++)
    {
        if (*ptrCommonChannelSettings[i].present == true)
        {
            Pulse = NewPulse[*ptrCommonChannelSettings[i].channelNum-1];    // We subtract one because in the array the channel numbers start at 0, not 1
            if (*ptrCommonChannelSettings[i].pulse == Pulse) { *ptrCommonChannelSettings[i].updated = false; }
            else {*ptrCommonChannelSettings[i].updated = true; *ptrCommonChannelSettings[i].pulse = Pulse; }
        }
    }

    return;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
// GET FRAME IN STRING FORMAT - LOW LEVEL RADIO READING
// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
// This is similar to the above in that it obtains a frame of pulse widths from either the PPM, SBus or iBus decoders, but instead of putting them into our channel objects, 
// it assembles them into a string and passes them back to the calling function in the form of a character array. This is used to send pulse widths to the PC. 
void OP_Radio::GetStringFrame(char *chrArray, uint8_t buffer, uint8_t &StrLength, char delimiter, uint8_t HiLo)
{
// We only return 8 channels at a time. For SBus and iBus, you can request LOW or HIGH which will return either channels 1-8 or 9-16
#define COUNT_STRING_CHANNELS 8    
    
    uint8_t StartChan;
    uint8_t EndChan;
    int16_t NewPulse[channelCount];        // Temporary array of pulses - in this case we always use all channels available, even if more than ChannelsUtilized
    String str = "";                       // String

    // Initialize total length
    StrLength = 0;

    // Fill our NewPulse array with the new frame of data
    switch (Protocol)
    {
        case PROTOCOL_PPM:
            HiLo = LOW; // By definition, PPM only goes up to 8 channels, so we only return the low 8 regardless of what was passed
            PPMDecoder->GetPPM_Frame(NewPulse, channelCount);
            break;
        
        case PROTOCOL_SBUS:
            // We will only call this function if we checked for a new frame first, so we don't need to poll SBus again here
            SBusDecoder->GetSBus_Frame(NewPulse, channelCount);
            break;      

        case PROTOCOL_iBUS:
            // We will only call this function if we checked for a new frame first, so we don't need to poll iBus again here
            iBusDecoder->GetiBus_Frame(NewPulse, channelCount);
            break;      

        case PROTOCOL_NONE:
        default:
            // Bad news bears, hope we don't end up here
            return;
    }

    // reserve space for string
    str.reserve(5*(COUNT_STRING_CHANNELS+1));   // Each channel can take up to 5 characters including delimiter. Set the string size 1 more than this to be safe.
    
    // Decide which (up to) 8 channels to send
    if (channelCount <= COUNT_STRING_CHANNELS)
    {   // We can fit all channels into a single string
        StartChan = 0;
        EndChan = channelCount;
    }
    else if (HiLo == LOW)
    {   // We have more channels than can fit, and we are only going to send the first half
        StartChan = 0;
        EndChan = COUNT_STRING_CHANNELS;
    }
    else if (HiLo == HIGH)
    {   // We have more channels than can fit, and we are sending all channels beyond the first half
        StartChan = COUNT_STRING_CHANNELS;
        EndChan = channelCount;
    }

    // Create a sring of channels 
    for (uint8_t i=StartChan; i<EndChan; i++)
    {
        str.concat(String(NewPulse[i], DEC));
        str.concat(delimiter);
    }

    // Convert string to char array and store in the array that was passed
    str.toCharArray(chrArray, buffer); 
    StrLength = str.length();
}


// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
// COMMANDS - STICKS
// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
void OP_Radio::GetStickCommand(stick_channel &ch)
{
    // If the last command was zero, this will be false, otherwise true. 
    boolean WasSomething = ch.command;

    if      (ch.pulse >= (ch.Settings->pulseCenter + ch.Settings->deadband))
    {
        if  (ch.Settings->reversed) ch.command = map(ch.pulse, ch.Settings->pulseCenter, ch.Settings->pulseMax, 0, MOTOR_MAX_REVSPEED);
        else                        ch.command = map(ch.pulse, ch.Settings->pulseCenter, ch.Settings->pulseMax, 0, MOTOR_MAX_FWDSPEED);
    }
    else if (ch.pulse <= (ch.Settings->pulseCenter - ch.Settings->deadband))
    {
        if  (ch.Settings->reversed) ch.command = map(ch.pulse, ch.Settings->pulseMin, ch.Settings->pulseCenter, MOTOR_MAX_FWDSPEED, 0);
        else                        ch.command = map(ch.pulse, ch.Settings->pulseMin, ch.Settings->pulseCenter, MOTOR_MAX_REVSPEED, 0);
    }
    else
    {   
        ch.command = 0;
        if (!WasSomething) ch.updated = false;  // In this case, it was zero to start with, and is still zero. Even though the pulse might have changed slightly, 
                                                // the command didn't really update (basically we are still within deadband).
    }

    if (ch.command != 0)
    {   // Keep the command in limits
        ch.command = constrain(ch.command, MOTOR_MAX_REVSPEED, MOTOR_MAX_FWDSPEED);
        // If the new command is something, and the last command was nothing (0), then we set the started flag. 
        ch.started = !WasSomething;  
    }
    
    return;
}


// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
// COMMANDS - ABSTRACT "SPECIAL STICK"
// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
// This only gets called if you have assigned special function triggers to turret stick positions. If you have put all your special function
// triggers on auxillary channels, this never gets used. 
int OP_Radio::GetSpecialPosition(sf_channel &sfc)
{   // The analog stick used for turret control is also used as a 9-position switch when the stick is moved to any of the four corners or
    // the extreme up/down/left/right, or the center. Here we determine if the stick is at one of these special positions. 
    int StickPos = 0;
    // How can pulse be greater or less than pulseMax or pulseMin? See LoadChannelSettings() below. If function triggers have been assigned to
    // any special positions, we modify pulseMin and Max by TURRETSTICK_PULSESUBTRACT (defined in OP_Radio.h). Subsequently, values up to
    // those adjusted amounts are treated as linear stick movements, values beyond it are special positions
    if      (Sticks.Elevation.pulse > Sticks.Elevation.Settings->pulseMax)
    {       Sticks.Elevation.Settings->reversed ? StickPos |= Bottom  : StickPos |= Top; }
    else if (Sticks.Elevation.pulse < Sticks.Elevation.Settings->pulseMin)
    {       Sticks.Elevation.Settings->reversed ? StickPos |= Top     : StickPos |= Bottom; }
    else    StickPos |= Middle;
    if      (Sticks.Azimuth.pulse > Sticks.Azimuth.Settings->pulseMax)
    {       Sticks.Azimuth.Settings->reversed ? StickPos |= Left  : StickPos |= Right; }
    else if (Sticks.Azimuth.pulse < Sticks.Azimuth.Settings->pulseMin)
    {       Sticks.Azimuth.Settings->reversed ? StickPos |= Right : StickPos |= Left; }
    else    StickPos |= Center;

    // In the above we added these values:
    // Elevation channel: 
    // Top      = 32,       // 00100000     at the top
    // Middle   = 16,       // 00010000     in the middle (between top and bottom)
    // Bottom   = 8,        // 00001000     at the bottom
    // Azimuth channel:
    // Left     = 4,        // 00000100     at the left
    // Center   = 2,        // 00000010     in the center (between left and right)
    // Right    = 1         // 00000001     at the right

    // So now our StickPos is one of these 9 numbers. Assigning one of these numbers as the trigger ID 
    // of a special function, will cause that function to be called whenever the stick is in that position. 
    // Top left      = 36    (32+4)
    // Top center    = 34    (32+2)
    // Top right     = 33    (32+1)
    // Middle left   = 20    (16+4)
    // Middle center = 18    (16+2)    - this is stick untouched
    // Middle right  = 17    (16+1)
    // Bottom left   = 12    (8+4)
    // Bottom center = 10    (8+2)
    // Bottom right  = 9     (8+1)

    // Return updated flag and new value. 
    if (sfc.Position == StickPos)
    {
        sfc.updated = false;
    }
    else
    {
        sfc.Position = StickPos;
        sfc.updated = true;
    }

    return StickPos;
}
// If using the "special stick", we will ignore some movements of the turret sticks, see GetCommands above. 
// When the brief ignore period is over, we re-enable these flags. 
void OP_Radio::EnableElevationStick(void)
{
    // Clear the ignore flag
    Sticks.Elevation.ignore = false;
}
void OP_Radio::EnableAzimuthStick(void)
{
    // Clear the ignore flag
    Sticks.Azimuth.ignore = false;
}



// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
// COMMANDS - DIGITAL AUX CHANNELS 
// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
void OP_Radio::GetSwitchPosition(int a)
{
    int pulse = AuxChannel[a].pulse;

    uint8_t numPos = AuxChannel[a].Settings->numPositions;

    // Thanks to Rob Tillaart for the distance function
    // http://forum.arduino.cc/index.php?topic=254836.0

    // This takes a pulse and returns the switch position with the 
    // nearest matching pulse
    
    byte POS = 0; 
    int d = 9999;
    int distance = abs(RC_MULTISWITCH_START_POS - pulse);

    for (int i = 1; i < numPos; i++)
    {
        switch (numPos)
        {
            case 2:  d = abs(MultiSwitch_MatchArray2[i] - pulse);   break;
            case 3:  d = abs(MultiSwitch_MatchArray3[i] - pulse);   break;
            case 4:  d = abs(MultiSwitch_MatchArray4[i] - pulse);   break;
            case 5:  d = abs(MultiSwitch_MatchArray5[i] - pulse);   break;
            case 6:  d = abs(MultiSwitch_MatchArray6[i] - pulse);   break;
            case 7:  d = abs(MultiSwitch_MatchArray7[i] - pulse);   break;
            case 8:  d = abs(MultiSwitch_MatchArray8[i] - pulse);   break;
            case 9:  d = abs(MultiSwitch_MatchArray9[i] - pulse);   break;
            case 10: d = abs(MultiSwitch_MatchArray10[i] - pulse);   break;
            case 11: d = abs(MultiSwitch_MatchArray11[i] - pulse);   break;
            case 12: d = abs(MultiSwitch_MatchArray12[i] - pulse);   break;            
        }

        if (d < distance)
        {
            POS = i;
            distance = d;
        }
    }
    
    // Add 1 to POS because from here we don't want zero-based
    POS += 1;
    
    // Swap positions if channel is reversed. 
    if (AuxChannel[a].Settings->reversed)
    {
        switch (numPos)
        {
            case 2: 
                if (POS == Pos1) POS = Pos2; 
                else POS = Pos1; 
                break;
            case 3: 
                if (POS == Pos1) POS = Pos3; 
                else if (POS == Pos3) POS = Pos1; 
                break;
            case 4: 
                if (POS == Pos1) POS = Pos4; 
                else if (POS == Pos2) POS = Pos3; 
                else if (POS == Pos3) POS = Pos2; 
                else if (POS == Pos4) POS = Pos1; 
                break;
            case 5: 
                if (POS == Pos1) POS = Pos5; 
                else if (POS == Pos2) POS = Pos4; 
                else if (POS == Pos4) POS = Pos2; 
                else if (POS == Pos5) POS = Pos1; 
                break;                
            case 6: 
                if (POS == Pos1) POS = Pos6; 
                else if (POS == Pos2) POS = Pos5; 
                else if (POS == Pos3) POS = Pos4; 
                else if (POS == Pos4) POS = Pos3; 
                else if (POS == Pos5) POS = Pos2; 
                else if (POS == Pos6) POS = Pos1; 
                break;     
            case 7:
                if (POS == Pos1) POS = Pos7; 
                else if (POS == Pos2) POS = Pos6; 
                else if (POS == Pos3) POS = Pos5; 
                else if (POS == Pos5) POS = Pos3; 
                else if (POS == Pos6) POS = Pos2; 
                else if (POS == Pos7) POS = Pos1; 
                break;
            case 8:
                if (POS == Pos1) POS = Pos8; 
                else if (POS == Pos2) POS = Pos7; 
                else if (POS == Pos3) POS = Pos6; 
                else if (POS == Pos4) POS = Pos5; 
                else if (POS == Pos5) POS = Pos4; 
                else if (POS == Pos6) POS = Pos3;             
                else if (POS == Pos7) POS = Pos2; 
                else if (POS == Pos8) POS = Pos1; 
                break;
            case 9:
                if (POS == Pos1) POS = Pos9; 
                else if (POS == Pos2) POS = Pos8; 
                else if (POS == Pos3) POS = Pos7; 
                else if (POS == Pos4) POS = Pos6; 
                else if (POS == Pos6) POS = Pos4; 
                else if (POS == Pos7) POS = Pos3;             
                else if (POS == Pos8) POS = Pos2; 
                else if (POS == Pos9) POS = Pos1;             
                break;
            case 10:
                if (POS == Pos1) POS = Pos10; 
                else if (POS == Pos2) POS = Pos9; 
                else if (POS == Pos3) POS = Pos8; 
                else if (POS == Pos4) POS = Pos7; 
                else if (POS == Pos5) POS = Pos6; 
                else if (POS == Pos6) POS = Pos5;
                else if (POS == Pos7) POS = Pos4;             
                else if (POS == Pos8) POS = Pos3; 
                else if (POS == Pos9) POS = Pos2;             
                else if (POS == Pos10) POS = Pos1; 
                break;
            case 11:
                if (POS == Pos1) POS = Pos11; 
                else if (POS == Pos2) POS = Pos10; 
                else if (POS == Pos3) POS = Pos9; 
                else if (POS == Pos4) POS = Pos8; 
                else if (POS == Pos5) POS = Pos7; 
                else if (POS == Pos7) POS = Pos5;
                else if (POS == Pos8) POS = Pos4;             
                else if (POS == Pos9) POS = Pos3; 
                else if (POS == Pos10) POS = Pos2;             
                else if (POS == Pos11) POS = Pos1; 
                break;
            case 12:
                if (POS == Pos1) POS = Pos12; 
                else if (POS == Pos2) POS = Pos11; 
                else if (POS == Pos3) POS = Pos10; 
                else if (POS == Pos4) POS = Pos9; 
                else if (POS == Pos5) POS = Pos8; 
                else if (POS == Pos6) POS = Pos7;
                else if (POS == Pos7) POS = Pos6;             
                else if (POS == Pos8) POS = Pos5; 
                else if (POS == Pos9) POS = Pos4;             
                else if (POS == Pos10) POS = Pos3;             
                else if (POS == Pos11) POS = Pos2;             
                else if (POS == Pos11) POS = Pos1; 
                break;                
        }    
    }
    
    // We re-set the Updated flag here even though it was already set before, because a small change in the pulse might not 
    // actually result in any change in the switchPos
    if (AuxChannel[a].switchPos == POS)
    {
        AuxChannel[a].updated = false;
    }
    else
    {
        AuxChannel[a].updated = true;
        AuxChannel[a].switchPos = POS;
    }    
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
// FAILSAFE
// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
// If the watchdog timer expires, this function will be called
void OP_Radio::SetChannelsFailSafe()
{   
    if (!InFailsafe)    
    {
        Sticks.Throttle.pulse = Sticks.Throttle.Settings->pulseCenter; 
        Sticks.Throttle.command     = 0;
        Sticks.Turn.pulse = Sticks.Turn.Settings->pulseCenter; 
        Sticks.Turn.command         = 0;
        Sticks.Elevation.pulse = Sticks.Elevation.Settings->pulseCenter; 
        Sticks.Elevation.command    = 0;
        Sticks.Azimuth.pulse = Sticks.Azimuth.Settings->pulseCenter; 
        Sticks.Azimuth.command      = 0;
        SpecialStick.Position = MC;    // Middle/center, means no special command
        // We could put the aux channels to middle, but we have no idea what the user assigned to those positions. Best to just leave them at their last values. 
        //for (uint8_t a=0; a<AUXCHANNELS; a++)
        //{   
        //    AuxChannel[a].switchPos = Pos3;
        //    AuxChannel[a].pulse = 1500;
        //}
        SetAllChannelUpdates();    // This sets the updated flag for every channel. We want the main code to read the new failsafe values we have just written above. 
        InFailsafe = true;         // Set the failsafe flag. The sketch will check this flag in order to take its own actions on failsafe. 
    }
}


// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
// UTILITIES
// ---------------------------------------------------------------------------------------------------------------------------------------------------->>
boolean OP_Radio::hasBegun(void)
{   // We want to know if we've already called the begin() function or not
    return didWeBegin;
}
RADIO_PROTOCOL OP_Radio::getProtocol(void)
{
    return Protocol;
}

decodeState_t OP_Radio::Status(void)
{
    switch (Protocol)
    {
        case PROTOCOL_PPM:
            return PPMDecoder->getState();
            break;
        
        case PROTOCOL_SBUS:
            pollSBus();     // SBus needs polled
            return SBusDecoder->getState();
            break;
            
        case PROTOCOL_iBUS:
            polliBus();     // iBus needs polled
            return iBusDecoder->getState();
            break;

        case PROTOCOL_NONE:
        default:
            return ACQUIRING_state;
    }
}

boolean OP_Radio::NewFrame(void)
{
    switch (Protocol)
    {
        case PROTOCOL_PPM: 
            return PPMDecoder->NewFrame;
            break;
        
        case PROTOCOL_SBUS:
            pollSBus();     // SBus needs polled
            return SBusDecoder->NewFrame;
            break;
            
        case PROTOCOL_iBUS:
            polliBus();     // iBus needs polled
            return iBusDecoder->NewFrame;
            break;
            
        case PROTOCOL_NONE:
        default:
            return false;
    }
}

uint8_t OP_Radio::getChannelCount(void)
{
    // This sets & returns the actual number of channels detected in the radio data stream. 
    // It does not necessarily represent the number of channels we will use
    switch (Protocol)
    {
        case PROTOCOL_PPM: 
            channelCount = PPMDecoder->getChanCount();
            break;
        
        case PROTOCOL_SBUS:
            channelCount = SBusDecoder->getChanCount();
            break;
        
        case PROTOCOL_iBUS:
            channelCount = iBusDecoder->getChanCount();
            break;
        
        case PROTOCOL_NONE:
        default:
            channelCount = 0;
    }
    
    return channelCount;
}

void OP_Radio::ClearAllChannelUpdates()
{
    Sticks.Throttle.updated = false;
    Sticks.Turn.updated = false;
    Sticks.Elevation.updated = false;
    Sticks.Azimuth.updated = false;
    SpecialStick.updated = false;  
    for (uint8_t a=0; a<AUXCHANNELS; a++) { AuxChannel[a].updated = false; }
}

void OP_Radio::SetAllChannelUpdates()
{
    Sticks.Throttle.updated = true;
    Sticks.Turn.updated = true;
    Sticks.Elevation.updated = true;
    Sticks.Azimuth.updated = true;
    SpecialStick.updated = true;  
    for (uint8_t a=0; a<AUXCHANNELS; a++) { AuxChannel[a].updated = true; }
}

void OP_Radio::Update(void)
{   
    pollSBus();
    polliBus();
    // We don't need to update radioTimer because it is just a pointer to the sketch's timer, 
    // and the sketch will update that itself. 
}

void OP_Radio::pollSBus(void)
{   // This checks if we're using the SBus protocol, and if so, updates it
    if (Protocol == PROTOCOL_SBUS) { SBusDecoder->update(); }
}

void OP_Radio::polliBus(void)
{   // This checks if we're using the iBus protocol, and if so, updates it
    if (Protocol == PROTOCOL_iBUS) { iBusDecoder->update(); }
}

void OP_Radio::slowDownForPCComm(void)
{
    // Some protocols may operate at a speed too fast for reliable streaming to the PC, 
    // we can use this to slow them down temporarily when performing Read Radio from OP Config
    switch (Protocol)
    {
        case PROTOCOL_PPM:                                      break;   // No slow down implemented for PPM
        case PROTOCOL_SBUS: SBusDecoder->slowDownForPCComm();   break;
        case PROTOCOL_iBUS: iBusDecoder->slowDownForPCComm();   break;
    }
}
    
void OP_Radio::defaultSpeed(void)
{
    // This reverts the protocol back to its default speed for normal operation
    switch (Protocol)
    {
        case PROTOCOL_PPM:                                      break;   // No change needed for PPM
        case PROTOCOL_SBUS: SBusDecoder->defaultSpeed();        break;
        case PROTOCOL_iBUS: iBusDecoder->defaultSpeed();        break;
    }
}

