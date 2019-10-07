/* OP_RadioDefines.h    Open Panzer Radio - defines and structs related to the radio and channels
 * Source:              openpanzer.org              
 * Authors:             Luke Middleton
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
 
#ifndef OP_RADIODEFINES_H
#define OP_RADIODEFINES_H

#include <Arduino.h>

// CHANNEL COUNTS                                               
const byte COUNT_OP_CHANNELS    = 16;   // We can read up to 8 PPM channels (defined in OP_PPMDecode.h) or 16 SBUS channels (OP_SBusDecode.h)
                                        // If we wished to use less than that we could set this to less, but we will leave it at 16
                                        // 4 stick channels and (up to) 12 aux channels
const byte STICKCHANNELS        = 4;    // There are always 4 channels associated with the two transmitter sticks, no more, no less
const byte AUXCHANNELS          = 12;   // There can be (up to) 12 aux channels


// Don't change the order of the elements in the following structs! 
// If you do, you will need to adjust OP_EEPROM_VarInfo.h and a bunch of other stuff too

//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// STICK CHANNELS (the four sticks)
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
#define DEFAULT_DEADBAND          15    // Default stick deadband (pulses less than deadband away from stick center are ignored)

typedef struct stick_channel_settings{  // Settings are saved to EEPROM
    uint8_t channelNum;                 // Which number are we in the PPM input stream
    int16_t pulseMin;                   // PPM pulse min
    int16_t pulseMax;                   // PPM pulse max
    int16_t pulseCenter;                // PPM pulse center
    uint8_t deadband;                   // Pulse values below deadband are ignored
    boolean reversed;                   // Is the channel reversed
};

typedef struct stick_channel {
    boolean present;                    // Is this channel present? It should be. 
    boolean updated;                    // Has the command changed since last check
    boolean started;                    // If the prior command was Zero (nothing), and this command is Something, set started = true
    boolean ignore;                     // Ignore the command. Only implemented on turret sticks (elevation and azimuth)
    int16_t pulse;                      // Current PPM pulse
    int16_t command;                    // scaled command
    stick_channel_settings *Settings;   // Common settings
};        

typedef struct stick_channels {
    stick_channel Throttle;
    stick_channel Turn;
    stick_channel Elevation;
    stick_channel Azimuth;
};



//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// AUX CHANNELS (switches or knobs)
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
enum switch_positions : byte {          // Names for the switch positions
    NullPos = 0,
    Pos1,
    Pos2,
    Pos3,
    Pos4,
    Pos5,
    Pos6,
    Pos7,
    Pos8,
    Pos9,
    Pos10,
    Pos11,
    Pos12
};

typedef struct aux_channel_settings{    // Settings are saved to EEPROM
    uint8_t channelNum;                 // Which number are we in the PPM input stream
    int16_t pulseMin;                   // PPM pulse min
    int16_t pulseMax;                   // PPM pulse max    
    int16_t pulseCenter;                // PPM pulse center 
    boolean Digital;                    // Is this a digital channel (switch input) or an analog knob input? 
    uint8_t numPositions;               // If digital, how many positions does this switch have
    boolean reversed;                   // Is the channel reversed
};

typedef struct aux_channels {
    boolean present;                    // Is this channel connected or present?
    boolean updated;                    // Has the value changed since last time? 
    int16_t pulse;                      // PPM pulse for this channel - if channel is analog input
    switch_positions switchPos;         // Current switch position, calculated from PPM pulse
    aux_channel_settings *Settings;
};

#define RC_MULTISWITCH_START_POS    1000
const int16_t MultiSwitch_MatchArray2[2] = {
    1000,  // 0
    2000   // 1
};
const int16_t MultiSwitch_MatchArray3[3] = {
    1000,  // 0
    1500,  // 1
    2000   // 2
};
const int16_t MultiSwitch_MatchArray4[4] = {
    1000,  // 0
    1333,  // 1
    1667,  // 2
    1800   // 3
};
const int16_t MultiSwitch_MatchArray5[5] = {
    1000,  // 0
    1250,  // 1
    1500,  // 2
    1750,  // 3
    1800   // 4
};
const int16_t MultiSwitch_MatchArray6[6] = {
    1000,  // 0
    1200,  // 1
    1400,  // 2
    1600,  // 3
    1800,  // 4
    2000   // 5
};
const int16_t MultiSwitch_MatchArray7[7] = {
    1000,   // 0
    1167,   // 1
    1333,   // 2
    1500,   // 3
    1667,   // 4
    1833,   // 5
    2000    // 6
};
const int16_t MultiSwitch_MatchArray8[8] = {
    1000,   // 0
    1143,   // 1
    1286,   // 2
    1429,   // 3
    1571,   // 4
    1714,   // 5
    1857,   // 6
    2000    // 7
};
const int16_t MultiSwitch_MatchArray9[9] = {
    1000,   // 0
    1125,   // 1
    1250,   // 2
    1375,   // 3
    1500,   // 4
    1625,   // 5
    1750,   // 6
    1875,   // 7
    2000    // 8
};
const int16_t MultiSwitch_MatchArray10[10] = {
    1000,   // 0
    1111,   // 1
    1222,   // 2
    1333,   // 3
    1444,   // 4
    1556,   // 5
    1667,   // 6
    1778,   // 7
    1889,   // 8
    2000    // 9
};
const int16_t MultiSwitch_MatchArray11[11] = {
    1000,   // 0
    1100,   // 1
    1200,   // 2
    1300,   // 3
    1400,   // 4
    1500,   // 5
    1600,   // 6
    1700,   // 7
    1800,   // 8
    1900,   // 9
    2000    // 10
};
const int16_t MultiSwitch_MatchArray12[12] = {
    1000,   // 0
    1091,   // 1
    1182,   // 2
    1273,   // 3
    1364,   // 4
    1455,   // 5
    1545,   // 6
    1636,   // 7
    1727,   // 8
    1818,   // 9
    1909,   // 10
    2000    // 11
};


//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// COMMON CHANNEL SETTINGS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// Although linear stick and aux channels are sort of different constructs, it is useful to be able to access
// a few common parameters from both in a single array. Here we create a struct of pointers to these
// common elements, in the sketch we will create an array of this struct
typedef struct common_channel_settings {
    uint8_t *channelNum;                // Which number are we in the PPM input stream
    int16_t *pulse;                     // PPM pulse 
    boolean *updated;                   // Has the pulse changed since last check
    boolean *started;                   // Has a command just started
    boolean *present;                   // Did we detect this channel in the PPM stream? 
};



//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// TURRET STICK SPECIAL POSITIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
const byte SPECIALPOSITIONS          = 9;   // There are 9 special positions on the left stick (TL, TC, TR, ML, MC, MR, BL, BC, BR)
const byte TURRETSTICK_PULSESUBTRACT = 35;  // PWM pulses - how close to the very end of stick movement, do we consider a pulse a special position. 

#define MAX_SPEC_POS 36                 // The highest number of any special position
enum turretStick_Positions : byte {
    TL = 36,                            // Top left
    TC = 34,                            // Top center
    TR = 33,                            // Top right
    ML = 20,                            // Middle left
    MC = 18,                            // Middle center
    MR = 17,                            // Middle right
    BL = 12,                            // Bottom left
    BC = 10,                            // Bottom center
    BR = 9                              // Bottom right
};

enum border_vals : byte {
    nullborder  = 128,                  // 10000000     not decided
    Stick   = 64,                       // 01000000     within regular stick limits
    // By adding any two of these we can determine the special position of the stick:
    Top     = 32,                       // 00100000     at the top
    Middle  = 16,                       // 00010000     in the middle (between top and bottom)
    Bottom  = 8,                        // 00001000     at the bottom
    Left    = 4,                        // 00000100     at the left
    Center  = 2,                        // 00000010     in the center (between left and right)
    Right   = 1                         // 00000001     at the right
};

// Top left      = 36    (32+4)
// Top center    = 34    (32+2)
// Top right     = 33    (32+1)
// Middle left   = 20    (16+4)
// Middle center = 18    (16+2)    - this is stick untouched (in center)
// Middle right  = 17    (16+1)
// Bottom left   = 12    (8+4)
// Bottom center = 10    (8+2)
// Bottom right  = 9     (8+1)

// We create a virtual special function "channel" that represents the position and status of the
// turret stick as it relates to any special positions. 
typedef struct sf_channel 
{
    int Position;
    boolean updated;
};


//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// RADIO STREAM DEFINES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// These are shared by the PPM and SBus decoders

#define MIN_POSSIBLE_PULSE      750                     // Reasonable pulsewidths           
#define DEFAULT_PULSE_CENTER    1500
#define MAX_POSSIBLE_PULSE      2250                        


// Various states the decoders can have
typedef enum {
    NULL_state=-1, 
    NOT_SYNCHED_state, 
    ACQUIRING_state, 
    READY_state,
    FAILSAFE_state
 } decodeState_t;



#endif
