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


const byte COUNT_SPECFUNCTIONS  = 76;   // Count of special functions. 
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
    SF_USER_SOUND1_ONCE = 35,       // 35
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
    SF_RC1_PASS         = 55,       // 55   -- analog function ("PASS" for pass-through)
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
    SF_DUMP_DEBUG       = 75        // 75
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
 0,1,1,0,0,1            // 70-75    70,73,74 analog
 };
#define isSpecialFunctionDigital(f) pgm_read_byte_far(&DigitalFunctionsTable[f])

// Function names stored in Flash memory
const char Func000[] PROGMEM_FAR = "NULL FUNCTION";                             // 0
const char Func001[] PROGMEM_FAR = "Engine - Toggle On/Off";                    // 1
const char Func002[] PROGMEM_FAR = "Engine - Turn On";                          // 2
const char Func003[] PROGMEM_FAR = "Engine - Turn Off";                         // 3
const char Func004[] PROGMEM_FAR = "Transmission - Toggle On/Off";              // 4
const char Func005[] PROGMEM_FAR = "Transmission - Turn On";                    // 5
const char Func006[] PROGMEM_FAR = "Transmission - Turn Off";                   // 6
const char Func007[] PROGMEM_FAR = "Cannon Fire";                               // 7
const char Func008[] PROGMEM_FAR = "Fire Airsoft / Mechanical Recoil Unit";     // 8
const char Func009[] PROGMEM_FAR = "Recoil Servo";                              // 9
const char Func010[] PROGMEM_FAR = "High Intensity Flash";                      // 10 
const char Func011[] PROGMEM_FAR = "Machine Gun - Fire";                        // 11
const char Func012[] PROGMEM_FAR = "Machine Gun - Stop";                        // 12
const char Func013[] PROGMEM_FAR = "Airsoft/Mech Recoil - Enable";              // 13
const char Func014[] PROGMEM_FAR = "Airsoft/Mech Recoil - Disable";             // 14
const char Func015[] PROGMEM_FAR = "Airsoft/Mech Recoil - Toggle";              // 15
const char Func016[] PROGMEM_FAR = "Light 1 (Headlights) - Toggle On/Off";      // 16
const char Func017[] PROGMEM_FAR = "Light 1 (Headlights) - Turn On";            // 17
const char Func018[] PROGMEM_FAR = "Light 1 (Headlights) - Turn Off";           // 18
const char Func019[] PROGMEM_FAR = "Light 2 - Toggle On/Off";                   // 19
const char Func020[] PROGMEM_FAR = "Light 2 - Turn On";                         // 20
const char Func021[] PROGMEM_FAR = "Light 2 - Turn Off";                        // 21
const char Func022[] PROGMEM_FAR = "Running Lights - Toggle On/Off";            // 22
const char Func023[] PROGMEM_FAR = "Running Lights - Turn On";                  // 23
const char Func024[] PROGMEM_FAR = "Running Lights - Turn Off";                 // 24
const char Func025[] PROGMEM_FAR = "Aux Output - Toggle On/Off";                // 25
const char Func026[] PROGMEM_FAR = "Aux Output - Turn On";                      // 26
const char Func027[] PROGMEM_FAR = "Aux Output - Turn Off";                     // 27
const char Func028[] PROGMEM_FAR = "Aux Output - Set Level";                    // 28
const char Func029[] PROGMEM_FAR = "Aux Output - Preset Dim Level";             // 29
const char Func030[] PROGMEM_FAR = "Aux Output - Flash";                        // 30
const char Func031[] PROGMEM_FAR = "Aux Output - Blink";                        // 31
const char Func032[] PROGMEM_FAR = "Aux Output - Toggle Blink";                 // 32
const char Func033[] PROGMEM_FAR = "Aux Output - Revolving Light";              // 33 
const char Func034[] PROGMEM_FAR = "Aux Output - Toggle Revolving Light";       // 34
const char Func035[] PROGMEM_FAR = "User Sound 1 - Play Once";                  // 35
const char Func036[] PROGMEM_FAR = "User Sound 1 - Repeat";                     // 36
const char Func037[] PROGMEM_FAR = "User Sound 1 - Stop";                       // 37
const char Func038[] PROGMEM_FAR = "User Sound 2 - Play Once";                  // 38
const char Func039[] PROGMEM_FAR = "User Sound 2 - Repeat";                     // 39
const char Func040[] PROGMEM_FAR = "User Sound 2- Stop";                        // 40
const char Func041[] PROGMEM_FAR = "External Output A - Toggle";                // 41
const char Func042[] PROGMEM_FAR = "External Output A - Turn On";               // 42
const char Func043[] PROGMEM_FAR = "External Output A - Turn Off";              // 43             
const char Func044[] PROGMEM_FAR = "External Output B - Toggle";                // 44
const char Func045[] PROGMEM_FAR = "External Output B - Turn On";               // 45
const char Func046[] PROGMEM_FAR = "External Output B - Turn Off";              // 46
const char Func047[] PROGMEM_FAR = "Set Acceleration Level";                    // 47
const char Func048[] PROGMEM_FAR = "Set Deceleration Level";                    // 48
const char Func049[] PROGMEM_FAR = "Set Turn Mode = 1";                         // 49
const char Func050[] PROGMEM_FAR = "Set Turn Mode = 2";                         // 50
const char Func051[] PROGMEM_FAR = "Set Turn Mode = 3";                         // 51
const char Func052[] PROGMEM_FAR = "Smoker - Manual Control";                   // 52
const char Func053[] PROGMEM_FAR = "Motor A - Manual Control";                  // 53
const char Func054[] PROGMEM_FAR = "Motor B - Manual Control";                  // 54
const char Func055[] PROGMEM_FAR = "RC Output 1 - Pass-through";                // 55
const char Func056[] PROGMEM_FAR = "RC Output 2 - Pass-through";                // 56
const char Func057[] PROGMEM_FAR = "RC Output 3 - Pass-through";                // 57
const char Func058[] PROGMEM_FAR = "RC Output 4 - Pass-through";                // 58
const char Func059[] PROGMEM_FAR = "RC Output 1 - Pan Servo";                   // 59
const char Func060[] PROGMEM_FAR = "RC Output 2 - Pan Servo";                   // 60
const char Func061[] PROGMEM_FAR = "RC Output 3 - Pan Servo";                   // 61
const char Func062[] PROGMEM_FAR = "RC Output 4 - Pan Servo";                   // 62
const char Func063[] PROGMEM_FAR = "Barrel Stabilization - Turn On";            // 63
const char Func064[] PROGMEM_FAR = "Barrel Stabilization - Turn Off";           // 64
const char Func065[] PROGMEM_FAR = "Barrel Stabilization - Toggle";             // 65
const char Func066[] PROGMEM_FAR = "Barrel Stabilization - Set Sensitivity";    // 66
const char Func067[] PROGMEM_FAR = "Hill Physics - Turn On";                    // 67
const char Func068[] PROGMEM_FAR = "Hill Physics - Turn Off";                   // 68
const char Func069[] PROGMEM_FAR = "Hill Physics - Toggle";                     // 69
const char Func070[] PROGMEM_FAR = "Hill Physics - Set Sensitivity";            // 70
const char Func071[] PROGMEM_FAR = "User Function 1";                           // 71
const char Func072[] PROGMEM_FAR = "User Function 2";                           // 72
const char Func073[] PROGMEM_FAR = "Analog User Function 1";                    // 73
const char Func074[] PROGMEM_FAR = "Analog User Function 2";                    // 74
const char Func075[] PROGMEM_FAR = "Dump Settings";                             // 75

//const PROGMEM boolean DigitalFunctionsTable[COUNT_SPECFUNCTIONS] =
//const uint32_t * const function_name_table[COUNT_SPECFUNCTIONS] PROGMEM_FAR = 
/*
const char * const function_name_table[COUNT_SPECFUNCTIONS] PROGMEM_FAR = 
{
 Func000, Func001, Func002, Func003, Func004, Func005, Func006, Func007, Func008, Func009,
 Func010, Func011, Func012, Func013, Func014, Func015, Func016, Func017, Func018, Func019,
 Func020, Func021, Func022, Func023, Func024, Func025, Func026, Func027, Func028, Func029,
 Func030, Func031, Func032, Func033, Func034, Func035, Func036, Func037, Func038, Func039,
 Func040, Func041, Func042, Func043, Func044, Func045, Func046, Func047, Func048, Func049,
 Func050, Func051, Func052, Func053, Func054, Func055, Func056, Func057, Func058, Func059,
 Func060, Func061, Func062, Func063, Func064, Func065, Func066, Func067, Func068, Func069,
 Func070, Func071, Func072, Func073, Func074, Func075
};
*/
//                                                     , Func076, Func077, Func078, Func079,
// Func080, Func081, Func082, Func083, Func084, Func085, Func086, Func087, Func088, Func089,
// Func090, Func091, Func092, Func093, Func094, Func095, Func096, Func097, Func098, Func099,
// Func100, Func101, Func102, Func103, Func104, Func105, Func106, Func107, Func108, Func109,


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
