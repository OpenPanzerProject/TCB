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
 

#include <OP_Sound.h>

// Little function to help us print out actual sound device names, rather than numbers. 
// To use, call something like this:  Serial.print(printSoundDevice(SOUND_DEVICE))
const __FlashStringHelper *printSoundDevice(SOUND_DEVICE Device)
{
    if(Device>SD_LAST_SD) Device=SD_LAST_SD+1;
    const __FlashStringHelper *Names[SD_LAST_SD+2]={F("Benedini TBS Mini"),F("OP Teensy Sound Card"),F("Beier USM-RC-2"),F("UNKNOWN")};
    return Names[Device];
}






void OP_TeensySound::begin()
{

}

// Engine sound functions   
void OP_TeensySound::StartEngine(void)
{

}
void OP_TeensySound::StopEngine(void)
{

}
void OP_TeensySound::SetEngineSpeed(int)
{

}
void OP_TeensySound::IdleEngine(void)
{

}

// Repair sounds    
void OP_TeensySound::Repair(void)
{

}
void OP_TeensySound::StopRepairSound(void)
{

}

// Cannon   
void OP_TeensySound::Cannon(void)
{

}
void OP_TeensySound::CannonHit(void)
{

}
void OP_TeensySound::Destroyed(void)
{

}

// Machine Gun  
void OP_TeensySound::MachineGun(void)
{

}
void OP_TeensySound::StopMachineGun(void)
{

}
void OP_TeensySound::MGHit(void)
{

}

// Turret/Barrel
void OP_TeensySound::Turret(void)
{

}
void OP_TeensySound::StopTurret(void)
{

}
void OP_TeensySound::TurretSound_SetEnabled(boolean)
{

}
void OP_TeensySound::Barrel(void)
{

}
void OP_TeensySound::StopBarrel(void)
{

}
void OP_TeensySound::BarrelSound_SetEnabled(boolean)
{

}

// Headlight
void OP_TeensySound::HeadlightSound(void)
{

}
void OP_TeensySound::HeadlightSound_SetEnabled(boolean)
{

}

// User sounds
void OP_TeensySound::UserSound1(void)
{

}
void OP_TeensySound::UserSound1_Repeat(void)
{

}
void OP_TeensySound::UserSound1_Stop(void)
{

}
void OP_TeensySound::UserSound2(void)
{

}
void OP_TeensySound::UserSound2_Repeat(void)
{

}
void OP_TeensySound::UserSound2_Stop(void)
{

}

// Squeaks  
void OP_TeensySound::StartSqueaks(void)
{

}
void OP_TeensySound::StopSqueaks(void)
{

}
boolean OP_TeensySound::AreSqueaksActive(void)
{

}
void OP_TeensySound::SetSqueak1_Interval(unsigned int, unsigned int)
{

}
void OP_TeensySound::SetSqueak2_Interval(unsigned int, unsigned int)
{

}
void OP_TeensySound::SetSqueak3_Interval(unsigned int, unsigned int)
{

}
void OP_TeensySound::Squeak1_SetEnabled(boolean)
{

}
void OP_TeensySound::Squeak2_SetEnabled(boolean)
{

}
void OP_TeensySound::Squeak3_SetEnabled(boolean)
{

}

// Beeps
void OP_TeensySound::Beep(void)
{

}
void OP_TeensySound::Beeps(int)
{

}

