/* OP_Sound.cpp     Open Panzer Sound - container class for various supported sound devices
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
 

#include "OP_Sound.h"

// Little function to help us print out actual sound device names, rather than numbers. 
// To use, call something like this:  Serial.print(printSoundDevice(SOUND_DEVICE))
const __FlashStringHelper *printSoundDevice(SOUND_DEVICE Device)
{
    if(Device>SD_LAST_SD) Device=SD_LAST_SD+1;
    const __FlashStringHelper *Names[SD_LAST_SD+2]={F("Benedini TBS Mini"),F("Open Panzer Sound Card"),F("Taigen Sound Card"),F("Beier USM-RC-2"),F("Benedini TBS Micro"),F("UNKNOWN")};
    return Names[Device];
}





