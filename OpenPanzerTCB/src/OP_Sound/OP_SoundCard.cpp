/* OP_SoundCard.cpp     Library for communicating via serial with Open Panzer sound cards
 * Source:              openpanzer.org      
 * Authors:             Luke Middleton
 *
 * NOTE:                OP_SoundCard does not have a corresponding .h (header) file, other than the header information
 *                      in OP_Sound.h
 *
 * LICENSE
 * ===============================================================================================================
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

#include "OP_Sound.h"


void OP_SoundCard::begin() 
{ 
    // Initialize to false, but they will ultimately get set to the user's preference
    _squeaksActive = false;
    _headlightEnabled = false;
    _headlight2Enabled = false;
    _turretEnabled = false;
    _turretSoundActive = false;
    _barrelEnabled = false;    
    _barrelSoundActive = false;
}

void OP_SoundCard::command(byte command, byte value, byte modifier) const
{
    _port->write(OPSC_ADDRESS);
    _port->write(command);
    _port->write(value);
    _port->write(modifier);
    _port->write((OPSC_ADDRESS + command + value + modifier) & B01111111);
}

void OP_SoundCard::command(byte command, byte value) const
{
    this->command(command, value, 0); 
}

void OP_SoundCard::command(byte command) const
{
    this->command(command, 0, 0);
}

void OP_SoundCard::SendSqueakIntervals(unsigned int min, unsigned int max, uint8_t squeakNum) const
{
    uint8_t fmin;
    uint8_t fmax;
    // OP Config allows intervals from 500 to 10,000 mS (1/2 to 10 seconds) in 50 mS increments. 
    min = constrain(min, 500, 10000);
    max = constrain(max, 500, 10000);
    
    // The value we pass is in 50mS increments and will be some number from 0-190 (0=500, 190=10000)
    fmin = (min - 500) / 50;    
    fmax = (max - 500) / 50;
    command(OPSC_CMD_SQUEAK_SET_MIN, fmin, squeakNum);  // Command, value, modifier
    command(OPSC_CMD_SQUEAK_SET_MAX, fmax, squeakNum);    
}

