/* OP_PPMDecode.cpp Open Panzer PPM Decode - a library for decoding typical RC PPM streams into individual channel pulse widths
 * Source:          openpanzer.org              
 * Authors:         Michael Margolis, Luke Middleton
 *
 * This library is based on the ServoDecode library written by Michael Margolis (Arduino Forum user "MEM").
 * His original work can be found on the Arduino forum under the title "Decoding Radio Control signal pulses":
 * http://forum.arduino.cc/index.php?topic=22140.0
 *
 * However, some changes have been made. The PPM signal now is assigned to an External Interrupt pin, in this case:
 *    Arduino Pin 3, External INT1, or in terms of the Atmega chip itself, 
 *    Atmega  Pin 7, External INT5
 * 
 * The only interrupt we are now monitoring is the external pin interrupt. We don't do any timer overflows since we are not resetting TCNT1 ever. 
 * We also got rid of failsafe values. We still set a Failsafe flag, but what to do in that event is left up to the OP_Radio class. 
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
 
#include "OP_PPMDecode.h"

volatile uint16_t       PPMDecode::Ticks[MAX_PPM_CHANNELS + 1];     // Array holding the channel tick count. We have +1 since 0 will be our sync pulse, rest are channels
volatile uint8_t        PPMDecode::Channel;                         // number of channels detected so far in the frame (first channel is 1)
volatile uint8_t        PPMDecode::stateCount;                      // counts the number of times this state has been repeated  
volatile decodeState_t  PPMDecode::State;                           // The current state
volatile uint8_t        PPMDecode::NbrChannels;                     // the total number of channels detected in a complete frame
volatile uint16_t       PPMDecode::tickStamp;                       // Timestamp
volatile boolean        PPMDecode::NewFrame;                        // Boolean variable to indicate a new complete PPM frame has arrived or been read. 

// Constructor
PPMDecode::PPMDecode(){}

void PPMDecode::begin()
{

    // INITALIZE
    // ------------------------------------------------------------------------------------------------------------------------>>
    pinMode(PPM_PIN,INPUT_PULLUP);                                  // Set pin to input with internal pullup enabled
    Channel = 0;                                                    // Current channel is 0
    State = NOT_SYNCHED_state;                                      // PPM decoder not yet synched
    NewFrame = false;                                               // We haven't received a frame yet, so it hasn't been read either
    clearTicks();                                                   // Set all tick counts to 0


    // SETUP EXTERNAL INTERRUPT
    // ------------------------------------------------------------------------------------------------------------------------>>
    // EICRB (External Interrupt Control Register B)
    // Only present on chips with more than 4 external interrupts, this one controls external interrupts 4-7
    // There are two bits in the control register for each of the 4 interrupts. We are interested in external interrupt 5
    // so we are going to be setting Interrupt Sense Control bits 5-0 and 5-1 (ISC50, ISC51). These bits determine
    // the type of interrupt that is triggered. 
    // 
    // These are the possible settings: 
    // ISCn1    ISCn0   Type
    // 0        0       Interrupt on low level
    // 0        1       Any change generates interrupt
    // 1        0       Interrupt on falling edge
    // 1        1       Interrupt on rising edge

    // We tell it which setting we want through PPM_MODE which is defined in OP_PPMDecode.h. Mode can be:
    // CHANGE
    // RISING
    // FALLING
    // These are defined elsewhere as:
    // #define CHANGE   1       (01)
    // #define FALLING  2       (10)
    // #define RISING   3       (11)

    // But for our purposes, we are only interested in RISING or FALLING

    // What this line does is basically clear the two ISC5 bits, then sets them according to the selected PPM_MODE
    EICRB = (EICRB & ~((1 << ISC50) | (1 << ISC51))) | (PPM_MODE << ISC50);
    
    // They are doing an AND NOT
    // First piece: (EICRB & ~((1 << ISC50) | (1 << ISC51)))
    // This sets the two ISC50:51 bits to 1. Then it NOTs these (~ means NOT) so now both bits are zero. Then it ANDS these two bits with EICRB so in other words, 
    // the ISC50 and ISC51 bits of EICRB are now both zero. 
    // Basically, all this does is clear the bits to be doubly sure (we are doing an OR after all) 
    
    // Second piece: | (mode << ISC50)
    // This now sets the two bits to whatever is specified in PPM_MODE through an OR 


    // ENABLE EXTERNAL INTERRUPT
    // ------------------------------------------------------------------------------------------------------------------------>>
    // Ok, now that we have set up the type of interrupt, we must enable it. To turn on the external interrupt, 
    // we set the appropriate bit in the External Interrupt Mask Register (EIMSK). We are turning on external interrupt 5
    // so we SET bit 5 by ORing a 1 shifted over to the correct bit. 
    EIMSK |= (1 << INT5);
    

    // TIMER 1
    // ------------------------------------------------------------------------------------------------------------------------>>
    // PPMDecode uses Timer 1 to determine incoming pulse-widths. Timer 1 is setup in the main sketch, using a macro defined in OP_Settings.h
    byte sregRestore = SREG;        // Save interrupt register
    cli();                          // Disable interrupts
        tickStamp = TCNT1;          // Save time to start
    SREG = sregRestore;             // Restore interrupts
    
}

void PPMDecode::clearTicks()
{
    // Initialize ticks to 0 so we will be able to tell if they were ever correctly measured or not.
    for(byte chan = 0; chan<MAX_PPM_CHANNELS; chan++) 
    {
        Ticks[chan] = 0;    
    }
    // Ticks has one extra element for the sync pulse, so we need to add one more initialization
    Ticks[MAX_PPM_CHANNELS] = 0;
}

void PPMDecode::shutdown()
{   // If we end up using a different radio protocol instead, we will want to disable the interrupt function of this pin,
    // otherwise the iBus would be setting it off
    EIMSK &= ~(1 << INT5);  // Disable interrupt 5  
    EIFR |= (1 << INTF5);   // Clear any interrupt flag (clear by writing logic one to bit)
    
    // Also reset some class variables
    NbrChannels = Channel = 0;          // No channels
    State = NOT_SYNCHED_state;          // PPM decoder not synched
    NewFrame = false;                   //  No new data
    clearTicks();       
}

// This is Atmega external Interrupt 5 on Atmega2560 pin 7 (TQFP). Arduino would call it external Interrupt 1 on Arduino pin 3. But they are the same thing.
// See: Arduino\hardware\arduino\avr\cores\arduino\WInterrupts.c for the Arduino translation
ISR(INT5_vect){
    PPMDecode::INT5_PPM_ISR();
    
}

void PPMDecode::INT5_PPM_ISR()
{
    // How long since last interrupt
    uint16_t elapsedTicks = TCNT1 - tickStamp;

    // Save the current time for next go round
    tickStamp = TCNT1; 
    
    // Without an overflow, we will only go into FailSafe if the number of channels changes (highly unlikely), or if we get a pulsewidth less than 
    // the length required to be a synch pulse but not equal to a valid frame pulse. 

    if(elapsedTicks >= SYNC_GAP_TICKLEN)
    {   // Sync pulse was detected. We process the system state at the end of each frame,
        Ticks[0] = elapsedTicks;                                // Save the sync pulse length for debugging in Slot 0 
        
        if(State == READY_state) {                          
            if( Channel != NbrChannels)                         // If the number of channels is unstable, go into failsafe
            {                    
                State = FAILSAFE_state;
            }
            else                                                // In this case we have completed a full frame of channel data
            {
                NewFrame = true;
            }
        }
        else{                                                   // We're not running yet - should we be? 
            if (State == NOT_SYNCHED_state)                     // NOT_SYNCHED_STATE is the state we are initialized to. If we are here, we have detected our first sync pulse. 
            {                       
                State = ACQUIRING_state;                        // Set to ACQUIRING_state and start collecting channel data
                stateCount = 0;                                 // We keep acquiring and incrementing stateCount until we have enough valid frames to consider ourselves stable. 
            }
            else if ( State == ACQUIRING_state && Channel >= MIN_PPM_CHANNELS)  
            {                                                   // If we are in ACQUIRING_state we have been collecting channel data. We keep collecting until we have ACQUISITION_count
                if (++stateCount >= PPM_ACQUISITION_COUNT)      // number of valid frames under our belt - but we also define a valid frame as having at least 4 channels.
                {   
                    State = READY_state;                        // Ok, we have enough complete frames, we think we know what we're doing now, so let's roll! 
                    NbrChannels = Channel;                      // Save the number of channels detected from this last frame of the acquisition period. This becomes our channel count. 
                }                                               // Channel count will not change until the program is rebooted and the acquisition is run once again during startup
            }
            else if ( State == FAILSAFE_state)                  // We were in Failsafe, but if Channel == NbrChannels we read good pulses on 
            {                                                   // all channels, so we can return to READY_state (so long as we got at least 4 channels)
                if (Channel == NbrChannels && NbrChannels >= MIN_PPM_CHANNELS) 
                {   
                    State = READY_state;
                }
            }
        }
        // Whenever we have a pulse this long, it is a signal to reset the channel number
        Channel = 0;                                            // Reset the channel counter to the beginning 
    }
    // Not a sync pulse, this is a channel pulse, so add em up.
    else if(Channel < MAX_PPM_CHANNELS) 
    {                                                           // check if its a valid channel pulse and save it
        if( (elapsedTicks >= MIN_PPM_TICKLEN)  && (elapsedTicks <= MAX_PPM_TICKLEN) )   // Check for valid channel data
        { 
            Ticks[++Channel] = elapsedTicks;                    // Good pulse, save it. We'll do the math to convert Ticks to microseconds some other time, outside the ISR
        }
        else if (State == READY_state)                          // This pulse does not fall into a valid range, go to failsafe
        {                       
            State = FAILSAFE_state;                             // Use fail safe values if input data invalid  
            Channel = 0;                                        // Reset the channel count, we'll start over next round
        }
    }
}


decodeState_t PPMDecode::getState()
{
    return State;
}


uint8_t PPMDecode::getChanCount()
{
    return NbrChannels;
}


void PPMDecode::GetPPM_Frame( int16_t pulseArray[], int16_t chanCount)
{
    byte sregRestore = SREG;                                    // Save interrupt register
    cli();                                                      // Disable interrupts
    for (uint8_t i=0; i<chanCount; i++)
    {
        pulseArray[i] = Ticks[i+1] / PPM_TICKS_PER_uS;          // Divide Ticks by number of ticks in a uS to convert to uS. We add 1 to Ticks array to skip the sync pulse which we don't care about here. 
    }
    SREG = sregRestore;                                         // Restore interrupt register
    
    NewFrame = false;                                           // We've read this frame, so it's no longer new
}



