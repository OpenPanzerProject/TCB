/* OP_FunctionsTriggers.h   Open Panzer functions & triggers - defines and structs related to special functions and their triggers
 * Source:                  openpanzer.org              
 * Authors:                 Luke Middleton
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
 
#ifndef OP_FUNCTIONSTRIGGERS_H
#define OP_FUNCTIONSTRIGGERS_H
#include "OP_Settings.h"


//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// SPECIAL FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>

#define trigger_id_multiplier_auxchannel    1000        // Aux channel trigger ID is defined as:
                                                        // (multiplier * channel number) + (number of switch positions * switch_pos_multiplier) + switch position (1-5)
#define trigger_id_multiplier_ports         100         // External ports input trigger ID is defined as:
                                                        // (multipler * port number) + 0/1 (off/on)
#define switch_pos_multiplier               10          // Move the number of switch positions to the tens slot (the actual position will be in the ones slot)      

#define ANALOG_SPECFUNCTION_MAX_VAL         1023
#define ANALOG_SPECFUNCTION_CENTER_VAL      511
#define ANALOG_SPECFUNCTION_MIN_VAL         0


const byte COUNT_SPECFUNCTIONS  = 98;   // Count of special functions. 
// We don't want Arduino turning these into ints, so use " : byte" to keep the enum to bytes (chars)
// This also means we can't have more than 256 special functions
enum _special_function : byte {
    SF_NULL_FUNCTION    = 0,        // 0    -- no function assigned
    SF_ENGINE_TOGGLE    = 1,        // 1
    SF_ENGINE_ON        = 2,        // 2
    SF_ENGINE_OFF       = 3,        // 3
    SF_TRANS_TOGGLE     = 4,        // 4
    SF_TRANS_ON         = 5,        // 5
    SF_TRANS_OFF        = 6,        // 6
    SF_CANNON_FIRE      = 7,        // 7
    SF_MECH_BARREL      = 8,        // 8
    SF_RECOIL_SERVO     = 9,        // 9
    SF_HI_FLASH         = 10,       // 10
    SF_MG_FIRE          = 11,       // 11
    SF_MG_OFF           = 12,       // 12
    SF_BARREL_ENABLE    = 13,       // 13
    SF_BARREL_DISABLE   = 14,       // 14
    SF_BARREL_TOGGLE    = 15,       // 15
    SF_LIGHT1_TOGGLE    = 16,       // 16
    SF_LIGHT1_ON        = 17,       // 17
    SF_LIGHT1_OFF       = 18,       // 18
    SF_LIGHT2_TOGGLE    = 19,       // 19
    SF_LIGHT2_ON        = 20,       // 20
    SF_LIGHT2_OFF       = 21,       // 21   
    SF_RUNNINGLIGHTS_TOGGLE = 22,   // 22
    SF_RUNNINGLIGHTS_ON = 23,       // 23
    SF_RUNNINGLIGHTS_OFF= 24,       // 24
    SF_AUXOUT_TOGGLE    = 25,       // 25
    SF_AUXOUT_ON        = 26,       // 26
    SF_AUXOUT_OFF       = 27,       // 27
    SF_AUXOUT_LEVEL     = 28,       // 28   -- analog function
    SF_AUXOUT_PRESETDIM = 29,       // 29
    SF_AUXOUT_FLASH     = 30,       // 30
    SF_AUXOUT_BLINK     = 31,       // 31
    SF_AUXOUT_TOGGLEBLINK = 32,     // 32
    SF_AUXOUT_REVOLVE   = 33,       // 33
    SF_AUXOUT_TOGGLEREVOLVE = 34,   // 34
    SF_USER_SOUND1_ONCE = 35,       // 35   -- see also 86-91 for user sounds 3 & 4
    SF_USER_SOUND1_RPT  = 36,       // 36
    SF_USER_SOUND1_OFF  = 37,       // 37
    SF_USER_SOUND2_ONCE = 38,       // 38
    SF_USER_SOUND2_RPT  = 39,       // 39
    SF_USER_SOUND2_OFF  = 40,       // 40
    SF_OUTPUT_A_TOGGLE  = 41,       // 41
    SF_OUTPUT_A_ON      = 42,       // 42
    SF_OUTPUT_A_OFF     = 43,       // 43
    SF_OUTPUT_B_TOGGLE  = 44,       // 44
    SF_OUTPUT_B_ON      = 45,       // 45
    SF_OUTPUT_B_OFF     = 46,       // 46   
    SF_ACCEL_LEVEL      = 47,       // 47   -- analog function
    SF_DECEL_LEVEL      = 48,       // 48   -- analog function
    SF_TURNMODE_1       = 49,       // 49
    SF_TURNMODE_2       = 50,       // 50
    SF_TURNMODE_3       = 51,       // 51
    SF_SMOKER           = 52,       // 52   -- analog function
    SF_MOTOR_A          = 53,       // 53   -- analog function
    SF_MOTOR_B          = 54,       // 54   -- analog function
    SF_RC1_PASS         = 55,       // 55   -- analog function ("PASS" for pass-through) -- see also 92-97 for pass-throughs 6-8
    SF_RC2_PASS         = 56,       // 56   -- analog function
    SF_RC3_PASS         = 57,       // 57   -- analog function
    SF_RC4_PASS         = 58,       // 58   -- analog function
    SF_RC1_PASS_PAN     = 59,       // 59   -- analog function (Pan servo passthrough signal)
    SF_RC2_PASS_PAN     = 60,       // 60   -- analog function
    SF_RC3_PASS_PAN     = 61,       // 61   -- analog function
    SF_RC4_PASS_PAN     = 62,       // 62   -- analog function    
    SF_BARREL_STAB_ON   = 63,       // 63 
    SF_BARREL_STAB_OFF  = 64,       // 64
    SF_BARREL_STAB_TOGGLE = 65,     // 65
    SF_BARREL_STAB_LEVEL= 66,       // 66   -- analog function
    SF_HILLS_ON         = 67,       // 67 
    SF_HILLS_OFF        = 68,       // 68
    SF_HILLS_TOGGLE     = 69,       // 69
    SF_HILLS_LEVEL      = 70,       // 70   -- analog function
    SF_USER_FUNC_1      = 71,       // 71
    SF_USER_FUNC_2      = 72,       // 72
    SF_USER_ANLG_1      = 73,       // 73   -- analog function
    SF_USER_ANLG_2      = 74,       // 74   -- analog function
    SF_DUMP_DEBUG       = 75,       // 75
    SF_NT_ENABLE        = 76,       // 76
    SF_NT_DISABLE       = 77,       // 77
    SF_NT_TOGGLE        = 78,       // 78
    SF_DRIVEPROFILE_1   = 79,       // 79
    SF_DRIVEPROFILE_2   = 80,       // 80
    SF_DRIVEPROFILE_TOGGLE = 81,    // 81
    SF_SMOKER_ENABLE    = 82,       // 82
    SF_SMOKER_DISABLE   = 83,       // 83
    SF_SMOKER_TOGGLE    = 84,       // 84
    SF_SET_VOLUME       = 85,       // 85
    SF_USER_SOUND3_ONCE = 86,       // 86   -- see also 35-40 for user sounds 1 & 2
    SF_USER_SOUND3_RPT  = 87,       // 87
    SF_USER_SOUND3_OFF  = 88,       // 88
    SF_USER_SOUND4_ONCE = 89,       // 89
    SF_USER_SOUND4_RPT  = 90,       // 90
    SF_USER_SOUND4_OFF  = 91,       // 91
    SF_RC6_PASS         = 92,       // 92   -- analog function ("PASS" for pass-through) -- see also 55-62 for pass-throughs 1-4
    SF_RC7_PASS         = 93,       // 93   -- analog function
    SF_RC8_PASS         = 94,       // 94   -- analog function
    SF_RC6_PASS_PAN     = 95,       // 95   -- analog function (Pan servo passthrough signal)
    SF_RC7_PASS_PAN     = 96,       // 96   -- analog function
    SF_RC8_PASS_PAN     = 97        // 97   -- analog function    
};

// This is really kludgy, and it makes no difference to the running of the program, but we do use it
// to help print the function triggers out the serial port. For each function trigger above, we create
// an array with 1 in the slot if the function is digital, and 0 in the slot if the function is analog. 
const boolean DigitalFunctionsTable[COUNT_SPECFUNCTIONS] PROGMEM_FAR =
{
 1,1,1,1,1,1,1,1,1,1,   // 0-9
 1,1,1,1,1,1,1,1,1,1,   // 10-19
 1,1,1,1,1,1,1,1,0,1,   // 20-29    28 analog
 1,1,1,1,1,1,1,1,1,1,   // 30-39    
 1,1,1,1,1,1,1,0,0,1,   // 40-49    47,48 analog
 1,1,0,0,0,0,0,0,0,0,   // 50-59    52-59 analog
 0,0,0,1,1,1,0,1,1,1,   // 60-69    60-62,66 analog
 0,1,1,0,0,1,0,0,0,1,   // 70-79    70,73,74 analog
 1,1,1,1,1,0,1,1,1,1,   // 80-89    85 analog
 1,1,0,0,0,0,0,0        // 90-97    92-97 analog    
 };
// This macro lets us pass a _special_function number and it will return 1 if the function is a digital function, 0 if analog
#define isSpecialFunctionDigital(f) pgm_read_byte_far(pgm_get_far_address(DigitalFunctionsTable) + (uint32_t)f);

// The FUNCNAME_CHARS set to 41 means each string must be 40 chars or less (the compiler will "helpfully" null-terminate each string, which adds one extra byte). 
// The approach shown here is a very wasteful way of storing strings since many of our strings are much less than 40 characters long, but it is the only way I have found
// to store them in FAR program memory and still be able to access them using a reasonable array index, WHILE not needing to make custom modifications to the 
// linker or makefile scripts that would render this project difficult for the lay user to compile with the stock Arduino IDE. 
// Thankfully the one single thing we still have plenty of is program space. Even if we reach the maximum of 256 special functions, at 41 chars each
// these names would take up < 11 kilobytes (11,000 bytes). Taking into account our bootloader and the code we have as of spring 2016, even 
// with all 256 functions defined we would still be below 100 kb program memory, out of about 258k we have to work with. 
#define FUNCNAME_CHARS  41
const char _FunctionNames_[COUNT_SPECFUNCTIONS][FUNCNAME_CHARS] PROGMEM_FAR = 
{   "NULL FUNCTION",                             // 0
    "Engine - Toggle On/Off",                    // 1
    "Engine - Turn On",                          // 2
    "Engine - Turn Off",                         // 3
    "Transmission - Toggle On/Off",              // 4
    "Transmission - Turn On",                    // 5
    "Transmission - Turn Off",                   // 6
    "Cannon Fire",                               // 7
    "Fire Airsoft / Mechanical Recoil Unit",     // 8
    "Recoil Servo",                              // 9
    "High Intensity Flash",                      // 10 
    "Machine Gun - Fire",                        // 11
    "Machine Gun - Stop",                        // 12
    "Airsoft/Mech Recoil - Enable",              // 13
    "Airsoft/Mech Recoil - Disable",             // 14
    "Airsoft/Mech Recoil - Toggle",              // 15
    "Light 1 (Headlights) - Toggle On/Off",      // 16
    "Light 1 (Headlights) - Turn On",            // 17
    "Light 1 (Headlights) - Turn Off",           // 18
    "Light 2 - Toggle On/Off",                   // 19
    "Light 2 - Turn On",                         // 20
    "Light 2 - Turn Off",                        // 21
    "Running Lights - Toggle On/Off",            // 22
    "Running Lights - Turn On",                  // 23
    "Running Lights - Turn Off",                 // 24
    "Aux Output - Toggle On/Off",                // 25
    "Aux Output - Turn On",                      // 26
    "Aux Output - Turn Off",                     // 27
    "Aux Output - Set Level",                    // 28
    "Aux Output - Preset Dim Level",             // 29
    "Aux Output - Flash",                        // 30
    "Aux Output - Blink",                        // 31
    "Aux Output - Toggle Blink",                 // 32
    "Aux Output - Revolving Light",              // 33 
    "Aux Output - Toggle Revolving Light",       // 34
    "User Sound 1 - Play Once",                  // 35   -- see also 86-91 for user sounds 3 & 4
    "User Sound 1 - Repeat",                     // 36
    "User Sound 1 - Stop",                       // 37
    "User Sound 2 - Play Once",                  // 38
    "User Sound 2 - Repeat",                     // 39
    "User Sound 2- Stop",                        // 40
    "External Output A - Toggle",                // 41
    "External Output A - Turn On",               // 42
    "External Output A - Turn Off",              // 43             
    "External Output B - Toggle",                // 44
    "External Output B - Turn On",               // 45
    "External Output B - Turn Off",              // 46
    "Set Acceleration Level",                    // 47
    "Set Deceleration Level",                    // 48
    "Set Turn Mode = 1",                         // 49
    "Set Turn Mode = 2",                         // 50
    "Set Turn Mode = 3",                         // 51
    "Smoker - Manual Control",                   // 52
    "Motor A - Manual Control",                  // 53
    "Motor B - Manual Control",                  // 54
    "RC Output 1 - Pass-through",                // 55   -- see also 92-97 for pass-throughs 6-8
    "RC Output 2 - Pass-through",                // 56
    "RC Output 3 - Pass-through",                // 57
    "RC Output 4 - Pass-through",                // 58
    "RC Output 1 - Pan Servo",                   // 59
    "RC Output 2 - Pan Servo",                   // 60
    "RC Output 3 - Pan Servo",                   // 61
    "RC Output 4 - Pan Servo",                   // 62
    "Barrel Stabilization - Turn On",            // 63
    "Barrel Stabilization - Turn Off",           // 64
    "Barrel Stabilization - Toggle",             // 65
    "Barrel Stabilization - Set Sensitivity",    // 66
    "Hill Physics - Turn On",                    // 67
    "Hill Physics - Turn Off",                   // 68
    "Hill Physics - Toggle",                     // 69
    "Hill Physics - Set Sensitivity",            // 70
    "User Function 1",                           // 71
    "User Function 2",                           // 72
    "Analog User Function 1",                    // 73
    "Analog User Function 2",                    // 74
    "Dump Settings",                             // 75
    "Neutral Turn - Enable",                     // 76
    "Neutral Turn - Disable",                    // 77
    "Neutral Turn - Toggle",                     // 78
    "Drive Profile - Set to 1",                  // 79
    "Drive Profile - Set to 2",                  // 80
    "Drive Profile - Toggle",                    // 81           
    "Smoker - Enable",                           // 82
    "Smoker - Disable",                          // 83
    "Smoker - Toggle",                           // 84
    "Sound Card - Set Volume",                   // 85
    "User Sound 3 - Play Once",                  // 86   -- see also 35-40 for user sounds 1 & 2
    "User Sound 3 - Repeat",                     // 87
    "User Sound 3 - Stop",                       // 88
    "User Sound 4 - Play Once",                  // 89
    "User Sound 4 - Repeat",                     // 90
    "User Sound 4 - Stop",                       // 91
    "RC Output 6 - Pass-through",                // 92   -- see also 55-62 for pass-throughs 1-4
    "RC Output 7 - Pass-through",                // 93
    "RC Output 8 - Pass-through",                // 94
    "RC Output 6 - Pan Servo",                   // 95   
    "RC Output 7 - Pan Servo",                   // 96
    "RC Output 8 - Pan Servo"                    // 97   
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// FUNCTION TRIGGERS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
#define MAX_FUNCTION_TRIGGERS 40            // Maximum number of triggers we can save
// An array of MAX_FUNCTION_TRIGGERS of this struct will be saved in EEPROM: 
typedef struct _functionTrigger {
    uint16_t TriggerID;                     // Each trigger has an ID
    _special_function specialFunction;      // The trigger will be linked to a special function in the sketch
};

// Trigger sources:
enum _trigger_source : byte {
    TS_NULL_TRIGGER = 0,   // no trigger
    TS_TURRET_STICK = 1,   // Turret stick
    TS_AUX1,               // Aux channel 1
    TS_AUX2,               // Aux channel 2
    TS_AUX3,               // Aux channel 3
    TS_AUX4,               // Aux channel 4
    TS_AUX5,               // Aux channel 5
    TS_AUX6,               // Aux channel 6
    TS_AUX7,               // Aux channel 7
    TS_AUX8,               // Aux channel 8
    TS_AUX9,               // Aux channel 9
    TS_AUX10,              // Aux channel 10
    TS_AUX11,              // Aux channel 11
    TS_AUX12,              // Aux channel 12
    TS_INPUT_A,            // External input A (if set to input)
    TS_INPUT_B             // External input B (if set to input)
};

// Triggers for special functions can be set by the user. The TriggerID lets the program know what event should
// trigger the special function. Presently TriggerIDs can be turret stick special positions, aux channel inputs, or digital I/O ports (when set to input). 

// In the case of turret stick special positions, the possible TriggerID values are: 
    // Top left      = 36    (32+4)
    // Top center    = 34    (32+2)
    // Top right     = 33    (32+1)
    // Middle left   = 20    (16+4)
    // Middle center = 18    (16+2)    - this is stick untouched
    // Middle right  = 17    (16+1)
    // Bottom left   = 12    (8+4)
    // Bottom center = 10    (8+2)
    // Bottom right  = 9     (8+1)

// Trigger IDs can also be digital aux channel inputs. The TriggerID in that case will be: 
//   (AuxChannelNumber * trigger_id_multiplier_auxchannel) + (NumberOfSwitchPositions * switch_pos_multiplier) + Switch Position    
//   So for example, if you wanted to trigger a function when the Aux Channel 3 switch is in position 2 of a 2-position switch, 
//   the trigger ID will be: 3025 <- Yes, that is right! 3000 for the 3rd Aux Channel, 20 for a two-position switch, 5 for position 2
//   Does that make sense? Remember that positions are always 1-5. If you have a two-position switch, position 1 = 1 and position 2 = 5.
//   If you have a three position switch, position 1 = 1, position 2 = 3, position 3 = 5. 
//   If you have a five-position switch, each position equals its own number (1-5)

// Trigger IDs can also be digital I/O ports (when set to input). The TriggerID in that case will be: 
//   (PortNumber * trigger_id_multiplier_ports) + Input Switch Position
//   These are always considered 2 position switches so we don't need to include information about the number of positions
//   For example, the trigger ID for Port B (A = 1, B = 2) in the On position would be: 201
//   The trigger ID for Port A in the Off position would be 100. 

// Trigger IDs can also be *analog*, in which case they won't have any switch position information and the analog value will be taken
// from somewhwere besides the Trigger ID itself. 


// At the top of the sketch we will also create an array of MAX_FUNCTION_TRIGGERS function pointers. 
// Whenever a _functionTrigger.TriggerID matches some external trigger condition, the function
// in that array matching the same position as the TriggerID in the _functionTrigger array will get called. 
// Yes, we could put this function pointer in the _functionTrigger struct directly, but we want to save
// an array of the _functionTrigger struct in EEPROM. This would take up more room and isn't needed in EEPROM.
// It might be hard to understand how these work until you look at the main sketch. The function pointer is
// to a function that takes an unsigned 2 byte integer and returns void. Most special functions don't need any arguments
// but some do (the analog functions), so even for those that don't need it, those functions will need an 
// uint16_t parameter that will simply be ignored. 
// In the case of analog functions, even though we can pass any number up to ~65,000, all analog functions will be 
// written so as to expect some value between 0 and 1023 (10-bit number). What it does with that number is up to you, 
// but values passed must be scaled to that range and the function must be expecting that range. 
typedef void(*void_FunctionPointer_uint16)(uint16_t);




#endif
