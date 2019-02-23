/* OP_IO.h          Open Panzer IO - defines and structs related to the two general purpose i/o ports on the TCB board
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
 
#ifndef OP_IO_H
#define OP_IO_H


//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// GENERAL PURPOSE I/O
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>

#define NUM_IO_PORTS    2
#define IO_PULSE_TIME   300     // Time in milliseconds to pulse the IO port if set to output and the user triggers the pulse function

typedef struct external_io_settings{
    uint8_t dataDirection;  // 1 = output, 0 = input
    uint8_t dataType;       // If input, 1 = "digital" (on/off only), 0 = analog (variable), default to digital. If Output, 1 = normally high, 0 = normally low, default normally high.
};

typedef struct external_io{
    boolean inputActive;    // If input, hast his been assigned as a trigger to any function?
    uint16_t inputValue;    // If input, what is the analog reading (0-1023 if analog) or digital reading (0/1 if digital)
    boolean updated;        // If input, has the input value changed since last time? 
    external_io_settings Settings;  // Settings
};





#endif
