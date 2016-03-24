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


const byte COUNT_SPECFUNCTIONS  = 68;   // Count of special functions. 
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
    SF_LIGHT1_TOGGLE    = 13,       // 13
    SF_LIGHT1_ON        = 14,       // 14
    SF_LIGHT1_OFF       = 15,       // 15
    SF_LIGHT2_TOGGLE    = 16,       // 16
    SF_LIGHT2_ON        = 17,       // 17
    SF_LIGHT2_OFF       = 18,       // 18   
    SF_RUNNINGLIGHTS_TOGGLE = 19,   // 19
    SF_RUNNINGLIGHTS_ON = 20,       // 20
    SF_RUNNINGLIGHTS_OFF= 21,       // 21
    SF_AUXOUT_TOGGLE    = 22,       // 22
    SF_AUXOUT_ON        = 23,       // 23
    SF_AUXOUT_OFF       = 24,       // 24
    SF_AUXOUT_LEVEL     = 25,       // 25   -- analog function
    SF_AUXOUT_PRESETDIM = 26,       // 26
    SF_AUXOUT_FLASH     = 27,       // 27
    SF_AUXOUT_BLINK     = 28,       // 28
    SF_AUXOUT_TOGGLEBLINK = 29,     // 29
    SF_AUXOUT_REVOLVE   = 30,       // 30
    SF_AUXOUT_TOGGLEREVOLVE = 31,   // 31
    SF_USER_SOUND1_ONCE = 32,       // 32
    SF_USER_SOUND1_RPT  = 33,       // 33
    SF_USER_SOUND1_OFF  = 34,       // 34
    SF_USER_SOUND2_ONCE = 35,       // 35
    SF_USER_SOUND2_RPT  = 36,       // 36
    SF_USER_SOUND2_OFF  = 37,       // 37
    SF_OUTPUT_A_TOGGLE  = 38,       // 38
    SF_OUTPUT_A_ON      = 39,       // 39
    SF_OUTPUT_A_OFF     = 40,       // 40
    SF_OUTPUT_B_TOGGLE  = 41,       // 41
    SF_OUTPUT_B_ON      = 42,       // 42
    SF_OUTPUT_B_OFF     = 43,       // 43   
    SF_ACCEL_LEVEL      = 44,       // 44   -- analog function
    SF_DECEL_LEVEL      = 45,       // 45   -- analog function
    SF_TURNMODE_1       = 46,       // 46
    SF_TURNMODE_2       = 47,       // 47
    SF_TURNMODE_3       = 48,       // 48
    SF_SMOKER           = 49,       // 49   -- analog function
    SF_MOTOR_A          = 50,       // 50   -- analog function
    SF_MOTOR_B          = 51,       // 51   -- analog function
    SF_SERVO1_PASS      = 52,       // 52   -- analog function ("PASS" for pass-through)
    SF_SERVO2_PASS      = 53,       // 53   -- analog function
    SF_SERVO3_PASS      = 54,       // 54   -- analog function
    SF_SERVO4_PASS      = 55,       // 55   -- analog function
    SF_BARREL_ON        = 56,       // 56 
    SF_BARREL_OFF       = 57,       // 57
    SF_BARREL_TOGGLE    = 58,       // 58
    SF_BARREL_LEVEL     = 59,       // 59   -- analog function
    SF_HILLS_ON         = 60,       // 60 
    SF_HILLS_OFF        = 61,       // 61
    SF_HILLS_TOGGLE     = 62,       // 62
    SF_HILLS_LEVEL      = 63,       // 63   -- analog function
    SF_USER_FUNC_1      = 64,       // 64
    SF_USER_FUNC_2      = 65,       // 65
    SF_USER_ANLG_1      = 66,       // 66   -- analog function
    SF_USER_ANLG_2      = 67        // 67   -- analog function
};

// This is really kludgy, and it makes no difference to the running of the program, but we do use it
// to help print the function triggers out the serial port. For each function trigger above, we create
// an array with 1 in the slot if the function is digital, and 0 in the slot if the function is analog. 
const PROGMEM boolean DigitalFunctionsTable[COUNT_SPECFUNCTIONS] =
{
 1,1,1,1,1,1,1,1,1,1,   // 0-9
 1,1,1,1,1,1,1,1,1,1,   // 10-19
 1,1,1,1,1,0,1,1,1,1,   // 20-29    25 analog
 1,1,1,1,1,1,1,1,1,1,   // 30-39    
 1,1,1,1,0,0,1,1,1,0,   // 40-49    44,45,49 analog
 0,0,0,0,0,0,1,1,1,0,   // 50-59    50,51,52,53,54,55,59 analog
 1,1,1,0,1,1,0,0        // 60-67    63,66,67 analog
 };
#define isSpecialFunctionDigital(f) pgm_read_byte_far(&DigitalFunctionsTable[f])


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
