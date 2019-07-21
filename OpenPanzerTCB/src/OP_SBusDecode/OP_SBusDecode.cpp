/* OP_SBusDecode.cpp Open Panzer SBus Decoder - a library for decoding FrSky/Futaba S.Bus serial radio data
 * Source:           openpanzer.org             
 * Authors:          Luke Middleton
 *
 * This library owes much to the work of three other individuals: 
 * Mike Blandford (MikeB)       SBusToPPM:      http://openrcforums.com/forum/viewtopic.php?f=91&t=5476&start=30                    (04-Mar-2015)
 * Michel (mstrens)             CPPM_SBUS:      http://openrcforums.com/forum/viewtopic.php?f=84&t=6361&start=30#p92825             (09-Feb-2015)
 * Uwe Gartmann                 SBUS-Library:   https://developer.mbed.org/users/Digixx/notebook/futaba-s-bus-controlled-by-mbed/   (09-Mar-2012)
 * 
 * This library reads the 25 byte SBus protocol and converts the data into 16 analog channels. An additional 2 digital (on/off) channels are specified in the
 * protocol and could be implemented but presently are ignored.
 *
 * The SBus protocol is 100,000 baud inverted serial. After undoing the inversion in hardware, we can read the data stream using a common serial port. 
 * Ironically because Arduino appropriates the hardware serial interrupt ISR(USART3_RX_vect), this library needs to be polled in order to update 
 * (unlike the PPM decoder library that is based entirely on interrupts and updates itself as data comes in). The one offseting advantage is 
 * that the Arduino hardware serial library implements a convenient 64 byte buffer. We could probably make a better implementation of this library if we 
 * appropriated the ISR ourselves (specifically see Uwe Gartmann's library above, the other two implement polling like this one). But, we would like to keep
 * this compatible with Arduino, and this approach seems to work ok but does require some extra work in the OP_Radio class. 
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
 
#include "OP_SBusDecode.h"

HardwareSerial        * SBusDecode::_serial;                            // Hardware serial port
uint8_t                 SBusDecode::SBusData[SBUS_FRAME_BYTES];         // 25 bytes in an SBus frame
uint8_t                 SBusDecode::sbus_pointer;                       // Pointer to SBusData array
uint16_t                SBusDecode::Pulses[SBUS_CHANNELS];              // 16 channel pulse widths
uint8_t                 SBusDecode::stateCount;                         // counts the number of times this state has been repeated  
decodeState_t           SBusDecode::State;                              // The current state
boolean                 SBusDecode::NewFrame;                           // Boolean variable to indicate a new complete PPM frame has arrived or been read. 
uint8_t                 SBusDecode::frameCount;                         // 
uint8_t                 SBusDecode::framesToDiscard;                    // How many frames to skip for every one read

// Constructor
SBusDecode::SBusDecode(){}

void SBusDecode::begin()
{
    // According to here: https://developer.mbed.org/users/Digixx/notebook/futaba-s-bus-controlled-by-mbed/
    // The signal is sent every 14mS or 7mS in highspeed mode (~70-140 Hz). 
    // A frame is 25 bytes, each byte is 12 bits long (including stop bits, etc.). At 100k baud, that means a frame takes exactly 3mS (scope shows very close to that). 
    // Therefore in highspeed mode there would 4mS inactivity after each frame, or in analog mode, 11mS. 
    // But a scope of an FrSky X4R shows a gap of 6mS. 
    // The mbed library sets an interrupt on serial reception, when the first byte is received, it essentially waits 1mS and then starts to decode the frame. 
    // MikeB library uses polling. mstrens library also uses polling, but he checks if the first byte of a frame arives less than 3mS after the end of the last one 
    // and throws an error if so. He also throws an error if there is *more* than 3mS between subsequent chars (but there should be a lot less than that). 
    // A guy in the discussion section of the mbed page checks for less than 4mS between frames and no more than 150uS between subsequent chars.
    // Because we are using the Arduino hardware serial port and reading out of their buffer (not directly from the USART Rx registers), we can't effectively check
    // the time between bytes within the stream, but we can check for the time between transmissions. 
    
    // Set Rx pin to input
    SBUS_DDR &= ~(1 << SBUS_RXPIN);         // Input is selected when Data DiRection bit is cleared
    // Set pullup
    SBUS_PORT |= (1 << SBUS_RXPIN);         // Pullups selected when port pin bit set

    // Initialize Arduino Serial port
    _serial = &SBUS_SERIAL;
    _serial->begin(100000, SERIAL_8E2); // 100k baud, 8 data bits, even parity, 2 stop bits

    // But because we also like to do things manually for educational purposes, here is the explicit setup:

    // Make sure power reduction hasn't turned off this serial port
    PRR1 &= ~(1 << SBUS_PRUSART);
    //Set baud rate
    SBUS_UBRRH = (unsigned char)(UBRR_SBUS>>8);
    SBUS_UBRRL = (unsigned char)UBRR_SBUS;
    // Set frame format 
    SBUS_UCSRC = 0x2E;                  // mode normal (asynchronous USART), even parity , 2 stop bit, 8 bits data, no polarity for asynchronous
    
    // Clear flags, set normal speed mode, turn off multi-processor communication mode
    SBUS_UCSRA = 0xFC;                  // Clear flags (by writing 1 to them), but leave U2Xn = 0 because we don't want double speed mode, we can get a perfect 100k baud rate in normal speed mode with UBRR = 9.
                                        // Also turn off multi-processor communication mode
    
    // Enable receiver only  
    SBUS_UCSRB = 0x90;                  // Rx interrupt enabled, Rx enabled, TX disabled, only 8 bits

    // Timer 1 
    // SBusDecode uses Timer 1 Compare C (OCR1C). Timer 1 is setup in the main sketch, using a macro defined in OP_Settings.h
    // We can set OCR1C to some time in the future, and the OCF1C flag (Output Compare match flag) of the TIFR1 (Timer 1 Interrupt Flag Register)
    // will get set when that time is reached. We can use this flag to determine if the timing of our incoming pulse stream is correct. 
    // Note: you could also set an interrupt to occur when this flag is set, which is a frequent application, but we do not need it here! 
    // NOTE: Even worse, if you enable the output compare interrupt but do not specify an ISR, the IDE will NOT warn you about it, but will go off
    // and choose an ISR maybe defined somewhere else in some unrelated library or who knows, and you will get all kinds of random and inexplicable behavior!
    // Don't ask us how many countless hours we spent trying to troubleshoot this one! 

    // Other initializations
    State = NOT_SYNCHED_state;                          // Decoder not yet synched
    stateCount = 0;                                     // Repeated a state 0 times
    frameCount = 0;                                     // 0 frames received
    NewFrame = false;                                   // We haven't received a frame yet, so it hasn't been read either

    // Initialize pulses to Center for safety
    for(uint8_t i = 0; i<SBUS_CHANNELS; i++) 
    {
        Pulses[i] = DEFAULT_PULSE_CENTER;       
    }
    
    // Set the number of frames to skip to the default
    defaultSpeed();
}

void SBusDecode::shutdown()
{   // If we end up using PPM input instead, we will want to disable the serial function of this pin,
    // otherwise the PPM could be setting it off
    _serial->end();
    SBUS_UCSRB &= ~(1 << SBUS_RXCIE);   // Disable receive interrupts
    SBUS_UCSRB &= ~(1 << SBUS_RXEN);    // Disable receiver
    SBUS_UCSRA = 0x00;                  // Clear all interrupt flags
}


// Read all received data and calculate channel data
void SBusDecode::update()
{
uint8_t b;
boolean TimeFlag;
uint8_t UART_error;
    
    while (_serial->available())
    {
        UART_error = SBUS_UCSRA & 0x1C;             // Save error
        b = _serial->read();                        // Get data from serial Rx 
        TimeFlag = TIFR1 & (1 << OCF1C );           // Save the  Compare C flag. If 1, it means our set amount of time has been exceeded since last char. 
                                                    // This may be good or bad depending, we will check below.
        TIFR1 |= (1 << OCF1C );                     // Reset the compare flag 
        OCR1C = TCNT1 + SBUS_MIN_TICKS_BEFORE_START;    // Flag again 3mS from now
    
        if ( UART_error )   
        {   
            sbus_pointer = 0;                       // If there is a receive error, reset the frame
            State = FAILSAFE_state;                 // Set state to Failsafe
        }
        else    
        { 
            if ( sbus_pointer == 0 )                // first char    
            { 
                if  ( TimeFlag && b == SBUS_STARTBYTE )         
                {   // If there is *more* than 3 msec since previous char (TimeFlag = true, which in this case is good), 
                    // and the first char equals the start byte, we save the byte and increment the pointer 
                    SBusData[sbus_pointer++] = b;
                    
                    if (State == NOT_SYNCHED_state) 
                    {                               // NOT_SYNCHED_STATE is the state we are initialized to. That means this is our first start byte detected.
                        State = ACQUIRING_state;    // Set state to ACQUIRING and start collecting channel data.
                        stateCount = 0;             // We keep acquiring and incrementing stateCount until we have enough valid frames to consider ourselves stable. 
                    }
                }
            }
            else                                    // not first char
            {
                if ( TimeFlag ) 
                {   // If there is *more* than 3mS since previous char (TimeFlag = true, which in this case is bad), 
                    // reset the count and start looking for start byte again. 
                    sbus_pointer = 0 ;              // Reset the frame
                    stateCount = 0;
                    if (State == READY_state) State = FAILSAFE_state;         // Set state to Failsafe if we were previously connected, otherwise leave in existing state (acquiring or not synched). 
                }
                else 
                {
                    // Save the byte and increment pointer
                    SBusData[sbus_pointer++] = b ;

                    if ( sbus_pointer == SBUS_FRAME_BYTES )     // We've reached the end (we hope)
                    {   
                        if (  b == SBUS_ENDBYTE )               // Valid final byte received
                        {
                            // We've received a full frame, but did SBus report an error? 
                            if (SBusData[23] & (1<<2)) 
                            {   // SBus signal lost
                                State = FAILSAFE_state; 
                            }
                            else if (SBusData[23] & (1<<3)) 
                            {   // SBus signal failsafe
                                State = FAILSAFE_state; 
                            }   
                            else    // No SBus error                        
                            {
                                if ( State == ACQUIRING_state)  
                                {   // If we are in ACQUIRING_state we have been collecting channel data. We keep collecting until we have ACQUISITION count of frames under our belt.
                                    // We are only in Acquiring state once - when the program first boots. After that we will only be either READY or FAILSAFE
                                    if(++stateCount >= SBUS_ACQUISITION_COUNT) 
                                    {
                                        State = READY_state;    // Ok, we have enough complete frames, we think we know what we're doing now, so let's roll! 
                                    }       
                                }
                                else
                                {
                                    State = READY_state;        // Valid frame, keep at Ready
                                    stateCount = 0;             // reset
                                }

                                if (++frameCount > framesToDiscard)
                                {
                                    // Set the new frame flag
                                    NewFrame = true;
                                    // Convert SBus data to individual channel pulse-widths
                                    ConvertSBus_to_PWM();
                                    frameCount = 0;
                                }
                                else
                                {
                                    NewFrame = false; 
                                }
                            }
                        }
                        
                        // Regardless of what the outcome was, we reached SBUS_FRAME_BYTES, so reset the frame
                        sbus_pointer = 0;
                   }
                }   
            }
        }
    }
}

void SBusDecode::ConvertSBus_to_PWM()
{
    // Convert SBus frame to 16 channel pulse-widths.
    
    // This clever bit is taken from Mike Blandford's SbusToPpm code from 04-Mar-2015
    // http://openrcforums.com/forum/viewtopic.php?f=91&t=5476&start=30
    
    // Another interesting option would be a large PROGMEM lookup table. 
    // It would take ~4Kb but we have plenty of program space. Theoretically it would be faster,
    // though by how much I have not tested. I was waiting to see if I would have a speed problem, 
    // and so far we don't. 
    
    uint8_t inputbitsavailable = 0;
    uint32_t inputbits = 0;
    uint8_t *sbus = SBusData;
    sbus++;    // Skip start byte
    for ( uint8_t i = 0 ; i < SBUS_CHANNELS ; i += 1 )
    {
        uint16_t temp;
        while ( inputbitsavailable < 11 )
        {   
            inputbits |= (uint32_t)*sbus++ << inputbitsavailable;
            inputbitsavailable += 8;
        }
        
        // Convert the SBUS data (0-2047) to PWM pulse widths (~884 ~2160 uS)
        temp = ( (int16_t)( inputbits & 0x7FF ) - 0x3E0 ) * 5 / 8 + 1500;
        
        // If the pulse is valid, save it. Otherwise the result is that we keep the last value. See OP_RadioDefines.h for min and max pulsewidths.
        if (( temp > MIN_POSSIBLE_PULSE) && (temp < MAX_POSSIBLE_PULSE)) { Pulses[i] = temp; }
        
        inputbitsavailable -= 11 ;
        inputbits >>= 11 ;
    }
    
    // PRESENTLY WE IGNORE THE TWO DIGITAL CHANNELS - but if you wanted them, this is how you'd read them.
    // Of course, you'd also have to create a channel 17 & 18 because right now we only go up to 16.
    // Digital Channels 1 & 2
    // SBusData[23] & SBUS_DIGI_CHAN1_BIT ? channels[16] = 1 : channels[16] = 0;
    // SBusData[23] & SBUS_DIGI_CHAN2_BIT ? channels[17] = 1 : channels[17] = 0;
}

void SBusDecode::GetSBus_Frame( int16_t pulseArray[], int16_t chanCount)
{
    // This copies as many channels are requested from an SBusData frame to the array that is passed as a parameter
    byte sregRestore = SREG;        // Save interrupt register
    cli();                          // Disable interrupts
    for (uint8_t i=0; i<chanCount; i++)
    {
        pulseArray[i] = Pulses[i];          
    }
    SREG = sregRestore;             // Restore interrupt register
    
    NewFrame = false;               // We've read this frame, so it's no longer new
}

decodeState_t SBusDecode::getState()
{
    return State;
}

uint8_t SBusDecode::getChanCount()
{
    return SBUS_CHANNELS;
}

void SBusDecode::slowDownForPCComm(void)
{
    // Discard even more frames so that streaming data to the PC doesn't bog down
    framesToDiscard = SBUS_PCCOMM_DISCARD_FRAMES;
}

void SBusDecode::defaultSpeed(void)
{
    // Revert to the default number of discarded frames DEFAULT_DISCARD_FRAMES,
    // this gives us maximum responsiveness when in normal operation
    framesToDiscard = SBUS_DEFAULT_DISCARD_FRAMES;    
}

