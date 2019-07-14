/* OP_Servo.cpp     Open Panzer Servo - a library for writing servo pulses to 8 pins on a single port
 * Source:          openpanzer.org              
 * Authors:         Michael Margolis, DuaneB, Luke Middleton
 *
 * This library is a modification of DuaneB's RCArduionFastLib: http://rcarduino.blogspot.com/
 * ...which itself was a modification of the standard Arduino Servo library written by Michael Margolis (Arduino Forum user "MEM")
 *
 * This library is intended to be used on a specific piece of hardware, the Open Panzer TCB board. 
 * It has been stripped down substantially. It can only drive 8 servos, and they must all be on the same hardware port (PORTA for now). 
 * Additions have been made for servo ramping (used for panning servos and other effects, such as recoil)
 * This library is therefore not compatible with MEM or DuaneB's library nor is it entirely compatible with most Arduino boards except the Mega (with caveats). 
 * 
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



#include "OP_Servo.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                   //
//  THIS LIBRARY ONLY WORKS WITH 8 PINS CONNECTED TO A SINGLE PORT REGISTER.                         //
//  THE PORT MUST BE SET HERE!                                                                       //
//                                                                                                   //
    volatile uint8_t* ServoDDR  = &DDRA;                                                             //
    volatile uint8_t* ServoPort = &PORTA;                                                            //
//                                                                                                   //
//                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////

 
// Static variables must be initialized outside the class 
volatile OP_Servos::PortPin OP_Servos::Channel[SERVO_OUT_COUNT]; 
volatile uint8_t OP_Servos::CurrentChannel;

// They are set to private, so you won't be able to access them from the sketch
boolean  OP_Servos::initialized = false;


// CONSTRUCTOR - INITIALIZE CLASS VARIABLES
OP_Servos::OP_Servos() 
{
    // Only do this once. The constructor can actually be called several times since several motor
    // objects are subclasses, and each time they get created this gets run. But we only really 
    // want it to run a single time. 
    
    if (!initialized)
    {
        // This initializes the channel pulse widths to SERVO_OUT_CENTERPULSE
        // AND it also sets Enabled = False to start with. An explicit call to attach 
        // will be required to enable a servo. 
        CurrentChannel = 0;
        uint8_t sreg = SREG;            // Save interrupt register
        cli();                          // Disable interrupts
            while(CurrentChannel < SERVO_OUT_COUNT)
            {
                Channel[CurrentChannel].NumTicks = SERVO_uS_TO_TICKS(SERVO_OUT_CENTERPULSE); 
                Channel[CurrentChannel].MinTicks = SERVO_uS_TO_TICKS(SERVO_OUT_MINPULSE); 
                Channel[CurrentChannel].MaxTicks = SERVO_uS_TO_TICKS(SERVO_OUT_MAXPULSE); 
                Channel[CurrentChannel].PinMask = 1 << CurrentChannel;
                Channel[CurrentChannel].Enabled = false;
                Channel[CurrentChannel].TickStep = 0;       // Default to no ramp
                Channel[CurrentChannel].RecoilState = 0;    // No recoil effect
                Channel[CurrentChannel].RecoilTickStep_Return = 0;      // Default no return effect 
                CurrentChannel++;
            }
        SREG = sreg;                    // Restore register
        
        // We just hard-code the FrameSpace setup here rather than have the user do it. 
        // Because this library assumes and already knows we have 8 servos, we already know the maximum guaranteed refresh rate we can achieve is 62.5hz:
        // Assume the worst case scenario where each servo pulse is at the maximum width of 2000 uS, that means the servos themselves will take 8*2000 = 16,000 uS
        // If we set FrameSpace to 0 (meaning no extra delay), that would be:
        // 1,000,000 uS per second / 16,000 uS per frame = 62.5 frames per second, in other words, 62 hz refresh rate. 
        //
        // 50hz is a pretty standard refresh rate, that would be a complete frame every (1 million uS per second / 50 times per second) = 20,000 uS
        // In that case FrameSpace should be (20,000 - 16,000) = 4,000 uS. 

        // Here we are setting the framespace to 2000. This gives us a minimum guaranteed refresh rate of ~55hz but because the refresh rate isn't dynamic 
        // and depends on the actual pulse widths, it could be as high as 100hz (all 8 servos at minimum pulse width of 1000). 
        // Anyway, this should work fine with all normal servos. 
        OP_Servos::setFrameSpace(8, 2000);
        
        // Don't run this again
        initialized = true;
    }
}

void OP_Servos::begin()
{
    // OP_Servos uses Timer 1 Compare A interrupt. Timer 1 is setup in the main sketch, using a macro defined in OP_Settings.h

    // Enable Timer 1 Output Compare A interrupt
    TIMSK1 |= (1 << OCIE1A);        // TIMSK1, bit OCIE1A = Output Compare Interrupt Enable 1 A. 
                                    // Set this flag to one to enable an interrupt to occur when the TCNT1 equals the Timer 1 Compare A value (OCR1A)
    
    OCR1A = TCNT1 + 4000;           // Start in two milliseconds (4000 ticks at 2 ticks per microsecond/uS)

}

void OP_Servos::attach(uint8_t WhatChannel)
{
    if(WhatChannel >= SERVO_OUT_COUNT) return;

    // Attaching a servo sets the Enabled flag, and sets the pin to Output
    uint8_t sreg = SREG;            // Save interrupt register
    cli();                          // Disable interrupts
        Channel[WhatChannel].Enabled = true;
    SREG = sreg;                    // Restore register

    *ServoDDR |= 1 << WhatChannel;  // We set the pin to output

}


void OP_Servos::detach(uint8_t WhatChannel)
{
    // We may wish to use some of these pins for alternate uses besides servos. In that case, we can "detach" the channel which
    // sets the Enabled flag to false. When this flag is false, the ISR will not toggle the pin high and low although it 
    // will STILL set a timer for this channel, only nothing will happen during that span. In other words, the timer still acts as if we
    // have eight servo output channels, only disabled channels won't have their pins toggled. We set the NumTicks to MINPULSE
    // so any detached servos have the effect of slightly increasing the frame rate. 
    if (WhatChannel >= SERVO_OUT_COUNT) return;
    
    uint8_t sreg = SREG;        // Save interrupt register
    cli();                      // Disable interrupts
        Channel[WhatChannel].NumTicks = SERVO_uS_TO_TICKS(SERVO_OUT_MINPULSE);
        Channel[WhatChannel].Enabled = false;
    SREG = sreg;
}

boolean OP_Servos::isAttached(uint8_t WhatChannel)
{
    return Channel[WhatChannel].Enabled;
}


// Timer1 Output Compare A interrupt service routine
ISR(TIMER1_COMPA_vect)
{
    OP_Servos::OCR1A_ISR();
}


void OP_Servos::OCR1A_ISR()
{
    // Set the last pin low, set the next pin high, rollover if we reached the last pin
    if(CurrentChannel >= SERVO_OUT_COUNT)
    {   // Start over
        CurrentChannel = 0;
        OP_Servos::setPinLow(SERVO_OUT_COUNT-1);
    }
    else
    {   
        // -1 because we are turning off the last pin
        OP_Servos::setPinLow(CurrentChannel-1);
    }

    // Done with last pin, now set this pin high
    OP_Servos::setPinHigh(CurrentChannel);

    // Set the duration of the pulse. We may need to modify the actual pulse if this is a ramped servo
    if ( Channel[CurrentChannel].TickStep != 0 )
    {
        OP_Servos::setPulseWidthTimer_Ramp(CurrentChannel);
    }
    else 
    {
        if ( Channel[CurrentChannel].RecoilState == 1 && ((millis() - Channel[CurrentChannel].RecoilStartTime) >  Channel[CurrentChannel].RecoilTime_mS ))
        {   // Recoil time is up. Start the return. 
            Channel[CurrentChannel].TickStep = Channel[CurrentChannel].RecoilTickStep_Return;   // Ramp back to starting position at the rate specified in the recoil return setting
            Channel[CurrentChannel].RecoilState = 2;                // Kickback done, on the return journey
            OP_Servos::setPulseWidthTimer_Ramp(CurrentChannel);     // This is a ramped movement
        }       
        else 
        {   // This is the regular action. It simply sets the pulse width time
            OP_Servos::setPulseWidthTimer(CurrentChannel);
        }
    }

    // Go to next channel
    CurrentChannel++;
}



void OP_Servos::setPinHigh(uint8_t WhatChannel)
{
    if (Channel[WhatChannel].Enabled)
    {   // OR - set high the pin matching the mask  
        *ServoPort |= Channel[WhatChannel].PinMask; 
    }
}


void OP_Servos::setPinLow(uint8_t WhatChannel)
{
    // XOR - if two values match, set to 0. If they differ, set to 1
    // This turns off the pin that matches the mask, if it is already high (1 and 1 match so set to 0)
    // For all the other pins, which should already be off, they will match the 0s in the mask (0 and 0 match so set to 0, meaning, no difference)
    // If by any chance any other pin is high (we could be using it as a digital output), it will not match the mask (1 and 0 don't match)
    // In that case, XOR will set to 1 - so there again, it stays the same. 

    if (Channel[WhatChannel].Enabled)
    {   // In short, this line will turn off the high pin matching the mask, and leave all the other pins unchanged. 
        *ServoPort ^= Channel[WhatChannel].PinMask;
    }
}


// After we set an output pin high, we need to set the timer to come back for the end of the pulse 
void OP_Servos::setPulseWidthTimer(uint8_t WhatChannel)
{
    OCR1A = TCNT1 + Channel[WhatChannel].NumTicks; 
}


// This is a similar function to the above except it increments or decrements the pulsewidth by the value of Step
// We make it a separate function because it takes longer and this is run from within the ISR. If we don't need to use it,
// which we often won't, the faster, inline function above will be called
void OP_Servos::setPulseWidthTimer_Ramp(uint8_t WhatChannel)
{
    // Add step to current count. Step can be positive or negative.
    uint16_t TotalTicks = (uint16_t)((int16_t)Channel[WhatChannel].NumTicks + Channel[WhatChannel].TickStep);
    // TotalTicks needs to be unsigned, but we want signed for TickStep so we can add/subtract, hence the casting. 

    // Because adding or subtracting these could cause the value to go beyond our channel's 
    // specific min and max, we constrain
    if ((TotalTicks > Channel[WhatChannel].MaxTicks) || (TotalTicks < Channel[WhatChannel].MinTicks))
    {
        TotalTicks = constrain(TotalTicks, Channel[WhatChannel].MinTicks, Channel[WhatChannel].MaxTicks);

        // Exceeding the limits is also a signal we need to change the recoil status, if indeed this is a recoil event. 
        if (Channel[WhatChannel].RecoilState == 2)
        {// If RecoilState = 2, this means the barrel has completed the recoil and can now be reset
            Channel[WhatChannel].TickStep = 0;      // No more ramping
            Channel[WhatChannel].RecoilState = 0;   // Recoil over
        }
    }

    // After constraint, we set the channel's current tick value to the new value (plus or minus the step)
    // This is what allows it to continue to change gradually over time
    Channel[WhatChannel].NumTicks = TotalTicks;
    
    // Finally, set the timer
    OCR1A = TCNT1 + TotalTicks;
}


// Set a channel to a specific value in microseconds
void OP_Servos::writeMicroseconds(uint8_t WhatChannel, uint16_t Set_uS)
{
    if(WhatChannel > SERVO_OUT_COUNT)
    return;
    
    // Convert to Ticks
    uint16_t Set_Ticks = SERVO_uS_TO_TICKS(Set_uS);

    // Each servo can have its own min and max travel values. Constrain here to stay within these limits. 
    Set_Ticks = constrain(Set_Ticks, Channel[WhatChannel].MinTicks, Channel[WhatChannel].MaxTicks);

    uint8_t sreg = SREG;        // Disable interrupts while we update the multi byte value 
    cli();
    Channel[WhatChannel].NumTicks = Set_Ticks; 
    SREG = sreg;                // enable interrupts
}

uint16_t OP_Servos::getPulseWidth(uint8_t WhatChannel)
{
    // Return this servo's pulse width in uS
    if(WhatChannel > SERVO_OUT_COUNT)
    {
        return 0;
    }
    else
    {
        return SERVO_TICKS_TO_uS(Channel[WhatChannel].NumTicks);
    }
}

void OP_Servos::setMinPulseWidth(uint8_t WhatChannel, uint16_t Set_uS)
{
    // We allow each servo to have its own min/max pulsewidths (travel limits)
    // But they can't exceed the global maximums defined in the header file
    if(WhatChannel > SERVO_OUT_COUNT)
    return;

    // Constrain the value just in case
    Set_uS = constrain(Set_uS, SERVO_OUT_MINPULSE, SERVO_OUT_MAXPULSE);
    
    // Now convert to Ticks
    uint16_t Set_Ticks = SERVO_uS_TO_TICKS(Set_uS);

    uint8_t sreg = SREG;        // Disable interrupts while we update the multi byte value 
    cli();
    Channel[WhatChannel].MinTicks = Set_Ticks; 
    SREG = sreg;                // enable interr
}

void OP_Servos::setMaxPulseWidth(uint8_t WhatChannel, uint16_t Set_uS)
{
    // We allow each servo to have its own min/max pulsewidths (travel limits)
    // But they can't exceed the global maximums defined in the header file
    if(WhatChannel > SERVO_OUT_COUNT)
    return;

    // Constrain the value just in case
    Set_uS = constrain(Set_uS, SERVO_OUT_MINPULSE, SERVO_OUT_MAXPULSE);
    
    // Now convert to Ticks
    uint16_t Set_Ticks = SERVO_uS_TO_TICKS(Set_uS);

    uint8_t sreg = SREG;        // Disable interrupts while we update the multi byte value 
    cli();
    Channel[WhatChannel].MaxTicks = Set_Ticks; 
    SREG = sreg;            
}

uint16_t OP_Servos::getMinPulseWidth(uint8_t WhatChannel)
{
    // Return this servo's min pulse width in uS
    if(WhatChannel > SERVO_OUT_COUNT)
    {
        return SERVO_OUT_MINPULSE;
    }
    else
    {
        return SERVO_TICKS_TO_uS(Channel[WhatChannel].MinTicks);
    }
}

uint16_t OP_Servos::getMaxPulseWidth(uint8_t WhatChannel)
{
    // Return this servo's max pulse width in uS
    if(WhatChannel > SERVO_OUT_COUNT)
    {
        return SERVO_OUT_MAXPULSE;
    }
    else
    {
        return SERVO_TICKS_TO_uS(Channel[WhatChannel].MaxTicks);
    }
}

void OP_Servos::setRampSpeed_mS(uint8_t WhatChannel, uint16_t Set_mS, uint8_t Reversed)
{
    // Refresh rate is anywhere from 50~100 hz roughly
    
    // The smallest increment we could have would be one tick increase per frame
    // There is 1000 uS difference from a typical servo min pulse to max pulse (1000 - 2000uS)
    // At 2 ticks per uS that means 2000 frames at 1 tick per frame at the slowest rate to move the servo from one extreme to the other
    // At an average refresh rate of 70hZ (frames per second) this would take ~28 seconds to move the servo from one side to another
    
    // Let's assume a refresh rate of 70hZ (basically all 8 servos are sitting in middle positions, this should be the "average" we think)
    // If we set this to a CONSTANT we can assume some reasonable bounds: 
    // 28 seconds = 1 tick per frame. Any longer than that and we'd have to do less than 1 tick per frame, which is not possible given our setup
    // But for sure no one will need 28 seconds
    // 50mS (0.05 second) = ~570 ticks per frame. This will be much faster than the speed of any servo so at this point the user should just use writeMicroseconds
    // (A typical fast servo would take 0.15 seconds to travel 90 degrees)
    
    // For the user's convenience, we allow them to enter RampSpeed values in terms of milliseconds (mS). This is really just an approximation.
    // The value is intended to represent the time it would take for a servo to move from one extreme to another assuming a 70hZ refresh rate. 
    // In practice we won't always be moving the servo from one extreme to another and the refresh rate won't always be 70hZ. So it's rough but 
    // should be in the ballpark. 
    
    // To convert from mS to ticks per frame, the formula is: 
    // NumTicksFromOneExtremeToAnother / ((milliseconds_desired / 1000) * RefreshRate)
    // The first number is of course 2000 (1000 uS difference from min servo position to max servo position, at 2 ticks per uS = 2000 ticks difference from min to max)
    

    if (WhatChannel > SERVO_OUT_COUNT)
    return;

    // Here we constrain the number of milliseconds to our sane values, 50ms to 28 seconds
    // Assuming 70hZ refresh, keeping it under 28,000 will keep our minimum tick at least 1
    Set_mS = constrain(Set_mS, 50, 28000);

    // Now convert to Ticks per frame. The result is an integer so we are going to be truncating any decimal places in the final answer, 
    // but the actual calculation is done in floating-point. It doesn't matter that we are using standard min/max numbers of 1000/2000 here,
    // the servo will still be able to move to whatever the actual end-points are set to. 
    int16_t Set_Ticks = (int16_t)(2000.0 / (((float)Set_mS / 1000.0) * 70.0));
    
    // Are we moving forward or back
    if (Reversed) { Set_Ticks = -Set_Ticks; }

    uint8_t sreg = SREG;        // Disable interrupts while we update the multi byte value 
    cli();
    Channel[WhatChannel].TickStep = Set_Ticks; 
    SREG = sreg;            
}

void OP_Servos::setupRecoil_mS(uint8_t WhatChannel, uint16_t mS_Recoil, uint16_t mS_Return, boolean Reversed)
{
    // Basically the same as above, only we save two tickstep values: one for "recoiling" the barrel, and one for "returning" it to start position.
    if(WhatChannel > SERVO_OUT_COUNT)
    return;

    // Here we constrain the number of milliseconds to some sane values, 15ms to 28 seconds
    // Assuming 70hZ refresh, keeping it under 28,000 will keep our minimum tick at least 1
    // Anything under 15 would be superfluous because that means the servo would have to move the whole way in less than one frame. 
    mS_Recoil = constrain(mS_Recoil, 15, 28000);
    mS_Return = constrain(mS_Return, 15, 28000);

    // Now convert to Ticks per frame for the return movement. The result is an integer so we are going to be truncating any decimal places in the final answer, 
    // but the actual calculation is done in floating-point
    int16_t Return_Ticks = (int16_t)(2000.0 / (((float)mS_Return / 1000.0) * 70.0));

    uint16_t RecoiledNumTicks;  // For the recoiled movement, we don't ramp - we just go straight to the servo end-point (min or max depending on reversed status)
                                // and then wait for ms_Recoil time
    
    if (Reversed) 
    {   // If reversed we go up to MaxTicks for recoil, then subtract back down slowly to return
        RecoiledNumTicks = Channel[WhatChannel].MaxTicks;
        Return_Ticks = -(abs(Return_Ticks));
    }
    else
    {   // If not reversed we go down to MinTicks for recoil, then add back up slowly to return
        RecoiledNumTicks = Channel[WhatChannel].MinTicks;
        Return_Ticks =   abs(Return_Ticks);
    }

    uint8_t sreg = SREG;        // Disable interrupts while we update the multi byte value 
    cli();
        Channel[WhatChannel].RecoilTime_mS = mS_Recoil;             // How long to wait for the recoil action
        Channel[WhatChannel].RecoiledNumTicks = RecoiledNumTicks;   // Position of the recoiled servo (min or max depending on reversed status)
        Channel[WhatChannel].RecoilTickStep_Return = Return_Ticks;  // Tick step to give the correct length of time for the return action
    SREG = sreg;            
}

void OP_Servos::setRecoilReversed(uint8_t WhatChannel, boolean Reversed)
{
    uint8_t sreg = SREG;        
    cli();
        if (Reversed)
        {
            Channel[WhatChannel].RecoiledNumTicks = Channel[WhatChannel].MaxTicks;
            Channel[WhatChannel].RecoilTickStep_Return = -(abs(Channel[WhatChannel].RecoilTickStep_Return));
        }
        else
        {
            Channel[WhatChannel].RecoiledNumTicks = Channel[WhatChannel].MinTicks;
            Channel[WhatChannel].RecoilTickStep_Return =   abs(Channel[WhatChannel].RecoilTickStep_Return);
        }
    SREG = sreg;                
}

void OP_Servos::StartRecoil(uint8_t WhatChannel)
{
    // This kicks off a recoil event

    // The first phase is to move the servo as quickly as possible back to the opposite extreme. For this we do not use ramping, we simply set the servo position to the opposite end. 
    // Then we wait for RecoilTime_mS milliseconds to let the servo reach that position (value set by the user in OP Config). 
    
    // After that time expires the servo returns to battery slowly. The length of time it takes to return is also set by the user in OP Config, but in this case we use ramping by a certain
    // increment (RecoilTickStep_Return) that has been set to take approximately the time the user wants, and will result in the servo moving smoothly, and slowly back. 

    // Don't start a recoil event until the last one is complete
    if (Channel[WhatChannel].RecoilState != 0) 
    return;
    
    // Ok, now kick it off
    uint8_t sreg = SREG;        // Disable interrupts while we update the multi byte value 
    cli();
        Channel[WhatChannel].RecoilState = 1;
        Channel[WhatChannel].NumTicks = Channel[WhatChannel].RecoiledNumTicks;  // Go straight to the other extreme
        Channel[WhatChannel].RecoilStartTime = millis();                        // Record the time so we know when the wait should be up
    SREG = sreg;            
}

void OP_Servos::setRampStepPerFrame(uint8_t WhatChannel, int16_t Step)
{
    // This function allows the user to set the TickStep directly without having to convert from mS as above. 
    
    // Even a fairly fast servo takes about 0.15 seconds to traverse 90 degrees. Keeping our assumption of 70hZ refresh rate, this
    // speed would equal a change of approximately 200 ticks per frame. 
    
    // In practice it's better to keep the max even lower than this or else panning from a radio stick is too sensitive. 
    // The actual max value is set in the header file as SERVO_MAXRAMP_TICKSTEP, but something <=100 seems to work well. 
    
    if(WhatChannel > SERVO_OUT_COUNT)
    return;

    // Here we constrain the number of steps per frame to sane values
    Step = constrain(Step, -SERVO_MAXRAMP_TICKSTEP, SERVO_MAXRAMP_TICKSTEP);    

    uint8_t sreg = SREG;        // Disable interrupts while we update the multi byte value 
    cli();
    Channel[WhatChannel].TickStep = Step; 
    SREG = sreg;            
}


void OP_Servos::stopRamping(uint8_t WhatChannel)
{
    if(WhatChannel > SERVO_OUT_COUNT)
    return;

    uint8_t sreg = SREG;        // Disable interrupts while we update the multi byte value 
    cli();
    Channel[WhatChannel].TickStep = 0; 
    SREG = sreg;            
}


void OP_Servos::setFrameSpace(uint8_t WhatChannel, uint16_t Set_uS)
{
    uint8_t sreg = SREG;    // Save interrupt register
    cli();                  // Disable interrupts
        Channel[WhatChannel].NumTicks = SERVO_uS_TO_TICKS(Set_uS);
        Channel[WhatChannel].PinMask = 0;
    SREG = sreg;            // Restore register
}


// MOVED TO OP_Settings.h as defines, mostly just to have them all in one place. 
// Convert 
//uint16_t OP_Servos::uS_to_Ticks(uint16_t uS)
//{
//  return uS * 2;
//}
//uint16_t OP_Servos::Ticks_to_uS(uint16_t Ticks)
//{
//  return Ticks / 2;
//}
















