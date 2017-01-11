/* OP_SoundCard.cpp     Library for communicating via serial with Open Panzer sound cards
 * Source:              openpanzer.org      
 * Authors:             Luke Middleton
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

#include <OP_Sound.h>


void OP_SoundCard::begin()
{
    // Initialize
    _squeaksActive = false;
}

void OP_SoundCard::command(byte command, byte value) const
{
  _port->write(OPSC_ADDRESS);
  _port->write(command);
  _port->write(value);
  _port->write((OPSC_ADDRESS + command + value) & B01111111);
}

void OP_SoundCard::command(byte command) const
{
  this->command(command, 0);
}



