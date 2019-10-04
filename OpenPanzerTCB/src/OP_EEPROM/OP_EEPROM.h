/* OP_EEPROM.h      Open Panzer EEPROM - a library functions related to EEPROM usage
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
 
#ifndef OP_EEPROM_H
#define OP_EEPROM_H

#include <Arduino.h>
// This one has meta-data about every variable stored in a progrmem array
#include "OP_EEPROM_VarInfo.h"
// The order we include these is important. OP_EEPROM_Struct will need these defined first
#include "../OP_FT/OP_FunctionsTriggers.h"
#include "../OP_IO/OP_IO.h"
#include "../OP_IRLib/OP_IRLib.h"
#include "../OP_Tank/OP_Tank.h"
#include "../OP_Radio/OP_RadioDefines.h"
#include "../OP_Sound/OP_Sound.h"
#include "../OP_Motors/OP_Motors.h"
#include "../OP_Smoker/OP_Smoker.h"
#include "../OP_Driver/OP_Driver.h"
#include "../OP_Settings/OP_Settings.h"
#include "../EEPROMex/EEPROMex.h"   // We use the extended version, not Arduino's built-in EEPROM library. 
// This one is important. It has the definition of our eeprom struct
#include "OP_EEPROM_Struct.h"


//--------------------------------------------------------------------------------------------------------------------------------------->>
//=======================================================================================================================================>>
// VERY IMPORTANT ! 
//=======================================================================================================================================>>
// If any changes are made to the _eeprom_data struct in OP_EEPROM_Struct.h, the EEPROM_INIT definition below must be changed to a new number.
// This will force the sketch to re-initialize the EEPROM and all old data will be lost - in other words, you will have to redo
// radio setup, etc... (unless you wisely saved an OPZ file in OP Config first). This is an inconvenient but not as bad as if you don't reinitalize. 
// In that case EEPROM data corruption WILL occur and the sketch will exhibit unstable behavior!
// 

    #define EEPROM_INIT             0xBFF0          // Modified with 00.93.63 on 10/4/2019
//
//
//=======================================================================================================================================>>
//--------------------------------------------------------------------------------------------------------------------------------------->>


#define EEPROM_START_ADDRESS    0



// Class OP_EEPROM
class OP_EEPROM
{   public:
        OP_EEPROM(void);                            // Constructor
        static boolean begin(void);                 // This function will initialize EEPROM to default values if it hasn't been before.
                                                    // After initialization, the EEPROM_INIT number above will be written to EEPROM, so
                                                    // next time we'll know it's been done. This prevents us from writing to the EEPROM over and over
                                                    // every time we turn on the Arduino. 
        // Vars
        static _eeprom_data ramcopy;                // This is the complete eeprom struct that we will store in RAM
        
        // Functions
        static void loadRAMcopy(void);              // This will load the eeprom data from eeprom into our ramcopy struct in RAM
        
        static boolean readSerialEEPROM_byID(uint16_t ID, char * chrArray, uint8_t bufflen, uint8_t &stringlength);
        static boolean updateEEPROM_byID(uint16_t ID, uint32_t Value);      // This will update the variable of "ID" with "Value" in EEPROM

        static void factoryReset(void);             // This will force a call to Initialize_EEPROM(). All eeprom vars will be rest to default values. 

    protected:

        // Functions
        static void Initialize_RAMcopy(void);       // Called by Initilize_EEPROM. It sets all the RAM variables to default values. 
        static void Initialize_EEPROM(void);        // This copies all variables (at default values) from RAM into eeprom.
        
        static uint16_t findStorageVarInfo(_storage_var_info &svi, uint16_t findID);    // When we know the var ID but not the position in the array it occupies
        static boolean getStorageVarInfo(_storage_var_info &svi, uint16_t arrayPos);    // For when we already know the array element we want

        // Vars
        
};




#endif
