/* OP_iBusDecode.h  Open Panzer iBus Decoder - a library for decoding FlySky iBus serial radio data
 * Source:          openpanzer.org              
 * Authors:         Luke Middleton
 *
 * This library owes much to the work of other individuals: 
 * Povl H. Pedersen     iBus2PPM            https://github.com/povlhp                                 (08-Mar-2016)
 * Dave Borthwick       ibus_pic12f1572     https://bitbucket.org/daveborthwick/ibus_pic12f1572       (05-Dec-2015) 
 * 
 * This library reads the 32 byte iBus protocol and converts the data into 14 analog channels. Although the iBus protocol supports 14 channels, 
 * as of mid-2016 FlySky has yet to release a radio that will more than 10 (FS-i10, FS-i6S - the FS-i6 can also be hacked to transmit 10 channels).
 * They announced an 18 channel transmitter several years ago but it has not yet materialized. 
 *
 * The iBus protocol is 115,200 baud serial. That is a standard baud rate and the signal is not inverted so no hardware is needed, just connect to any UART. 
 * Ironically because Arduino appropriates the hardware serial interrupt ISR(USART3_RX_vect), this library needs to be polled in order to update 
 * (unlike the PPM decoder library that is based entirely on interrupts and updates itself as data comes in). The one offseting advantage is 
 * that the Arduino hardware serial library implements a convenient 64 byte buffer. We could probably make a better implementation of this library if we 
 * appropriated the ISR ourselves. But, we would like to keep this compatible with Arduino, and this approach seems to work ok but does require some extra work 
 * in the OP_Radio class.
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
 
#include "OP_iBusDecode.h"

HardwareSerial        * iBusDecode::_serial;                            // Hardware serial port
uint8_t                 iBusDecode::iBusData[iBus_FRAME_BYTES];         // 25 bytes in an iBus frame
uint8_t                 iBusDecode::iBus_pointer;                       // Pointer to iBusData array
uint16_t                iBusDecode::Pulses[iBus_CHANNELS];              // 16 channel pulse widths
uint8_t                 iBusDecode::stateCount;                         // counts the number of times this state has been repeated  
decodeState_t           iBusDecode::State;                              // The current state
boolean                 iBusDecode::NewFrame;                           // Boolean variable to indicate a new complete PPM frame has arrived or been read. 
uint8_t                 iBusDecode::frameCount;                         // 
uint8_t                 iBusDecode::framesToDiscard;                    // How many frames to skip for every one read

// Constructor
iBusDecode::iBusDecode(){}


void iBusDecode::begin()
{
    // A frame is 32 bytes, each byte is the standard 8 bits send at 115.2k baud, non inverted. Standard stuff, very simple and nice. 
    // A scope indicates the iBus frame takes roughly 3mS for all 32 bytes. A new frame is sent every ~4.7mS, meaning a new frame starts every 7.7mS. 
    // This results in a refresh rate of 130 Hz. 
   
    // Set Rx pin to input
    iBus_DDR &= ~(1 << iBus_RXPIN);         // Input is selected when Data DiRection bit is cleared
    // Set pullup
    iBus_PORT |= (1 << iBus_RXPIN);         // Pullups selected when port pin bit set

    // Initialize Arduino Serial port
    _serial = &iBus_SERIAL;
    _serial->begin(115200, SERIAL_8N1);     // 115.2k baud, 8 data bits, no parity, 1 stop bit

    // But because we also like to do things manually for educational purposes, here is the explicit setup:

    // Make sure power reduction hasn't turned off this serial port
    PRR1 &= ~(1 << iBus_PRUSART);
    //Set baud rate
    iBus_UBRRH = (unsigned char)(UBRR_iBus>>8);
    iBus_UBRRL = (unsigned char)UBRR_iBus;
    // Set frame format 
    // 00 - Asynchronous
    //   00 - No parity
    //     0 - 1 stop bit
    //      11 - 8 bits data
    //        0 - clock polarity, keep at 0 when in asynchronous mode
    // 00000110 = 0x06
    iBus_UCSRC = 0x06;                  // mode normal (asynchronous USART), no parity , 1 stop bit, 8 bits data, no polarity for asynchronous
    
    // Clear flags, set double speed mode, disable multi-processor mode
    iBus_UCSRA = 0xFE;                  // Flags are cleared by writing 1. The only other bits to worry about are: 
                                        // U2Xn - but we want to set that to 1 as well, for double USART speed. The double-speed mode lets us get closer to our desired baud rate.
                                        // MPCMn - multiprocessor communication mode, we want this off (0)
    // Enable receiver only  
    iBus_UCSRB = 0x90;                  // Rx interrupt enabled, Rx enabled, TX disabled, only 8 bits

    // Timer 1 
    // iBusDecode uses Timer 1 Compare C (OCR1C). Timer 1 is setup in the main sketch, using a macro defined in OP_Settings.h
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
    for(uint8_t i = 0; i<iBus_CHANNELS; i++) 
    {
        Pulses[i] = DEFAULT_PULSE_CENTER;       
    }
    
    // Start frame at zero
    iBus_pointer = 0;
    
    // Set the number of frames to skip to the default
    defaultSpeed();
}

void iBusDecode::shutdown()
{   // If we end up using PPM input instead, we will want to disable the serial function of this pin
    // otherwise the PPM could be setting it off
    _serial->end();
    iBus_UCSRB &= ~(1 << iBus_RXCIE);   // Disable receive interrupts
    iBus_UCSRB &= ~(1 << iBus_RXEN);    // Disable receiver
    iBus_UCSRA = 0x00;                  // Clear all interrupt flags
}


// Read all received data and calculate channel data
void iBusDecode::update()
{
uint8_t b;
boolean TimeFlag;
uint8_t UART_error;

uint16_t chksum;
uint16_t rxsum;

uint8_t i;
uint8_t offset;
uint16_t temp;
    
    
    while (_serial->available())
    {
        UART_error = iBus_UCSRA & 0x1C;                 // Save error
        b = _serial->read();                            // Get data from serial Rx 
        TimeFlag = TIFR1 & (1 << OCF1C );               // Save the  Compare C flag. If 1, it means our set amount of time has been exceeded since last char. 
                                                        // This may be good or bad depending, we will check below.
        TIFR1 |= (1 << OCF1C );                         // Reset the compare flag 
        OCR1C = TCNT1 + iBus_MIN_TICKS_BEFORE_START;    // Flag again 3.5mS from now
    
        if ( UART_error )   
        {   
            iBus_pointer = 0;                           // If there is a receive error, reset the frame
            State = FAILSAFE_state;                     // Set state to Failsafe
        }
        else
        { 
             if ( iBus_pointer == 0 )                    // First byte
            { 
                if ( TimeFlag && b == iBus_STARTBYTE )
                {   // If there is *more* than 3.5 msec since previous byte (TimeFlag = true, which in this case is good), 
                    // and the first byte equals the start byte, we save the byte and increment the pointer 
                    iBusData[iBus_pointer++] = b;
                    
                    if (State == NOT_SYNCHED_state) 
                    {                                   // NOT_SYNCHED_STATE is the state we are initialized to. That means this is our first start byte detected.
                        State = ACQUIRING_state;        // Set state to ACQUIRING and start collecting channel data.
                        stateCount = 0;                 // We keep acquiring and incrementing stateCount until we have enough valid frames to consider ourselves stable. 
                    }
                }
            }
            else                                        // Second byte or more 
            {
                if ( TimeFlag ) 
                {   // If there is *more* than 3.5mS since previous byte (TimeFlag = true, which in this case is bad), 
                    // reset the count and start looking for start byte again. 
                    iBus_pointer = 0 ;                  // Reset the frame
                    State = FAILSAFE_state;             // Set state to Failsafe
                }
                else                                    // Second byte
                {                                       
                    if  (iBus_pointer == 1 && b == iBus_CMDBYTE )         
                    {   // If the second byte equals the command byte, save it and increment the pointer. After this should be channel data. 
                        iBusData[iBus_pointer++] = b;

                        if (State == NOT_SYNCHED_state) 
                        {                               // NOT_SYNCHED_STATE is the state we are initialized to. That means this is our first start byte detected.
                            State = ACQUIRING_state;    // Set state to ACQUIRING and start collecting channel data.
                            stateCount = 0;             // We keep acquiring and incrementing stateCount until we have enough valid frames to consider ourselves stable. 
                        }
                    }
                    else if (iBus_pointer > 1)          // Third byte or more
                    {
                        // Save the byte and increment pointer
                        iBusData[iBus_pointer++] = b ;
                        
                        if ( iBus_pointer == iBus_FRAME_BYTES)   // We've reached the end
                        {   
                            // Regardless of what the outcome is, we reached iBus_FRAME_BYTES, so reset the frame for the next byte
                            iBus_pointer = 0;
                            
                            // Now decide if this a valid frame.  
                            
                            // Calculate checksum from received data (first 30 bytes)
                            chksum = 0xFFFF;
                            for (i = 0; i < 30; i++)
                            {   chksum -= iBusData[i]; }

                            // Combine the last two bytes of the packet to get the checksum sent
                            rxsum = iBusData[30] + (iBusData[31] << 8);

                            // Compare the calculated checksum against the checksum that was sent in the packet
                            if (chksum != rxsum) 
                            {   // iBus signal failed
                                State = FAILSAFE_state; 
                            }
                            else    // No iBus error, checksums match                        
                            {                       
                                if ( State == ACQUIRING_state)  
                                {   // If we are in ACQUIRING_state we have been collecting channel data. We keep collecting until we have ACQUISITION count of frames under our belt.
                                    // We are only in Acquiring state once - when the program first boots. After that we will only be either READY or FAILSAFE
                                    if(++stateCount >= iBus_ACQUISITION_COUNT) 
                                    {
                                        State = READY_state;    // Ok, we have enough complete frames, we think we know what we're doing now, so let's roll! 
                                    }       
                                }
                                else
                                {
                                    State = READY_state;        // Valid frame, keep at Ready
                                }

                                if (++frameCount > framesToDiscard)
                                {
                                    // Set the new frame flag
                                    NewFrame = true;
                                    
                                    // Convert iBus data to individual channel pulse-widths
                                    
                                    // offset = 2 because we are skipping the first two bytes (start and command)
                                    // Each time through we increment offset by 2 because we are concatenating two bytes for each channel's data
                                    for (i = 0, offset = 2; i < iBus_CHANNELS; i++, offset += 2) 
                                    {
                                        temp = iBusData[offset] + (iBusData[offset + 1] << 8);
                                        // If the pulse is valid, save it. Otherwise the result is that we keep the last value. See OP_RadioDefines.h for min and max pulsewidths.
                                        if (( temp > MIN_POSSIBLE_PULSE) && (temp < MAX_POSSIBLE_PULSE)) { Pulses[i] = temp; }
                                    }
                                    
                                    // Reset frameCount
                                    frameCount = 0;
                                }
                                else
                                {   // Discard frame
                                    NewFrame = false; 
                                }
                            }                    
                        }
                    }
                }   
            }
        }
    }
}

void iBusDecode::GetiBus_Frame( int16_t pulseArray[], int16_t chanCount)
{
    // This copies as many channels as are requested from an iBusData frame to the array that is passed as a parameter
    byte sregRestore = SREG;        // Save interrupt register
    cli();                          // Disable interrupts
    for (uint8_t i=0; i<chanCount; i++)
    {
        pulseArray[i] = Pulses[i];          
    }
    SREG = sregRestore;             // Restore interrupt register
    
    NewFrame = false;               // We've read this frame, so it's no longer new
}

decodeState_t iBusDecode::getState()
{
    return State;
}

uint8_t iBusDecode::getChanCount()
{
    return iBus_CHANNELS;
}

void iBusDecode::slowDownForPCComm(void)
{
    // Discard even more frames so that streaming data to the PC doesn't bog down
    framesToDiscard = IBUS_PCCOMM_DISCARD_FRAMES;
}

void iBusDecode::defaultSpeed(void)
{
    // Revert to the default number of discarded frames DEFAULT_DISCARD_FRAMES,
    // this gives us maximum responsiveness when in normal operation
    framesToDiscard = IBUS_DEFAULT_DISCARD_FRAMES;    
}

