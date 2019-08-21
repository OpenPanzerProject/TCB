/* OP_Radio.h       Open Panzer Radio - radio class for converting inputs (from various radio types) into more useful channel objects. 
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
 
#ifndef OP_RADIO_H
#define OP_RADIO_H

#include <Arduino.h>
#include "../OP_EEPROM/OP_EEPROM.h"
#include "OP_RadioDefines.h"
#include "../OP_PPMDecode/OP_PPMDecode.h"
#include "../OP_SBusDecode/OP_SBusDecode.h"
#include "../OP_IBusDecode/OP_iBusDecode.h"
#include "../OP_Motors/OP_Motors.h"
#include "../OP_SimpleTimer/OP_SimpleTimer.h"

typedef char RADIO_PROTOCOL; 
#define PROTOCOL_NONE   0                   // Unknown or none detected
#define PROTOCOL_PPM    1                   // PPM protocol
#define PROTOCOL_SBUS   2                   // SBus protocol
#define PROTOCOL_iBUS   3                   // iBus protocol
#define FIRST_RADIOPROTOCOL PROTOCOL_PPM    // So we know how to identify invalid values
#define LAST_RADIOPROTOCOL  PROTOCOL_iBUS   // 
const __FlashStringHelper *RadioProtocol(RADIO_PROTOCOL RP); // Returns a pointer to a flash-stored character string that is the name of the radio protocol

// This one is to print out turret stick positions
const __FlashStringHelper *TurretStickPosition(uint8_t TSP); // Returns a pointer to a flash-stored character string that is name of turret stick position

// This array is the length in characters of the text description of each turret stick position (as defined in OP_Radio.cpp)
const PROGMEM uint8_t TurretStickPositionStringLengths[SPECIALPOSITIONS+1] = { 7, 8, 9, 9, 11, 12, 12, 11, 12, 12 }; // We add one for "unknown"
#define TS_PositionString_Length(p) pgm_read_byte_far(&TurretStickPositionStringLengths[p])

                                    // From OP_PCComm.cpp we know we only have 800 mS to try all three protocols on the Read Radio routine (actually we can have up to 1 second but 
                                    // the setting is 800mS). So we set these to something that will let us try all three in that time frame. 1/4 second should be enough. 
#define PPM_TRY_TIME        250     // How long to try detecting a PPM signal, in mS. Only used in detect mode at startup. 
#define SBUS_TRY_TIME       250     // How long to try detecting an SBus signal, in mS. Only used in detect mode at startup. 
#define iBUS_TRY_TIME       250     // How long to try detecting an iBus signal, in mS. Only used in detect mode at startup. 

#define RADIO_FAILSAFE_MS   500     // If we exceed this amount of time in milliseconds without reading a valid radio frame, go into failsafe. 
                                    // 250 milliseconds is 1/4 second. That is a long time for an RC receiver, normally 12 PPM frames and over 25 SBus
                                    // frames would have arrived in that time (although we throw away every other SBus frame so in fact it's about the same as PPM). 
                                    // Nevertheless we give ourselves extra leeway with 1/2 second. 

class OP_Radio
{
    public: 
        OP_Radio();                                                     // Constructor
        static void             saveTimer(OP_SimpleTimer * t);          // Get a reference to the sketch's SimpleTimer
        static void             begin(_eeprom_data *storage);           // This loads the save eeprom information into the radio object, and does basic initialization
        
        static void             detect();                               // See what kind of signal is attached
        static boolean          hasBegun(void);                         // Did we already call the begin() function yet? 
        static boolean          GetCommands();                          // High level command handler
        static RADIO_PROTOCOL   getProtocol();                          // Return currently detected protocol
        static decodeState_t    Status(void);                           // This actually returns the status of the PPM or SBus decoder
        static boolean          NewFrame(void);                         // Is a new PPM frame available
        static uint8_t          getChannelCount(void);                  // How many channels did the PPM decoder detect. Once set, this value doesn't change until reboot.
        static int              ChannelsUtilized;                       // This is the number of channels utilized, assuming you already calculated it (getChannelCount calculates it too) 

        static boolean          UsingSpecialPositions;                  // Are any function triggers assigned to the "special stick" (turret stick special positions)
        static void             AdjustTurretStickEndPoints(void);       // If we are using special positions, we will need to artificially adjust the end-points of the turret stick
        static boolean          InFailsafe;                             // Are we in failsafe due to some radio problem?            

        static stick_channels   Sticks;                                 // Creates a collection of linear channels named Throttle, Turn, Elevation, Azimuth
        static sf_channel       SpecialStick;                           // This holds information about the turret stick, and whether it is being held in a position to indicate a special command
        static aux_channels     AuxChannel[AUXCHANNELS];                // Create AUXCHANNELS number of aux_channels

        static void             Update(void);                           // Update the radioTimer, and poll the SBus decoder if we're using it (PPM updates itself automatically)
        static void             GetStringFrame(char *chrArray, uint8_t buffer, uint8_t &StrLength, char delimiter, uint8_t HiLo = LOW); // Returns a string of pulses separated by delimiter. Used for PC comms
        static void             slowDownForPCComm(void);                // Some protocols may operate at a speed too fast for reliable streaming to the PC, we can use this to slow them down temporarily when performing Read Radio from OP Config
        static void             defaultSpeed(void);                     // This reverts the protocol back to its default speed for normal operation

    private:
    
        static boolean          didWeBegin;                             // Has the begin() function been called? 
        static RADIO_PROTOCOL   Protocol;                               // Which protocol detected
        static                  PPMDecode *PPMDecoder;                  // PPM Decoder object       
        static                  SBusDecode *SBusDecoder;                // SBus Decoder object
        static                  iBusDecode *iBusDecoder;                // iBus Decoder object
        static void             GetFrame(void);                         // Request a frame from the PPM/SBus decoder
        static void             GetStickCommand(stick_channel &ch);     // Calculate the four stick channel positions
        static int              GetSpecialPosition(sf_channel &sfc);    // Calculate the abstract "special stick" position, if used
        static void             EnableElevationStick(void);             // Re-enable this stick after a brief ignore delay
        static void             EnableAzimuthStick(void);               // Re-enable this stick after a brief ignore delay
        static void             GetSwitchPosition(int a);               // Calculate aux channel switch positions, if aux channel is set to digital input
    
        static void             SetChannelsFailSafe();                  // We lost connection with the radio. Set all channels to failsafe values. 
        static void             ClearAllChannelUpdates(void);           // Set all channels to "not updated"
        static void             SetAllChannelUpdates(void);             // Set all channels to "updated"
        
        static void             startWatchdog(void);                    // Create a watchdog timer
        static void             restartWatchdog(void);                  // Re-starts the watchdog timer. We call this every time we get a new frame of data from the radio. 
        static int              WatchdogTimerID;
        
        static void             failPPM(void);                          // If we fail to read PPM
        static void             failSBus(void);                         // If we fail to read SBus
        static void             failiBus(void);                         // If we fail to read iBus
        static boolean          PPMFailed;                              // Are we trying to detect PPM? 
        static boolean          SBusFailed;                             // Are we trying to detect SBus?
        static boolean          iBusFailed;                             // Are we trying to detect iBus?        
        static void             pollSBus(void);                         // SBus needs polling
        static void             polliBus(void);                         // iBus needs polling
        
        static OP_SimpleTimer * radioTimer;                             // Used for watchdog timer and other stuff. Pointer to the sketch's SimpleTimer, rather than creating a new instance of the class. 
        static uint8_t          channelCount;                           // How many channels were detected in the PPM stream
        static common_channel_settings  ptrCommonChannelSettings[(STICKCHANNELS + AUXCHANNELS)];    // This array of pointers to common channel settings for all channels allows us to loop through them quickly, see GetPPMFrame() in RadioInputs tab. 
                                                                                                    // And yes, it says "ptr" but you see no *. But look in OP_RadioDefines.h for the struct definition, it is all pointers. 
        static int16_t          ignoreTurretDelay_mS;                   // Local copy of the user variable stored in eeprom

};









#endif
