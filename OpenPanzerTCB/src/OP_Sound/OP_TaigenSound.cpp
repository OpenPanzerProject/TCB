/* OP_TaigenSound.cpp   Library for communicating with Taigen's standalone sound cards (released in 2017)
 * Source:              openpanzer.org      
 * Authors:             Luke Middleton
 *
 * NOTE:                OP_TaigenSound does not have a corresponding .h (header) file, other than the header information in OP_Sound.h
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


volatile uint16_t OP_TaigenSound::command; 


void OP_TaigenSound::begin() 
{ 
    // Initialize to false, but they will ultimately get set to the user's preference
    _turretEnabled = false;
    _turretSoundActive = false;
    _barrelEnabled = false;    
    _barrelSoundActive = false;
    
    // TaigenSound uses Timer 4 Overflow interrupt. Timer 4 is setup in the main sketch, using a macro defined in OP_Settings.h

    // We set Timer 4 to Fast PWM 10-bit (TOP = 1024) with a prescaler of 1. Timer 4 will tick (TCNT4 increment) once every 0.0625 uS, or in other words, one uS = 16 ticks. 
    // Timer 4 will overflow every (1024 ticks * 0.06uS per tick) = 64 uS. This means that for every 8 overflows just a hair over 1/2 mS will have transpired (8*64 = 512uS = 0.512mS). 
    // This works well because the data stream for the Taigen sound cards all involve pulses in increments of 1/2mS. 

    // Set pint to output, start high
    pinMode(pin_Prop1, OUTPUT);
    digitalWrite(pin_Prop1, HIGH); 

    // Enable Timer 4 Overflow interrupt
    TIMSK4 |= (1 << TOIE4);         // TIMSK4, bit TOIE4 = Timer Overflow Interrupt Enable 4
                                    // We set this flag to enable an interrupt to occur whenever TCNT4 equals TOP. 
                                    // We will need to wait longer than 64uS but we simply overflow multiple times and count the number of overflows in 64uS chunks until we reach the length of time we need. 
    
    // No command to start
    command = 0x00;
    
    // Engine start 0x40;
}


// We call a class function so that we can take advantage of class variables, 
// which are not visible to the actual ISR
ISR(TIMER4_OVF_vect)
{
    OP_TaigenSound::OVF_ISR();
}

// Timer 4 Overflow interrupt routine
void OP_TaigenSound::OVF_ISR()
{
    static uint8_t NextState = 0;
    static uint8_t OverflowsToWait = 0;
    static uint8_t OverflowsWaited = 0;
    static uint8_t bitNum = 0;
    static uint16_t data = 0;
    static boolean TriggerSent = false;

    #define OverflowPerHalfmS 8         // How many overflows equals 1/2 millisecond

    // We use a state machine to decide if the pin should go high or low, and for how long

    OverflowsWaited += 1; 
    
    if (OverflowsWaited >= OverflowsToWait)     
    {
        OverflowsWaited = 0;
        
        switch (NextState)
        {
            case 0:                                         // Header low
                digitalWrite(pin_Prop1, LOW);   
                OverflowsToWait = 8 * OverflowPerHalfmS;    // 4.16 mS, we just wait 4.096
                NextState = 1;  
                break;  
    
            case 1:                                         // Header high
                digitalWrite(pin_Prop1, HIGH);              // 0.52 mS, we wait 0.512
                OverflowsToWait = 1 * OverflowPerHalfmS;    
                NextState = 2;  
                break;  
                    
            case 2:                                         // Data low - the length of this determines if bit is 1 or 0
                digitalWrite(pin_Prop1, LOW);   
                if (data & TOP_BIT)     
                {   
                    OverflowsToWait = 3 * OverflowPerHalfmS;// If this bit is 1, drop for 1.56 mS, we do 1.536
                    NextState = 4;                          // 1 bits have shorter high portion after
                }   
                else    
                {   
                    OverflowsToWait = 1 * OverflowPerHalfmS;// If this bit is 0, drop for 0.52 ms, we do 0.512
                    NextState = 3;                          // 0 bits have longer high portion after
                }   
                break;  
                
            case 3:                                         // Data high - 0 bit
                digitalWrite(pin_Prop1, HIGH);  
                OverflowsToWait = 3 * OverflowPerHalfmS;    // If the bit was 0 (short), the high portion is longer (1.56mS, we do 1.536)
                if (++bitNum >= NUM_BITS)                   // Increment bitNum and see if we're done
                {       
                    bitNum = 0;                             // We've finished, reset the count
                    NextState = 5;                          // Next go to the end
                }   
                else    
                {   
                    data <<= 1;                             // Still have more, shift to next bit
                    NextState = 2;                          // and go back to 2
                }   
                break;  
                
            case 4:                                         // Data high - 1 bit
                digitalWrite(pin_Prop1, HIGH);  
                OverflowsToWait = 1 * OverflowPerHalfmS;    // If the bit was 1 (long), the high portion is shorter (0.52mS, we do 0.512)
                if (++bitNum >= NUM_BITS)                   // Increment bitNum and see if we're done
                {       
                    bitNum = 0;                             // We've finished, reset the count
                    NextState = 5;                          // Next go to the end
                }   
                else    
                {   
                    data <<= 1;                             // Still have more, shift to next bit
                    NextState = 2;                          // and go back to 2
                }   
                break;  
                    
            case 5:                                         // Closing low
                digitalWrite(pin_Prop1, LOW);               // 0.52 mS, we wait 0.512
                OverflowsToWait = 1 * OverflowPerHalfmS;    // This will only take a single overflow
                NextState = 6;            
                break;
            
            case 6:                                         // Gap high
                digitalWrite(pin_Prop1, HIGH);
                OverflowsToWait = 24 * OverflowPerHalfmS;   // 12.52 mS, we wait 12.288
                data = command;                             // Here is where we update our data variable
                
                // Start signal only gets sent once
                if (command & TS_MASK_ENGINE_START) 
                { 
                    if (TriggerSent)
                    {
                        command ^= TS_MASK_ENGINE_START;    // Clear the start signal
                        command |= TS_MASK_ENGINE_IDLE;     // Set the engine idle flag
                        TriggerSent = false;                // Reset
                    }
                    else    TriggerSent = true;             // Set flag so this only happens once
                }
                
                
                // Stop signal only gets sent once
                if (command & TS_MASK_ENGINE_STOP)
                {
                    if (TriggerSent) 
                    { 
                        command = 0;                        // Turn off all sounds
                        TriggerSent = false;                
                    }
                    else    TriggerSent = true;            
                }
                
                // Stop Cannon
                if (command & TS_MASK_CANNON)
                {
                    if (TriggerSent) 
                    { 
                        command ^= TS_MASK_CANNON;         
                        TriggerSent = false;
                    }
                    else    TriggerSent = true;            
                }

                // Stop Hit
                if (command & TS_MASK_CANNON_HIT)
                {
                    if (TriggerSent) 
                    { 
                        command ^= TS_MASK_CANNON_HIT;            
                        TriggerSent = false;
                    }
                    else    TriggerSent = true;            
                }
                
                // Stop Destroy
                if (command & TS_MASK_DESTROY)
                {
                    if (TriggerSent) 
                    { 
                        command ^= TS_MASK_DESTROY;        
                        TriggerSent = false;
                    }
                    else    TriggerSent = true;            
                }
                
                // Set our local data variable, then repeat
                data = command;               
                NextState = 0;                
                break;
        }
    }
}

// OR   set   bit without affecting other bits
// XOR  clear bit without affecting other bits

void OP_TaigenSound::StartEngine(void)
{
    command |= TS_MASK_ENGINE_START;
    command |= TS_MASK_ENGINE_IDLE;
}

void OP_TaigenSound::StopEngine(void)
{
    command = TS_MASK_ENGINE_STOP; // Clear all other bits and set the engine stop bit only
}

void OP_TaigenSound::SetEngineSpeed(int s)
{
    // Speed should be some value between MOTOR_MAX_REVSPEED (-255) to MOTOR_MAX_FWDSPEED (255). 
    // The engine speed is always positive, so we use abs() to convert reverse values. If anything outside -255/255 is passed, 
    // behavior will be unpredictable! Keep in mind also, in the case of engine sound, a speed of 0 should correspond to engine idle, not engine stopped. 
    
    // NOTES: To prevent a jerky sound you can manipulate ThrottleSpeed by adjusting the GetThrottleSpeed() function in OP_Driver.cpp. 
    // The Taigen sound card will definitely sound jerky if you jump straight to full speed from idle, or vice-versa. We've introduced some ramping
    // in OP_Driver and it does help, particularly on deceleration. The main problem with the Taigen sound card is that throttle response from idle
    // has a noticeable lag - this is nothing to do with the TCB or the signal not making it directly. But for whatever reason, when you first hit the throttle
    // the Taigen sound card does not respond immediately. 
    
    s = abs(s);
    s = constrain(s, 0, 255);
    
    // First, clear all engine bits. The ENGINE_CLEAR_MASK has 1 in every bit not related to the engine, but 0 in all bits that are. 
    command &= TS_ENGINE_CLEAR_MASK;
    
    // Now set appropriate engine bits
    if      (s < TS_SPEED1_RANGE)   command |= TS_MASK_ENGINE_IDLE; 
    else if (s < TS_SPEED2_RANGE)   command |= TS_MASK_ENGINE_SPEED_1;
    else if (s < TS_SPEED3_RANGE)   command |= TS_MASK_ENGINE_SPEED_2;
    else if (s < TS_SPEED4_RANGE)   command |= TS_MASK_ENGINE_SPEED_3;
    else if (s < TS_SPEED5_RANGE)   command |= TS_MASK_ENGINE_SPEED_4;
    else if (s < TS_SPEED6_RANGE)   command |= TS_MASK_ENGINE_SPEED_5;
    else                            command |= TS_MASK_ENGINE_SPEED_6;
}

void OP_TaigenSound::IdleEngine(void)
{
    SetEngineSpeed(0);
}

void OP_TaigenSound::MachineGun(void)
{
    command |= TS_MASK_MG;      
}

void OP_TaigenSound::StopMachineGun(void)
{
    command ^= TS_MASK_MG;      
}

void OP_TaigenSound::Barrel(void)
{ 
    if (_barrelEnabled && !_barrelSoundActive) 
    { 
        _barrelSoundActive = true;
        command |= TS_MASK_BARREL;
    } 
}

void OP_TaigenSound::StopBarrel(void)
{ 
    if (_barrelEnabled &&  _barrelSoundActive) 
    { 
        _barrelSoundActive = false;  
        command ^= TS_MASK_BARREL;
    }
}

void OP_TaigenSound::Turret(void)
{ 
    if (_turretEnabled && !_turretSoundActive) 
    { 
        _turretSoundActive = true;   
        command |= TS_MASK_TURRET;
    }
}

void OP_TaigenSound::StopTurret(void)
{ 
    if (_turretEnabled &&  _turretSoundActive) 
    { 
        _turretSoundActive = false;  
        command ^= TS_MASK_TURRET;
    } 
}

void OP_TaigenSound::Cannon(void)
{
    command |= TS_MASK_CANNON;
}

void OP_TaigenSound::CannonHit(void)
{
    command |= TS_MASK_CANNON_HIT;
}

void OP_TaigenSound::Destroyed(void)
{
    command |= TS_MASK_DESTROY;
}
