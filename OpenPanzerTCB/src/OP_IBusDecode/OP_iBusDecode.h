/* OP_iBusDecode.h  Open Panzer iBus Decoder - a library for decoding FlySky iBus serial radio data
 * Source:          openpanzer.org              
 * Authors:         Luke Middleton
 *
 * This library owes much to the work of other individuals: 
 * Povl H. Pedersen     iBus2PPM            https://github.com/povlhp                                 (08-Mar-2016)
 * Dave Borthwick       ibus_pic12f1572     https://bitbucket.org/daveborthwick/ibus_pic12f1572       (05-Dec-2015) 
 * 
 * This library reads the 32 byte iBus protocol and converts the data into 14 analog channels. Although the iBus protocol supports 14 channels, 
 * as of mid-2016 FlySky has yet to release a radio that will more than 10 (FS-i10, FS-i6S, FS-i6X - the FS-i6 can also be hacked to transmit 10 channels).
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

#ifndef OP_iBusDecode_H
#define OP_iBusDecode_H

#include <Arduino.h>
#include "../OP_Settings/OP_Settings.h"
#include "../OP_Radio/OP_RadioDefines.h"


// Uncomment only ONE of the below, to indicate which hardware serial the iBus will receive data on.
//#define iBus_SERIAL_0     0
//#define iBus_SERIAL_1     1
//#define iBus_SERIAL_2     2
#define iBus_SERIAL_3       3       // For the TCB, we want Serial 3

// Now we will save the appropriate registers for the selected serial port
// THIS IS ONLY GUARANTEED TO WORK FOR THE ATmega2560! 
#if defined (iBus_SERIAL_0)
    #define iBus_UBRRH      UBRR0H
    #define iBus_UBRRL      UBRR0L
    #define iBus_UCSRA      UCSR0A
    #define iBus_UCSRB      UCSR0B
    #define iBus_UCSRC      UCSR0C
    #define iBus_UDR        UDR0    
    #define iBus_SERIAL     Serial0
    #define iBus_PRUSART    PRUSART0
    #define iBus_RXEN       RXEN0
    #define iBus_RXC        RXC0
    #define iBus_RXCIE      RXCIE0
    #define iBus_PORT       PORTE
    #define iBus_DDR        DDRE
    #define iBus_RXPIN      PE0
#elif defined (iBus_SERIAL_1)
    #define iBus_UBRRH      UBRR1H
    #define iBus_UBRRL      UBRR1L
    #define iBus_UCSRA      UCSR1A
    #define iBus_UCSRB      UCSR1B
    #define iBus_UCSRC      UCSR1C
    #define iBus_UDR        UDR1    
    #define iBus_SERIAL     Serial1
    #define iBus_PRUSART    PRUSART1
    #define iBus_RXEN       RXEN1
    #define iBus_RXC        RXC1
    #define iBus_RXCIE      RXCIE1
    #define iBus_PORT       PORTD
    #define iBus_DDR        DDRD
    #define iBus_RXPIN      PD2
#elif defined (iBus_SERIAL_2)
    #define iBus_UBRRH      UBRR2H
    #define iBus_UBRRL      UBRR2L
    #define iBus_UCSRA      UCSR2A
    #define iBus_UCSRB      UCSR2B
    #define iBus_UCSRC      UCSR2C
    #define iBus_UDR        UDR2
    #define iBus_SERIAL     Serial2
    #define iBus_PRUSART    PRUSART2
    #define iBus_RXEN       RXEN2
    #define iBus_RXC        RXC2
    #define iBus_RXCIE      RXCIE2
    #define iBus_PORT       PORTH
    #define iBus_DDR        DDRH
    #define iBus_RXPIN      PH0
#else defined (iBus_SERIAL_3)
    #define iBus_UBRRH      UBRR3H
    #define iBus_UBRRL      UBRR3L
    #define iBus_UCSRA      UCSR3A
    #define iBus_UCSRB      UCSR3B
    #define iBus_UCSRC      UCSR3C
    #define iBus_UDR        UDR3
    #define iBus_SERIAL     Serial3
    #define iBus_PRUSART    PRUSART3
    #define iBus_RXEN       RXEN3
    #define iBus_RXC        RXC3
    #define iBus_RXCIE      RXCIE3
    #define iBus_PORT       PORTJ
    #define iBus_DDR        DDRJ
    #define iBus_RXPIN      PJ0
#endif

// iBus is 115.2k baud. Pre-calculate the UBBRn setting that will give us this baud rate.
#define UBRR_iBus                   16          // Normal mode UBRR would be 8: (F_CPU / (16*BAUD) ) -1 = (16,000,000 / (16 * 115,200) ) - 1 = 7.8 = ~8 UBRR
                                                // But according to the data sheet (pg 231), our actual baud rate is then -3.5% off of desired.
                                                // Instead we can set the USART to double-speed mode, then the formula for UBRR  becomes: 
                                                // High speed mode: (F_CPU / (8 *BAUD) ) -1 = (16,000,000 / (8  * 115,200) ) - 1 = 16.4 = ~16 UBRR
                                                // According to the datasheet our actual baud rate would then only be 2% off the desired value, so we set UBRR = 16. 
                                                // We then need to make sure we select double speed mode by setting U2Xn = 1, see begin() in OP_iBusDecode.cpp. 
                                                // See page 231 of the 2560 datasheet, or 208 for formulas
#define iBus_CHANNELS               14          // Number of iBus channels (max)
#define iBus_STARTBYTE              0x20        // First byte/sync byte of iBus frame should always be 0x20 (number of bytes in packet = 32 decimal)
#define iBus_CMDBYTE                0x40        // Second byte is the command. 0x40 indicates channel value to follow. 
#define iBus_FRAME_BYTES            32          // iBus packet is 32 bytes (2 byte header, 14 channels x 2 bytes, 2 byte checksum) = 32
                                                // byte 0				Sync byte, equals length of packet (including this byte). Should be 0x20 (decimal 32)
                                                // byte 1				Command = 0x40 indicates a string of RC channels to follow
                                                // byte 2,3				RC channel 1 = 256 * byte3 + byte2 (little endian)
                                                // byte 4,5				RC channel 2
                                                // ...					Further RC channels
                                                // byte len-2, len-1	Checksum = 0xFFFF - sum (all previous bytes) little endian
                                                        
#define iBus_ACQUISITION_COUNT      4           // Must have this many consecutive valid frames to transition to the ready state.

#define IBUS_DEFAULT_DISCARD_FRAMES 1           // If you want to only keep every N frames, set this to some number greater than 0. If 1, it will keep every other frame. 
#define IBUS_PCCOMM_DISCARD_FRAMES  2           // A new iBus frame starts every ~7.7mS which is a refresh rate of 130 Hz. 
                                                // Even half the refresh rate (65 Hz) is more than fast enough, although we do introduce some latency by skipping frames. 
                                                // But this is a tank, not a racing quad. So leave the default at 1 (every other). 
                                                // The second setting (IBUS_PCCOMM_DISCARD_FRAMES) is an optionally slower level for streaming to the PC during the Radio Setup routin. If we 
                                                // try to send even 8 channels of data every 8mS and it takes 5mS to send the packet and we need 3mS to read the next incoming 
                                                // sentence, timing becomes very tight and in testing I couldn't maintain stable comms. In fact, even discarding every other frame
                                                // it was glitchy, so we set this to 2 (keep one frame out of every 3). This is only in effect during PC streaming, not normal operation. 

// iBUS_TICKS_PER_uS is defined in OP_Settings.h
#define iBus_MIN_TICKS_BEFORE_START     (3500 * iBUS_TICKS_PER_uS)  // We set the min time between end of frame and beginning of next start byte to 3.5 mS (3500 uS)
                                                                    // In fact on the scope it seems to be a pretty consistent 4.7mS gap. 
//#define iBus_MAX_TICKS_BETWEEN_CHARS  (150  * iBUS_TICKS_PER_uS)  // NOT USED. We would need to write our own interrupt-based serial receiver, instead of using Arduino's, 
                                                                    // to implement the time-between-char check. 
class iBusDecode
{
    public:
        iBusDecode(); //Constructor
        void                    begin();
        void                    shutdown(void);                 // Turn the receiver off
        decodeState_t           getState(void);                 
        uint8_t                 getChanCount(void);             // Channel count - will always return iBus_CHANNELS
        void                    GetiBus_Frame(int16_t pulseArray[], int16_t chanCount);  // Copy a complete frame of pulses 
        static boolean          NewFrame;                       // Has an unread frame of data arrived? 
        void                    update(void);
        void                    slowDownForPCComm(void);        // Adjust on the fly how many frames we choose to discard, this will set it to IBUS_PCCOMM_DISCARD_FRAMES
        void                    defaultSpeed(void);             // Revert to the default number of discarded frames IBUS_DEFAULT_DISCARD_FRAMES
        
    private:
        static HardwareSerial   *_serial;                       // Hardware serial pointer
        static uint8_t          iBusData[iBus_FRAME_BYTES];     // Array to hold iBus frame bytes
        static uint8_t          iBus_pointer;                   // Pointer to current position of array
        static uint16_t         Pulses[iBus_CHANNELS];          // Array to hold pulse widths for all channels
    
        static uint8_t          stateCount;                     // counts the number of times this state has been repeated  
        static decodeState_t    State;                          // The current state
        static uint8_t          frameCount;                     // Used to keep track of frames for the purpose of discarding some
        static uint8_t          framesToDiscard;                // How many frames to discard for each frame we read
};


#endif 

