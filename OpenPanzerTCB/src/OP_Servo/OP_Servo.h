/* OP_Servo.h       Open Panzer Servo - a library for writing servo pulses to 8 pins on a single port
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
 
#ifndef OP_Servo_H
#define OP_Servo_H
#include <Arduino.h>
#include "../OP_Settings/OP_Settings.h"

// This library is stripped down and makes several assumptions, so don't change anything unless you know exactly what you are doing! 
// Assumption 1 - we only have 8 servos
// Assumption 2 - they are all on the same Atmega port (the port can be changed in OP_Servos.cpp)
// Assumption 3 - Servos are numbered 0-7. They correspond to Pins 0-7 of the selected Port register

// If anything about those assumptions changes, this library will not work correctly, or at all! 


// The frame space is treated as an extra "servo" so this number needs to be 1 greater than the total number of actual outputs
// This library is hard-coded to 8 servos, so this number should always be 9! 
#define SERVO_OUT_COUNT         9

// Default minimum and maximum servo pulse widths. They can be modified on a per-servo basis 
// later if the user needs. As a comparison you can also see what we allow for incoming pulses in OP_RadioDefines.h
#define SERVO_OUT_MINPULSE      750
#define SERVO_OUT_MAXPULSE      2250
#define SERVO_OUT_CENTERPULSE   1500

#define SERVO_MAXRAMP_TICKSTEP  50

class OP_Servos
{
    // We are using static for everything because we only want one instance of this class. 
public:
    OP_Servos();

    // configures timer1
    static void begin();

    // Called by the timer interrupt service routine, see the cpp file for details.
    // Don't really want it public, but it has to be for the ISR to see it
    static void OCR1A_ISR();

    // called to set the pulse width for a specific channel, pulse widths are in microseconds 
    static void attach(uint8_t);
    static void detach(uint8_t);
    static boolean isAttached(uint8_t);
    static void writeMicroseconds(uint8_t, uint16_t);
    static void setFrameSpace(uint8_t, uint16_t);
    static void setMinPulseWidth(uint8_t, uint16_t);
    static void setMaxPulseWidth(uint8_t, uint16_t);
    static uint16_t getMinPulseWidth(uint8_t);
    static uint16_t getMaxPulseWidth(uint8_t);
    static uint16_t getPulseWidth(uint8_t);
    static void setRampSpeed_mS(uint8_t, uint16_t, uint8_t);
    static void setRampStepPerFrame(uint8_t, int16_t);
    static void stopRamping(uint8_t);
    static void setupRecoil_mS(uint8_t, uint16_t, uint16_t, boolean);       // Setup recoil parameters, pass ramping speed in mS
    static void StartRecoil(uint8_t);   // Kick off a recoil event
    static void setRecoilReversed(uint8_t, boolean);
    
protected:
    class PortPin
    {   public:
            uint8_t  PinMask;
            uint16_t NumTicks;          // Current pulse width in timer ticks
            uint16_t MaxTicks;          // Maximum pulse width in ticks
            uint16_t MinTicks;          // Minimum pulse width in ticks
            boolean  Enabled;           // Is this servo enabled (attached)
            int16_t  TickStep;          // Used for slowly ramping a servo from one position to another
            uint8_t  RecoilState;       // Special flag for recoil effect
            uint32_t RecoilStartTime;   // Time when recoil action starts
            uint16_t RecoilTime_mS;     // How long to wait for the recoil action to complete
            uint16_t RecoiledNumTicks;  // The full-back position of the recoiled servo, in ticks. Will equal either MaxTicks or MinTicks depending on if the servo is reversed.
            int16_t  RecoilTickStep_Return; // Specific tick step for slowly returning the barrel to starting position after recoil kick
    };

    static boolean initialized; 
    static void setPinHigh(uint8_t) __attribute__((always_inline));
    static void setPinLow(uint8_t) __attribute__((always_inline));    
    static void setPulseWidthTimer(uint8_t) __attribute__((always_inline));
    static void setPulseWidthTimer_Ramp(uint8_t);
    
    // Information about each channel
    static volatile PortPin Channel[SERVO_OUT_COUNT]; 
    
    // current output channel
    static volatile uint8_t CurrentChannel;    

    // Moved to OP_Settings.h as defines
    // Convert microseconds to timer ticks 
    //static uint16_t uS_to_Ticks(uint16_t) __attribute__((always_inline));
    //static uint16_t Ticks_to_uS(uint16_t) __attribute__((always_inline));

private:    
    // Remember, static variables must be initialized outside the class

};



#endif

