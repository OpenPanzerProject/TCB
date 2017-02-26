/* OP_PPMDecode.h   Open Panzer PPM Decode - a library for decoding typical RC PPM streams into individual channel pulse widths
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
 * This has also required some changes to MEM's failsafe code.
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

#ifndef OP_PPMDecode_H
#define OP_PPMDecode_H

#include <Arduino.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include "../OP_Settings/OP_Settings.h"
#include "../OP_Radio/OP_RadioDefines.h"


#define PPM_PIN                 3                       // Input Pin in ARDUINO format. This is actually Atmega pin 7. 
                                                        // This is also Atmega External Interrupt 5
                                                        // In Arduino, that would be External Interrupt 1

#define PPM_MODE                RISING                  // Arduino defines these:
                                                        // #define FALLING  2       (10)
                                                        // #define RISING   3       (11)

#define MIN_PPM_CHANNELS        4                       // We don't accept anything less than 4 channels, even if the signal is valid
#define MAX_PPM_CHANNELS        8                       // maximum number of channels in PPM stream we can store. This is not the same as the
                                                        // the number of channels the main sketch actually uses (see COUNT_OP_CHANNELS in OP_Radio.h)
                                                        // If your radio spits out more than 8 channels of PPM data, the extra channels will be ignored. 
                                                        
#define PPM_ACQUISITION_COUNT   6                       // Must have this many consecutive valid frames to transition to the ready state.


// PPM_TICKS_PER_uS is defined in OP_Settings.h
#define MIN_PPM_TICKLEN     (MIN_POSSIBLE_PULSE * PPM_TICKS_PER_uS)     // Minimum valid pulse width, converted to timer ticks
#define MAX_PPM_TICKLEN     (MAX_POSSIBLE_PULSE * PPM_TICKS_PER_uS)     // Maximum valid pulse width, converted to timer ticks
#define SYNC_GAP_TICKLEN    (3000 * PPM_TICKS_PER_uS)       // we assume a space at least 3000uS is sync. Some use longer, but this would be the minimum (it exceeds the maximum possible pulse)


class PPMDecode
{
    public:
        PPMDecode(); //Constructor
        void                            begin();
        void                            shutdown();                     // Disable the PPM receiver
        void                            clearTicks();   
        decodeState_t                   getState();                     // Get state Function
        void                            GetPPM_Frame(int16_t pulseArray[], int16_t chanCount); // Get a full, complete frame of pulses. 
        uint8_t                         getChanCount();                 // Returns the number of channels in a full frame
        static void                     INT5_PPM_ISR(void);             // The actual ISR will call this public member function, in order that it can access class variables
        static volatile boolean         NewFrame;                       // Has an unread frame of data arrived? 
        
    private:
        static volatile uint16_t        Ticks[MAX_PPM_CHANNELS + 1];    // Array holding the channel tick count. We have +1 since 0 will be our sync pulse, rest are channels
        static volatile uint8_t         Channel;                        // number of channels detected so far in the frame (first channel is 1)
        static volatile uint8_t         stateCount;                     // Counts the number of times this state has been repeated  
        static volatile decodeState_t   State;                          // The current state
        static volatile uint8_t         NbrChannels;                    // the total number of channels detected in a complete frame
        static volatile uint16_t        tickStamp;  
};


#endif 

