/* OP_SBusDecode.h  Open Panzer SBus Decoder - a library for decoding FrSky/Futaba S.Bus serial radio data
 * Source:          openpanzer.org              
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

#ifndef OP_SBusDecode_H
#define OP_SBusDecode_H

#include <Arduino.h>
#include "../OP_Settings/OP_Settings.h"
#include "../OP_Radio/OP_RadioDefines.h"


// Uncomment only ONE of the below, to indicate which hardware serial the SBus will receive data on.
//#define SBUS_SERIAL_0     0
//#define SBUS_SERIAL_1     1
//#define SBUS_SERIAL_2     2
#define SBUS_SERIAL_3       3

// Now we will save the appropriate registers for the selected serial port
// THIS IS ONLY GUARANTEED TO WORK FOR THE ATmega2560! 
#if defined (SBUS_SERIAL_0)
    #define SBUS_UBRRH      UBRR0H
    #define SBUS_UBRRL      UBRR0L
    #define SBUS_UCSRA      UCSR0A
    #define SBUS_UCSRB      UCSR0B
    #define SBUS_UCSRC      UCSR0C
    #define SBUS_UDR        UDR0    
    #define SBUS_SERIAL     Serial0
    #define SBUS_PRUSART    PRUSART0
    #define SBUS_RXEN       RXEN0
    #define SBUS_RXC        RXC0
    #define SBUS_RXCIE      RXCIE0
    #define SBUS_PORT       PORTE
    #define SBUS_DDR        DDRE
    #define SBUS_RXPIN      PE0
#elif defined (SBUS_SERIAL_1)
    #define SBUS_UBRRH      UBRR1H
    #define SBUS_UBRRL      UBRR1L
    #define SBUS_UCSRA      UCSR1A
    #define SBUS_UCSRB      UCSR1B
    #define SBUS_UCSRC      UCSR1C
    #define SBUS_UDR        UDR1    
    #define SBUS_SERIAL     Serial1
    #define SBUS_PRUSART    PRUSART1
    #define SBUS_RXEN       RXEN1
    #define SBUS_RXC        RXC1
    #define SBUS_RXCIE      RXCIE1
    #define SBUS_PORT       PORTD
    #define SBUS_DDR        DDRD
    #define SBUS_RXPIN      PD2
#elif defined (SBUS_SERIAL_2)
    #define SBUS_UBRRH      UBRR2H
    #define SBUS_UBRRL      UBRR2L
    #define SBUS_UCSRA      UCSR2A
    #define SBUS_UCSRB      UCSR2B
    #define SBUS_UCSRC      UCSR2C
    #define SBUS_UDR        UDR2
    #define SBUS_SERIAL     Serial2
    #define SBUS_PRUSART    PRUSART2
    #define SBUS_RXEN       RXEN2
    #define SBUS_RXC        RXC2
    #define SBUS_RXCIE      RXCIE2
    #define SBUS_PORT       PORTH
    #define SBUS_DDR        DDRH
    #define SBUS_RXPIN      PH0
#else defined (SBUS_SERIAL_3)
    #define SBUS_UBRRH      UBRR3H
    #define SBUS_UBRRL      UBRR3L
    #define SBUS_UCSRA      UCSR3A
    #define SBUS_UCSRB      UCSR3B
    #define SBUS_UCSRC      UCSR3C
    #define SBUS_UDR        UDR3
    #define SBUS_SERIAL     Serial3
    #define SBUS_PRUSART    PRUSART3
    #define SBUS_RXEN       RXEN3
    #define SBUS_RXC        RXC3
    #define SBUS_RXCIE      RXCIE3
    #define SBUS_PORT       PORTJ
    #define SBUS_DDR        DDRJ
    #define SBUS_RXPIN      PJ0
#endif

// SBus is 100k baud. Pre-calculate the UBBRn setting that will give us this baud rate.
#define UBRR_SBUS                   9           // We can get a perfect match: 
                                                // (F_CPU / (16*BAUD) ) -1 = (16,000,000 / (16 * 100,000) ) - 1 = 9
                                                // Note this is in Normal Speed Mode, so make sure you set U2Xn = 0! (See the .cpp file)
    
#define SBUS_STARTBYTE              0x0F        // First byte of an SBus frame should always be 0x0F
#define SBUS_ENDBYTE                0x00        // Last byte of an SBus frame should always be 0

#define SBUS_DIGI_CHAN1_BIT         0x01        // Bit masks for reading the two digital channel values in byte 24 of an SBus sentence
#define SBUS_DIGI_CHAN2_BIT         0x02        // Presently we are ignoring these two channels
    
#define SBUS_CHANNELS               16          // Number of SBus analog channels (SBus also has 2 digital channels, which we ignore)
#define SBUS_FRAME_BYTES            25          // There are 25 bytes in an SBus frame                                                      
                                                        
#define SBUS_ACQUISITION_COUNT      4           // Must have this many consecutive valid frames to transition to the ready state.

#define SBUS_DEFAULT_DISCARD_FRAMES 1           // If you want to only keep every N frames, set this to some number greater than 0. If 1, it will keep every other frame. 
#define SBUS_PCCOMM_DISCARD_FRAMES  1           // FrSky SBus data is sent every 9mS which is a refresh rate of 111 Hz. There is some indication FrSky may
                                                // only update the data every other frame anyway (US firmware). Even half the refresh rate (55 Hz) is more than fast enough, 
                                                // although we do introduce some latency by skipping frames. But this is tank, not a racing quad. 9/1000ths-of-a-second latency is acceptable. 
                                                // The second setting (SBUS_PCCOMM_DISCARD_FRAMES) is an optionally slower level for streaming to the PC during the Radio Setup routine. 
                                                // If we try to send even 8 channels of data every 9mS and it takes 5mS to send the packet and we need 3mS to read the next incoming 
                                                // sentence, timing becomes very tight and in testing I couldn't maintain stable comms without skipping every other. 
                                                // However for now it doesn't seem necessary to slow it down more than the default so we can leave it at 1 as well. 

// SBUS_TICKS_PER_uS is defined in OP_Settings.h
#define SBUS_MIN_TICKS_BEFORE_START     (3000 * SBUS_TICKS_PER_uS)  // Min time between end of frame and beginning of next start byte is 3 mS (3000 uS)
//#define SBUS_MAX_TICKS_BETWEEN_CHARS  (150  * SBUS_TICKS_PER_uS)  // NOT USED. Max time between subsequent bytes is 150 uS. We would need to write our own interrupt-based 
                                                                    // serial receiver, instead of using Arduino's, to implement the time-between-char check. 
class SBusDecode
{
    public:
        SBusDecode(); //Constructor
        void                    begin();
        void                    shutdown(void);                 // Turn the receiver off
        decodeState_t           getState(void);                 
        uint8_t                 getChanCount(void);             // Channel count - will always return SBUS_CHANNELS
        void                    GetSBus_Frame(int16_t pulseArray[], int16_t chanCount);  // Copy a complete frame of pulses 
        static boolean          NewFrame;                       // Has an unread frame of data arrived? 
        void                    update(void);
        void                    slowDownForPCComm(void);        // Adjust on the fly how many frames we choose to discard, this will set it to SBUS_PCCOMM_DISCARD_FRAMES
        void                    defaultSpeed(void);             // Revert to the default number of discarded frames SBUS_DEFAULT_DISCARD_FRAMES        
        
    private:
        static void             ConvertSBus_to_PWM(void);       // Convert a frame of SBus data to pulse-widths
        
        static HardwareSerial   *_serial;                       // Hardware serial pointer
        static uint8_t          SBusData[SBUS_FRAME_BYTES];     // Array to hold SBus frame bytes
        static uint8_t          sbus_pointer;                   // Pointer to current position of array
        static uint16_t         Pulses[SBUS_CHANNELS];          // Array to hold pulse widths for all channels
    
        static uint8_t          stateCount;                     // counts the number of times this state has been repeated  
        static decodeState_t    State;                          // The current state
        static uint8_t          frameCount;                     // Used to keep track of frames for the purpose of discarding some
        static uint8_t          framesToDiscard;                // How many frames to discard for each frame we read
};


#endif 

