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
#include "../OP_Settings/OP_Settings.h"


//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// SPECIAL FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// Special functions are actions the user may want to control. Functions are initiated by triggers. You can assign multiple functions to a single trigger, or 
// multiple triggers to a single function. There are two basic types of functions, which we call "digital" and "analog." These terms are not meant to be taken in a
// strict technical sense. Digital functions are discrete actions like turn a light on or off or fire the cannon or play a sound. Analog functions typically adjust
// something along a scale, examples are "set the speed of motor output A" or "set the volume". Digital functions can be assigned to "digital" (discrete) triggers, 
// typically switches on your transmitter. Analog functions need to be assigned to analog triggers such as a knob or lever on your transmitter (or even a potentiometer
// attached to one of the general purpose inputs). 

#define ANALOG_SPECFUNCTION_MAX_VAL         1023    // Analog functions all take the same range of values from 0-1023. If an analog input (trigger) has a different 
#define ANALOG_SPECFUNCTION_CENTER_VAL      511     // scale, it will need to be mapped to this range before it can control an analog function. 
#define ANALOG_SPECFUNCTION_MIN_VAL         0

const byte COUNT_SPECFUNCTIONS  = 166;   // Count of special functions. 

// Each function has a number and an enum name. 
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
    SF_AUXOUT_FLASH     = 30,       // 30   -- see also 111 for inverse flash
    SF_AUXOUT_BLINK     = 31,       // 31
    SF_AUXOUT_TOGGLEBLINK = 32,     // 32
    SF_AUXOUT_REVOLVE   = 33,       // 33
    SF_AUXOUT_TOGGLEREVOLVE = 34,   // 34
    SF_USER_SOUND1_ONCE = 35,       // 35   -- see also 86-91 for user sounds 3 & 4 and 103-108 for user sounds 5 & 6
    SF_USER_SOUND1_RPT  = 36,       // 36
    SF_USER_SOUND1_OFF  = 37,       // 37
    SF_USER_SOUND2_ONCE = 38,       // 38
    SF_USER_SOUND2_RPT  = 39,       // 39
    SF_USER_SOUND2_OFF  = 40,       // 40
    SF_OUTPUT_A_TOGGLE  = 41,       // 41   -- see also 109 for pulse, and 156 for blink
    SF_OUTPUT_A_ON      = 42,       // 42
    SF_OUTPUT_A_OFF     = 43,       // 43
    SF_OUTPUT_B_TOGGLE  = 44,       // 44   -- see also 110 for pulse, and 157 for blink
    SF_OUTPUT_B_ON      = 45,       // 45
    SF_OUTPUT_B_OFF     = 46,       // 46   
    SF_ACCEL_LEVEL      = 47,       // 47   -- analog function
    SF_DECEL_LEVEL      = 48,       // 48   -- analog function
    SF_TURNMODE_1       = 49,       // 49
    SF_TURNMODE_2       = 50,       // 50
    SF_TURNMODE_3       = 51,       // 51
    SF_SMOKER           = 52,       // 52   -- analog function, sets the speed of the smoker manually -- see also 101-102 & 161 for manual on/off/toggle digital functions. See 82-84 for control of auto smoker.
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
    SF_SMOKER_ENABLE    = 82,       // 82   -- for control of _auto_ smoker functionality
    SF_SMOKER_DISABLE   = 83,       // 83
    SF_SMOKER_TOGGLE    = 84,       // 84
    SF_SET_VOLUME       = 85,       // 85
    SF_USER_SOUND3_ONCE = 86,       // 86   -- see also 35-40 for user sounds 1 & 2 and 103-108 for user sounds 5 & 6
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
    SF_RC8_PASS_PAN     = 97,       // 97   -- analog function    
    SF_INCR_VOLUME      = 98,       // 98
    SF_DECR_VOLUME      = 99,       // 99
    SF_STOP_VOLUME      = 100,      // 100
    SF_SMOKER_ON        = 101,      // 101  -- functions for controlling smoker manually, see also 52 for analog manual control of smoker output, and 161 for toggle. See 82-84 for control of auto smoker.
    SF_SMOKER_OFF       = 102,      // 102
    SF_USER_SOUND5_ONCE = 103,      // 103  -- see also 35-40 for user sounds 1 & 2 and 86-91 for user sounds 3 & 4
    SF_USER_SOUND5_RPT  = 104,      // 104
    SF_USER_SOUND5_OFF  = 105,      // 105
    SF_USER_SOUND6_ONCE = 106,      // 106
    SF_USER_SOUND6_RPT  = 107,      // 107
    SF_USER_SOUND6_OFF  = 108,      // 108    
    SF_OUTPUT_A_PULSE   = 109,      // 109   -- see also 41-43 for other OUTPUT_A functions
    SF_OUTPUT_B_PULSE   = 110,      // 110   -- see also 44-46 for other OUTPUT_B functions
    SF_AUXOUT_INV_FLASH = 111,      // 111   -- see also 30 for regular flash
    SF_MG2_FIRE         = 112,      // 112   -- Second machine gun (L2 output)
    SF_MG2_OFF          = 113,      // 113   
    SF_OVERLAY_ENABLE   = 114,      // 114
    SF_OVERLAY_DISABLE  = 115,      // 115
    SF_MANTRANS_FWD     = 116,      // 116
    SF_MANTRANS_REV     = 117,      // 117
    SF_MANTRANS_NEUTRAL = 118,      // 118
    SF_AUXOUT_TOGGLEDIM = 119,      // 119
    SF_MOTOR_A_ON       = 120,      // 120
    SF_MOTOR_A_OFF      = 121,      // 121
    SF_MOTOR_A_TOGGLE   = 122,      // 122
    SF_MOTOR_B_ON       = 123,      // 123
    SF_MOTOR_B_OFF      = 124,      // 124
    SF_MOTOR_B_TOGGLE   = 125,      // 125
    SF_USER_SOUND7_ONCE = 126,      // 126  -- see also 35-40 for user sounds 1 & 2, 86-91 for user sounds 3 & 4, 103-108 for user sounds 5 & 6
    SF_USER_SOUND7_RPT  = 127,      // 127  -- PEOPLE WANT SHIT TONS OF SOUNDS I GUESS
    SF_USER_SOUND7_OFF  = 128,      // 128
    SF_USER_SOUND8_ONCE = 129,      // 129
    SF_USER_SOUND8_RPT  = 130,      // 130
    SF_USER_SOUND8_OFF  = 131,      // 131
    SF_USER_SOUND9_ONCE = 132,      // 132  
    SF_USER_SOUND9_RPT  = 133,      // 133
    SF_USER_SOUND9_OFF  = 134,      // 134
    SF_USER_SOUND10_ONCE= 135,      // 135
    SF_USER_SOUND10_RPT = 136,      // 136
    SF_USER_SOUND10_OFF = 137,      // 137        
    SF_USER_SOUND11_ONCE= 138,      // 138  
    SF_USER_SOUND11_RPT = 139,      // 139
    SF_USER_SOUND11_OFF = 140,      // 140
    SF_USER_SOUND12_ONCE= 141,      // 141
    SF_USER_SOUND12_RPT = 142,      // 142
    SF_USER_SOUND12_OFF = 143,      // 143   
    SF_SBA_PLAYSTOP     = 144,      // 144
    SF_SBA_NEXT         = 145,      // 145
    SF_SBA_PREVIOUS     = 146,      // 146
    SF_SBA_RANDOM       = 147,      // 147
    SF_SBB_PLAYSTOP     = 148,      // 148
    SF_SBB_NEXT         = 149,      // 149
    SF_SBB_PREVIOUS     = 150,      // 150
    SF_SBB_RANDOM       = 151,      // 151
    SF_SPEED_75         = 154,      // 152
    SF_SPEED_50         = 153,      // 153
    SF_SPEED_25         = 152,      // 154
    SF_SPEED_RESTORE    = 155,      // 155
    SF_OUTPUT_A_BLINK   = 156,      // 156
    SF_OUTPUT_B_BLINK   = 157,      // 157
    SF_IR_ENABLE        = 158,      // 158
    SF_IR_DISABLE       = 159,      // 159
    SF_IR_TOGGLE        = 160,      // 160
    SF_SMOKER_MANTOGGLE = 161,      // 161  -- see also 52 for analog manual control of smoker output and 101-102 for manual on/off. This toggle is for manual control. See 82-84 for auto controls.
    SF_USER_SOUND_ALL_OFF = 162,    // 162
    SF_SMOKE_PREHEAT_ON = 163,      // 163
    SF_SMOKE_PREHEAT_OFF= 164,      // 164
    SF_SMOKE_PREHEAT_TOGGLE = 165   // 165  
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
 1,1,0,0,0,0,0,0,1,1,   // 90-99    92-97 analog    
 1,1,1,1,1,1,1,1,1,1,   // 100-109  
 1,1,1,1,1,1,1,1,1,1,   // 110-119
 1,1,1,1,1,1,1,1,1,1,   // 120-129
 1,1,1,1,1,1,1,1,1,1,   // 130-139
 1,1,1,1,1,1,1,1,1,1,   // 140-149
 1,1,1,1,1,1,1,1,1,1,   // 150-159
 1,1,1,1,1,1            // 160-165
 };
// This macro lets us pass a _special_function number and it will return 1 if the function is a digital function, 0 if analog
#define isSpecialFunctionDigital(f) pgm_read_byte_far(pgm_get_far_address(DigitalFunctionsTable) + (uint32_t)f);

// Friendly names for each function, stored in PROGMEM. Used for printint out the serial port. 
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
    "Aux Output - Flash",                        // 30  -- see also 111 for inverse flash
    "Aux Output - Blink",                        // 31
    "Aux Output - Toggle Blink",                 // 32
    "Aux Output - Revolving Light",              // 33 
    "Aux Output - Toggle Revolving Light",       // 34
    "User Sound 1 - Play Once",                  // 35   -- see also 86-91 for user sounds 3 & 4 and 103-108 for user sounds 5 & 6
    "User Sound 1 - Repeat",                     // 36
    "User Sound 1 - Stop",                       // 37
    "User Sound 2 - Play Once",                  // 38
    "User Sound 2 - Repeat",                     // 39
    "User Sound 2- Stop",                        // 40
    "External Output A - Toggle",                // 41   -- see also 109 for pulse and 156 for blink
    "External Output A - Turn On",               // 42
    "External Output A - Turn Off",              // 43             
    "External Output B - Toggle",                // 44   -- see also 110 for pulse and 157 for blink
    "External Output B - Turn On",               // 45
    "External Output B - Turn Off",              // 46
    "Set Acceleration Level",                    // 47
    "Set Deceleration Level",                    // 48
    "Set Turn Mode = 1",                         // 49
    "Set Turn Mode = 2",                         // 50
    "Set Turn Mode = 3",                         // 51
    "Smoker - Manual Control",                   // 52   -- see also 101-102 for digital on/off manual control of the smoker output and 161 for toggle
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
    "User Sound 3 - Play Once",                  // 86   -- see also 35-40 for user sounds 1 & 2 and 103-108 for user sounds 5 & 6
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
    "RC Output 8 - Pan Servo",                   // 97   
    "Start Increasing Volume",                   // 98
    "Start Decreasing Volume",                   // 99
    "Stop Changing Volume",                      // 100
    "Smoker - Manual On",                        // 101  -- see also 52 for analag manual control of the smoker output speed and 161 for toggle
    "Smoker - Manual Off",                       // 102
    "User Sound 5 - Play Once",                  // 103  -- see also 35-40 for user sounds 1 & 2 and 86-91 for user sounds 3 & 4
    "User Sound 5 - Repeat",                     // 104
    "User Sound 5 - Stop",                       // 105
    "User Sound 6 - Play Once",                  // 106
    "User Sound 6 - Repeat",                     // 107
    "User Sound 6 - Stop",                       // 108    
    "External Output A - Pulse",                 // 109 -- see also 41-43 for other OUTPUT_A functions
    "External Output B - Pulse",                 // 110 -- see also 44-46 for other OUTPUT_B functions   
    "Aux Output - Inverse Flash",                // 111 -- see also 30 for regular flash
    "2nd Machine Gun - Fire",                    // 112
    "2nd Machine Gun - Stop",                    // 113    
    "Track Sounds - Enable",                     // 114
    "Track Sounds - Disable",                    // 115
    "Manual Transmission - Forward",             // 116
    "Manual Transmission - Reverse",             // 117
    "Manual Transmission - Neutral",             // 118
    "Aux Output - Toggle Dim Level",             // 119
    "Motor A - On",                              // 120
    "Motor A - Off",                             // 121
    "Motor A - Toggle On/Off",                   // 122
    "Motor B - On",                              // 123
    "Motor B - Off",                             // 124
    "Motor B - Toggle On/Off",                   // 125
    "User Sound 7 - Play Once",                  // 126
    "User Sound 7 - Repeat",                     // 127
    "User Sound 7 - Stop",                       // 128
    "User Sound 8 - Play Once",                  // 129
    "User Sound 8 - Repeat",                     // 130
    "User Sound 8 - Stop",                       // 131
    "User Sound 9 - Play Once",                  // 132
    "User Sound 9 - Repeat",                     // 133
    "User Sound 9 - Stop",                       // 134
    "User Sound 10 - Play Once",                 // 135
    "User Sound 10 - Repeat",                    // 136
    "User Sound 10 - Stop",                      // 137
    "User Sound 11 - Play Once",                 // 138
    "User Sound 11 - Repeat",                    // 139
    "User Sound 11 - Stop",                      // 140    
    "User Sound 12 - Play Once",                 // 141
    "User Sound 12 - Repeat",                    // 142
    "User Sound 12 - Stop",                      // 143   
    "Sound Bank A - Play/Stop",                  // 144
    "Sound Bank A - Next",                       // 145
    "Sound Bank A - Previous",                   // 146
    "Sound Bank A - Random",                     // 147    
    "Sound Bank B - Play/Stop",                  // 148
    "Sound Bank B - Next",                       // 149
    "Sound Bank B - Previous",                   // 150
    "Sound Bank B - Random",                     // 151
    "Reduce Speed to 75%",                       // 152
    "Reduce Speed to 50%",                       // 153
    "Reduce Speed to 25%",                       // 154
    "Restore Speed",                             // 155      
    "External Output A - Blink",                 // 156
    "External Output B - Blink",                 // 157
    "IR - Enable",                               // 158
    "IR - Disable",                              // 159
    "IR - Toggle On/Off",                        // 160
    "Smoker - Manual Toggle",                    // 161  -- see also 52 for analag manual control of the smoker output speed and 101-102 for on/off
    "User Sounds - Stop All",                    // 162
    "Smoker Preheat - Enable",                   // 163
    "Smoker Preheat - Disable",                  // 164
    "Smoker Preheat - Toggle"                    // 165
};


//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// TRIGGERS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// As discussed above, triggers are the things which are assigned to functions and which cause the function to occur. As with functions there are two basic types of
// triggers, digital and analog. But there are further types beyond that, which represent essentially the source of the trigger (radio turret stick, general purpose 
// input pins, radio aux channels, or other esoteric sources). Each trigger has a unique Trigger ID. The Trigger ID is a number which identifies the trigger but which
// may also contain other information embedded into the number. For example, Trigger ID 6035 identifies radio aux channel #6 (6000), the fact that this channel represents a 
// 3-position switch (30) and that this trigger occurs when the switch is in position 3 (5 - yes, 5). 

// Trigger ID Ranges    Effective Range
// 0 - 99               0 - 36              Turret stick positions
// 100 - 999            100 - 202           General purpose I/O ports A & B (when set to inputs)
// 1000 - 18999         10021 - 12055       Aux radio channels 1-12
// 19000 - 19999        19001 - 19016       Ad-hoc triggers 1-16 (these are events rather than inputs)
// 20000 - 20999        20000 - 20090       Speed increase triggers (when speed rises above a certain level)
// 21000 - 21999        21001 - 21100       Speed decrease triggers (when speed falls below a certain level)
// 22000 - 22999        22001 - 22003       Variable (analog) triggers for throttle command (22001), engine speed (22002) and vehicle speed (22003).
// 23000 - 65535                            Unallocated

// Some range definitions
#define trigger_id_multiplier_ports         100         // External ports input trigger ID is defined as: (multipler_ports * port number) + 0/1 (off/on)

#define trigger_id_multiplier_auxchannel    1000        // Aux channel trigger ID is defined as:
                                                        // (multiplier_auxchannel * channel number) + (number of switch positions * switch_pos_multiplier) + switch position (1-5)
#define switch_pos_multiplier               10          // Move the number of switch positions to the tens slot (the actual position will be in the ones slot)  

#define trigger_id_adhoc_start              19000       // Trigger IDs for ad-hoc events. Range FROM trigger_id_adhoc_start TO (trigger_id_adhoc_start + trigger_id_adhoc_range - 1)
#define trigger_id_adhoc_range              1000

#define trigger_id_speed_increase           20000       // Trigger IDs for speed rising above  a set level. Range FROM trigger_id_speed_increase TO (trigger_id_speed_increase + trigger_id_speed_range - 1)
#define trigger_id_speed_decrease           21000       // Trigger IDs for speed falling below a set level. Range FROM trigger_id_speed_decrease TO (trigger_id_speed_decrease + trigger_id_speed_range - 1)
#define trigger_id_speed_range              1000        

#define trigger_id_throttle_command         22001       // Variable trigger synchronous with actual throttle stick position
#define trigger_id_engine_speed             22002       // Variable trigger synchronous with engine speed (modified/massaged throttle command)
#define trigger_id_vehicle_speed            22003       // Variable trigger synchronous with vehicle movement speed
#define trigger_id_steering_command         22004       // Variable trigger synchronous with steering input (stick position)
#define trigger_id_rotation_command         22005       // Variable trigger synchronous with turret rotation (stick position)
#define trigger_id_elevation_command        22006       // Variable trigger synchronous with barrel elevation (stick position)


// Function/Trigger Pair Definition
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
#define MAX_FUNCTION_TRIGGERS 40            // Maximum number of function/trigger pairs we can save

// An array of this struct will be saved in EEPROM and a copy in RAM. 
// These define function/trigger pairs: 
typedef struct _functionTrigger 
{
    uint16_t TriggerID;                     // Each _functionTrigger has a Trigger ID
    _special_function specialFunction;      // Each _functionTrigger has a function that will be executed when some input state matches the Trigger ID
};


// Trigger Sources
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// Trigger sources: these are not needed in TCB firmware but are used by OP Config, we keep a copy here for the fun of it. 
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
    TS_INPUT_B,            // External input B (if set to input)
    TS_SPEED_INCR,         // Vehicle speed increases beyond a set point
    TS_SPEED_DECR,         // Vehicle speed decreased below  a set point
    TS_THROTTLE_COMMAND,   // Throttle command (variable)
    TS_ENGINE_SPEED,       // Engine speed (variable)
    TS_VEHICLE_SPEED,      // Vehicle speed (variable)
    TS_STEERING_COMMAND,   // Steering input (variable)
    TS_ROTATION_COMMAND,   // Turrent rotation input (variable)
    TS_ELEVATION_COMMAND,  // Barrel elevation input (variable)
    TS_ADHC_BRAKES,        // Ad-hoc - brakes applied
    TS_ADHC_CANNONHIT,     // Ad-hoc - received cannon hit
    TS_ADHC_DESTROYED,     // Ad-hoc - vehicle destroyed
    TS_ADHC_CANNONRELOAD,  // Ad-hoc - cannon reloaded    
    TS_ADHC_ENGINE_START,  // Ad-hoc - engine start
    TS_ADHC_ENGINE_STOP,   // Ad-hoc - engine stop
    TS_ADHC_MOVE_FWD,      // Ad-hoc - forward movement started
    TS_ADHC_MOVE_REV,      // Ad-hoc - reverse movement started
    TS_ADHC_VEHICLE_STOP,  // Ad-hoc - vehicle stopped
    TS_ADHC_RIGHT_TURN,    // Ad-hoc - right turn started
    TS_ADHC_LEFT_TURN,     // Ad-hoc - left turn started
    TS_ADHC_NO_TURN,       // Ad-hoc - no turn
    TS_ADHC_UNUSED_13,     // Ad-hoc - unused      
    TS_ADHC_UNUSED_14,     // Ad-hoc - unused      
    TS_ADHC_UNUSED_15,     // Ad-hoc - unused      
    TS_ADHC_UNUSED_16      // Ad-hoc - unused          
};


// Turret Stick Triggers
//------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// The turret stick can occupy one of 9 positions, each of which can serve as a trigger. 
// You can see OP_RadioDefines.h for more on the turret stick special positions math. 
// The possible TriggerID values are: 
    // Top left      = 36    (32+4)
    // Top center    = 34    (32+2)
    // Top right     = 33    (32+1)
    // Middle left   = 20    (16+4)
    // Middle center = 18    (16+2)    - this is stick untouched, assuming your radio self-centers the stick
    // Middle right  = 17    (16+1)
    // Bottom left   = 12    (8+4)
    // Bottom center = 10    (8+2)
    // Bottom right  = 9     (8+1)


// General Purpose Input Pins A & B (I/O ports when set to input)
//------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// Trigger IDs can also be digital I/O ports (when set to input). The TriggerID in that case will be: 
//   (PortNumber * trigger_id_multiplier_ports) + Input Switch Position
//   These are always considered 2 position switches so we don't need to include information about the number of positions
//   For example, the trigger ID for Port B (A = 1, B = 2) in the On position would be: 201
//   The trigger ID for Port A in the Off position would be 100. 
// In the case of analog inputs the Trigger ID is simply (PortNumber * trigger_id_multiplier_ports) and the current position (value) of the input 
// is taken directly from an analog read instruction on the pin rather than being embedded in the Trigger ID itself. 


// Radio Aux Channels
//------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// Trigger IDs can also be digital aux channel inputs. The TriggerID in that case will be: 
//   (AuxChannelNumber * trigger_id_multiplier_auxchannel) + (NumberOfSwitchPositions * switch_pos_multiplier) + Switch Position    
//   So for example, if you wanted to trigger a function when the Aux Channel 3 switch is in position 2 of a 2-position switch, 
//   the trigger ID will be: 3025 <- Yes, that is right! 3000 for the 3rd Aux Channel, 20 for a two-position switch, 5 for position 2
//   Does that make sense? Remember that positions are always 1-5. If you have a two-position switch, position 1 = 1 and position 2 = 5.
//   If you have a three position switch, position 1 = 1, position 2 = 3, position 3 = 5. 
//   If you have a five-position switch, each position equals its own number (1-5)
// In the case of analog aux channels the Trigger ID is simply (AuxChannelNumber * trigger_id_multiplier_auxchannel) and the current position (value) of the channel
// is taken from the channel reading itself, not the Trigger ID. 


// Speed Increase and Decrease Triggers
//------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// Speed increase trigger IDs are defined as: 
//   trigger_id_speed_increase + speed level pct (number from 0-99)
//   When the vehicle speed changes from a number equal to or less than the set level percent, to a number higher than the set level percent, then the trigger occurs.
//   The set level is from 0-99 because you can not rise any higher than above 99 percent.
// Speed decrease trigger IDs are defined as: 
//   trigger_id_speed_decrease + speed level pct (number from 1-100)
//   When the vehicle speed changes from a number greater than or equal to the set level percent, to a number lower than the set level percent, then the trigger occurs.
//   The set level is from 1 to 100 because you can not fall any lower than below 1 percent. 


// Ad-Hoc Function Triggers
//------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// Ad-Hoc triggers are unlike other triggers in that they are not external inputs (radio channels, literal pin inputs, etc)
// Instead ad-hoc triggers are typically events that occur within the program, to which we may want to assign some other function/event. 
// Keep in mind, it doesn't make sense to create an ad-hoc trigger for most events. Consider the creation of an ad-hoc trigger to occur when the cannon 
// is fired. This could easily be done, and yes, cannon fire is an event that we might want other events to occur at the same time as. But - for the cannon 
// fire event to occur, you already have to assign a trigger to it (for example, a switch on your transmitter). If you want other things to happen at the 
// same time, just assign those other events to the same trigger! A trigger can control multiple events, so that is no problem. That would be the most 
// direct way and no need for us to create additional complexity here. 
// But in time, who knows, we may think of new events that we want for triggers, so it's good to have a process to implement them. 

// Count of active Ad-Hoc triggers
#define COUNT_ADHOC_TRIGGERS            12          // This number can not get higher than 16 unless you want to change some methods in the sketch

// Ad-Hoc trigger Flag Masks
// We don't anticipate needing many ad hoc trigger events. In the Sketch we will use a single 2-byte integer for 16 flags. The sketch sets the appropriate bit when an event occurs, 
// the flag causes execution of any function with the corresponding trigger ID defined below, and finally the flag bit is cleared after execution for the next round. 
#define ADHOCT_BIT_BRAKES_APPLIED       0           // Brakes just applied
#define ADHOCT_BIT_CANNON_HIT           1           // Vehicle received cannon hit
#define ADHOCT_BIT_VEHICLE_DESTROYED    2           // Vehicle destroyed
#define ADHOCT_BIT_CANNON_RELOADED      3           // Cannon reloaded
#define ADHOCT_BIT_ENGINE_START         4           // Engine started
#define ADHOCT_BIT_ENGINE_STOP          5           // Engine stopped
#define ADHOCT_BIT_MOVE_FORWARD         6           // Forward movement begun
#define ADHOCT_BIT_MOVE_REVERSE         7           // Reverse movement begun
#define ADHOCT_BIT_VEHICLE_STOP         8           // Vehicle has come to a stop
#define ADHOCT_BIT_RIGHT_TURN           9           // Right turn begun
#define ADHOCT_BIT_LEFT_TURN            10          // Left turn begun
#define ADHOCT_BIT_NO_TURN              11          // Vehicle is no longer turning

// Ad-Hoc trigger Triggger IDs 
// The trigger IDs must start at trigger_id_adhoc_start and go up from there, but not exceed (trigger_id_adhoc_start + trigger_id_adhoc_range - 1)
// In practice they shouldn't even go above 19015 because we only have 16 flags implemented in the sketch.
#define ADHOC_TRIGGER_BRAKES_APPLIED    trigger_id_adhoc_start + ADHOCT_BIT_BRAKES_APPLIED      // Ad-Hoc Trigger ID  1 - brakes just applied   19000
#define ADHOC_TRIGGER_CANNON_HIT        trigger_id_adhoc_start + ADHOCT_BIT_CANNON_HIT          // Ad-Hoc Trigger ID  2 - received cannon hit   19001
#define ADHOC_TRIGGER_VEHICLE_DESTROYED trigger_id_adhoc_start + ADHOCT_BIT_VEHICLE_DESTROYED   // Ad-Hoc Trigger ID  3 - vehicle destroyed     19002
#define ADHOC_TRIGGER_CANNON_RELOADED   trigger_id_adhoc_start + ADHOCT_BIT_CANNON_RELOADED     // Ad-Hoc Trigger ID  4 - cannon reloaded       19003
#define ADHOC_TRIGGER_ENGINE_START      trigger_id_adhoc_start + ADHOCT_BIT_ENGINE_START        // Ad-Hoc Trigger ID  5 - engine start          19004
#define ADHOC_TRIGGER_ENGINE_STOP       trigger_id_adhoc_start + ADHOCT_BIT_ENGINE_STOP         // Ad-Hoc Trigger ID  6 - engine stop           19005
#define ADHOC_TRIGGER_MOVE_FORWARD      trigger_id_adhoc_start + ADHOCT_BIT_MOVE_FORWARD        // Ad-Hoc Trigger ID  7 - forward movement      19006
#define ADHOC_TRIGGER_MOVE_REVERSE      trigger_id_adhoc_start + ADHOCT_BIT_MOVE_REVERSE        // Ad-Hoc Trigger ID  8 - reverse movement      19007
#define ADHOC_TRIGGER_VEHICLE_STOP      trigger_id_adhoc_start + ADHOCT_BIT_VEHICLE_STOP        // Ad-Hoc Trigger ID  9 - vehicle stopped       19008
#define ADHOC_TRIGGER_RIGHT_TURN        trigger_id_adhoc_start + ADHOCT_BIT_RIGHT_TURN          // Ad-Hoc Trigger ID 10 - right turn            19009
#define ADHOC_TRIGGER_LEFT_TURN         trigger_id_adhoc_start + ADHOCT_BIT_LEFT_TURN           // Ad-Hoc Trigger ID 11 - left turn             19010
#define ADHOC_TRIGGER_NO_TURN           trigger_id_adhoc_start + ADHOCT_BIT_NO_TURN             // Ad-Hoc Trigger ID 12 - no turn               19011
//                                      19012                                                   // Ad-Hoc Trigger ID 13
//                                      19013                                                   // Ad-Hoc Trigger ID 14
//                                      19014                                                   // Ad-Hoc Trigger ID 15
//                                      19015                                                   // Ad-Hoc Trigger ID 16



// Function Pointers
//------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// At the top of the sketch we will also create an array of MAX_FUNCTION_TRIGGERS function pointers. 
// Whenever a _functionTrigger.TriggerID matches some trigger condition, the function
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
