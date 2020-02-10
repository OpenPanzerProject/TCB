/* OP_Driver.cpp    Open Panzer Driver - a library of driving functions
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


#include "OP_Driver.h"
 
// Static variables must be initialized outside the class 
// Driving
uint8_t             OP_Driver::DriveType;
// Track recoil
uint8_t             OP_Driver::KickbackSpeed;
float               OP_Driver::DecelerationFactor;
uint8_t 		    OP_Driver::TrackRecoilDuration;
uint32_t			OP_Driver::TrackRecoilStartTime;
// Drive speed ramping
boolean             OP_Driver::DriveRampEnabled;
volatile uint16_t   OP_Driver::RampedDriveSpeed;
volatile int8_t     OP_Driver::DriveRampStep;
volatile int8_t     OP_Driver::DriveSkipNum;
volatile int8_t     OP_Driver::DriveSkipCount;
// Throttle speed ramping
boolean             OP_Driver::ThrottleRampEnabled;
volatile int16_t    OP_Driver::RampedThrottleSpeed; 
volatile int8_t     OP_Driver::ThrottleRampStep;    
volatile int8_t     OP_Driver::ThrottleSkipNum;
volatile int8_t     OP_Driver::ThrottleSkipCount;
// User settings
boolean             OP_Driver::AccelRampEnabled;
boolean             OP_Driver::DecelRampEnabled;
uint8_t             OP_Driver::AccelSkipNum;
uint8_t             OP_Driver::DecelSkipNum;
ACCEL_DRIVE_PRESET  OP_Driver::AccelPreset;
DECEL_DRIVE_PRESET  OP_Driver::DecelPreset;
// Braking
int16_t             OP_Driver::Reverse_FullStopCmd;
int16_t             OP_Driver::Forward_FullStopCmd;
// Turning
uint8_t             OP_Driver::TurnMode;
boolean             OP_Driver::NeutralTurnAllowed;  



// Little function to help us print out actual drive type names, rather than numbers. 
// To use, call something like this:  Serial.print(printDriveType(my_drive_type));
const __FlashStringHelper *printDriveType(DRIVETYPE Type) 
{
    if(Type>LAST_DT) Type=DT_UNKNOWN;
    const __FlashStringHelper *Names[LAST_DT+1]={F("UNKNOWN"),F("Tank"),F("Halftrack"),F("Car"),F("Tank (DKLM Gearbox)"),F("Tank (Tamiya DMD)"),F("Independent Tread Control")};
    return Names[Type];
};

// Little function to help us print out actual drive mode names, rather than numbers. 
// To use, call something like this:  Serial.print(printMode(DriveModeCommand));
const __FlashStringHelper *printMode(_driveModes Type) 
{
    if(Type>LAST_MODE) Type=UNKNOWN;
    const __FlashStringHelper *Names[LAST_MODE+1]={F("UNKNOWN"),F("STOP"),F("FORWARD"),F("REVERSE"),F("NEUTRAL TURN"),F("TRACK RECOIL")};
    return Names[Type];
};



// ------------------------------------------------------------------------------------------------------------------------------------------>>
// DRIVING FUNCTIONS
// ------------------------------------------------------------------------------------------------------------------------------------------>>

// CONSTRUCTOR - INITIALIZE CLASS VARIABLES
OP_Driver::OP_Driver(void) 
{
    DriveRampStep = DRIVE_RAMP_STEP_DEFAULT;
    ThrottleRampStep = DriveRampStep;   // Just initialize to the same as DriveRampStep. It will get set independently later. 
    
    DriveSkipCount = 0;
    ThrottleSkipCount = 0;  

    DriveRampEnabled = false;                       // This just initializes drive speed ramping to false, but it can be changed later depending on user settings

    ThrottleRampEnabled = true;                     // Ramping enabled for throttle. User doesn't need to mess with this one. It is used to manipulate the throttle
                                                    // signal sent to the sound unit and the smoker. 
        
    // Initialize these. They may be confusing. Assume COMPLETE_STOP_NEARLIMIT is 45. Assume MOTOR_MAX_FWDSPEED is 255. Reverse_FullStopCMD will then = 255-45 = 210
    // What this means, is if we are moving in reverse, and the user gives a *forward* command greater than 210, we will treat it as a command to come to a complete stop,
    // rather than a command to brake, which is what we will do if the command is less than 210.
    Reverse_FullStopCmd = MOTOR_MAX_FWDSPEED - COMPLETE_STOP_NEARLIMIT;
    Forward_FullStopCmd = abs(MOTOR_MAX_REVSPEED) - COMPLETE_STOP_NEARLIMIT;
}

void OP_Driver::begin(DRIVETYPE dt, uint8_t tm, boolean nta, uint8_t kbs, uint8_t dcf)
{
    DriveType = dt;                                 // Is this a tank with independent treads for steering? Or a halftrack with independent treads and also steerable front wheels?     
                                                    // Or a car with a single rear drive and steerable front wheels (also a halftrack with synchronized rear treads)
    TurnMode = tm;                                  // What turn mode to start off with
    NeutralTurnAllowed = nta;                       // Are neutral turns allowed
    
    KickbackSpeed = map(kbs, 0, 100, 0, 255);       // Kickback speed is passed as some number between 0-100, we want to scale it to 0-255
    DecelerationFactor = 0.65 + (0.0033 * (float)dcf);  // dcf will range from 0-100, what we want to end up at (DecelerationFactor) is a floating point number somewhere 
                                                    // roughly between 0.65 and 0.98. At 0.65 the kickback spike would last approximately 1/2 second (at full speed). At .98 it would last approximately 3.5 seconds.
    TrackRecoilDuration = dcf;						// We also have a simple track recoil option, which takes the value in deceleration factor and uses it as the literal number of mS for the recoil action to take

    // SET INTERRUPT FREQUENCY
    // -------------------------------------------------------------------------------------------------------------------------------------------------->    
    // Theory of operation: We create an interrupt that triggers 256 times a second. There are 256 steps possible between complete stop and full throttle. 
    // If we want to implement acceleration or deceleration constraints, we can disconnect drive speed from the user's thumb, and instead, only increase (or decrease) 
    // the speed by some small amount each time (or some number of times) that the interrupt triggers. If we increase the speed by 2 (our default "step") and we do so every time the 
    // interrupt hits, that means we would accelerate from a complete stop to full speed in 1/2 second. If we were to only increase the speed by 2 every *other* time the 
    // interrupt hits, we would take 1 full second to accelerate to full speed. In fact, at a default step increase of 2, for each number of interrupts that we skip, we increase
    // the amount of time it takes to accelerate to full speed by 1/2 second. That is why we chose a frequency of 256 hertz and a default step of 2. 
    // In the OP Config program, we give the user a choice of acceleration/deceleration constraint "levels" from 1 - 14, but these translate directly to the number of
    // interrupts to skip. At Level 1 the tank will accelerate in 1/2 second, at level 14 it will take 7 seconds. We don't go beyond 14 because even 7 seconds is probably 
    // slower than anyone wants to go. And we don't go less than 1, because faster than 1/2 second and you might as well turn off the acceleration constraint completely (which the user can do). 
    
    // We can get even more fancy if we want. We can programmatically tweak the increment "step", it doesn't always have to be 2. Higher steps mean faster acceleration (speed
    // increments more each interrupt), lower steps mean slower. And we can also programmatically modify the number of interrupts to skip, which as described will speed up or 
    // slow down the acceleration. 
    
    // Here is the code to setup an interrupt to occur 256 times a second:  
        // See the Atmega2560 datasheet, chapter 17 (16 bit timers)
        // See also The Secrets of Arduino PWM by Ken Shirriff: http://www.righto.com/2009/07/secrets-of-arduino-pwm.html
        
        // We are going to setup a recurring interrupt by using Timer 3 in CTC mode (Clear Timer on Compare match mode)
        // We will be using WGM mode 4 where the TOP value (compare match value) is OCR3A, which we can set to whatever we want (page 148)
        
        // We want a 256hz update rate (interrupt every 4mS = 0.004 seconds)
        
        // Rate will be equal to: 16 MHz (clock) / prescaler / OCR3A value
        
        // We will use a prescaler of 256, and set OCR3A to 244
        // 16mhz / 256 / 244 = 256 hz

        // Prescaler settings:
        // - On the Mega, for 16 bit timers only 1, 3, 4, and 5 (not timers 0 or 2), 
        // - using 16mHz system clock, 
        /*  
            If TCCRnB = xxxxx000, no clock, timer stopped
            If TCCRnB = xxxxx001, prescaler 1
            If TCCRnB = xxxxx010, prescaler 8 (clock/8)
            If TCCRnB = xxxxx011, prescaler 64 (Arduino default initialization)
            If TCCRnB = xxxxx100, prescaler 256
            If TCCRnB = xxxxx101, prescaler 1024
            If TCCRnB = xxxxx11x, external clock source, we don't care about. 
        */
        
        // Several settings need to be made 
        // Waveform Generation Mode bigs (WGM): split across TCCRnA and TCCRnB. We want Mode 4 CTC (0100)
        // Clock Select: three bits which control the control the prescaler, these are the last three bits of 
        //      TCCRnB (CSn2, CSn1, CSn0)
        // Compare Match Output A,B, C Mode bits (COMnA,B,C): these enable/disable/invert outputs A,B,C
        // We aren't outputing anything related to this timer to the pins so we set these all to zero (pins disconnected from timer, normal pin operation)
        //      Two bits each, in TCCRnA register
        
        // To set the compare match value (TOP), we write to OCR3A
        
        // Where "n" is Timer Number
        //TCCRnA    7   6       5   4       3   2       1   0
        //       COMnA1:0    COMnB1:0    COMnC1:0     WGMn1:0
        //TCCRnB    7       6   5     4   3     2  1 0
        //      ICNCn   ICESn   -   WGMn3:2     CSn2:0
        // We don't need to worry about ICNC (input capture noise canceller) or ICES (Input Capture Edge Select)

        // Looking at the above, we need to set TCCR3A = 0 (disconnect pins, last two bits of WGM = 0)
        // Then we need to set TCCR3B = 0x00 0 01 100   bit 3 of WGM = 1, CS3 = 100 (prescaler 256)
        
        // So for Timer 3, here is what we need
        // CTC Mode 4, TOP = OCR3A: WGM33:0 = 0100
        // Prescaler = 256: CS32:0 = 100
        // COM5A1:0, COM5B1:0, COM5C1:0 = 00 disconnected


        TCNT3 = 0;                      // clear the timer count  

        // Initilialise Timer3
        TCCR3A = 0;
        TCCR3B = _BV(WGM32) | _BV(CS32);
        OCR3A = 244;        // 16,000,000 / 256 prescaler / 256 hertz = 244 TOP value

        // Timer interrupts
        TIFR3 = 1;                      // clear all interrupt flags. Oddly they are manually cleared by writing them with a logic one. 
        TIMSK3 = _BV(OCIE3A);           // Enable the compare interrupt A. The Timer 3 Compare A interrupt vector is called when OCF3A flag is set (pg 166)
                                        // OCF3A flag is set whenever TCNT3 values matches OCR3A
                                        // Flag is automatically cleared when ISR executed. 
                                        // We've disabled the other Timer3 interrupts including the overflow. We shall want to reset TCNT3 each time the ISR is called. 
        //OP_Driver::DisableRampInterrupt();    // Start with the interrupt turned off
}

void OP_Driver::setDrivingProfileSettings(boolean are, boolean dre, ACCEL_DRIVE_PRESET adp, DECEL_DRIVE_PRESET ddp, uint8_t asn, uint8_t dsn)
{
    // We can have two "driving profiles" each with independent settings for accel/decel. But in this class we don't bother knowing anything about these profiles, rather
    // we let the main sketch remember which one we're on, and if we change, call this function with the correct settings for the profile. This lets us keep only a single
    // copy of the settings in this class. 
    AccelRampEnabled = are;                         // This can be changed by the user. Even if enabled, ramping may not always be used in all conditions
    DecelRampEnabled = dre;                         //     UseDriveRamp will be the flag that indicates whether it is presently active. 
    AccelPreset = adp;                              // Accel/decel presets 
    DecelPreset = ddp;
    AccelSkipNum = asn;                             // These are the number of interrupts to skip before incrementing the drive speed, when drive ramping is enabled.
    DecelSkipNum = dsn;                             // The interrupt occurs 256 times a second, but we don't have to increment the speed each time. 
}

void OP_Driver::SetRampInterrupt()
{
    // If either drive or throttle ramping is enabled, keep the interrupt going
    if (DriveRampEnabled || ThrottleRampEnabled) EnableRampInterrupt();
    // Otherwise, turn it off. No need to waste cycles. 
    else DisableRampInterrupt();
}
void OP_Driver::DisableRampInterrupt()
{   
    TIMSK3 &= ~_BV(OCIE3A);
}
void OP_Driver::EnableRampInterrupt()
{   
    TIMSK3 |= _BV(OCIE3A);
}

// The user sets these values in OP Config. But we may want to change them on the fly too.
// These functions assume that variableNum is already scaled correctly and will never go below 1 or exceed MAX_SKIP_NUM.
// These checks are taken care of in the main sketch, see the SpecFunctions tab. 
void OP_Driver::setAccelRampFrequency(uint8_t variableNum)
{
    AccelSkipNum = variableNum;
}
void OP_Driver::setDecelRampFrequency(uint8_t variableNum)
{
    DecelSkipNum = variableNum;
}
// And if we want to see what the values are...
uint8_t OP_Driver::getAccelRampFrequency(void)
{
    return AccelSkipNum;
}
uint8_t OP_Driver::getDecelRampFrequency(void)
{
    return DecelSkipNum;
}

// Timer3 Output Compare A interrupt service routine
ISR(TIMER3_COMPA_vect)
{
    // If we have private member variables they will not be visible to this ISR,
    // instead we need to call a member function of OP_Driver
    OP_Driver::OCR3A_ISR();
}

// This is the routine that gets called 256 times a second. We keep it as short as possible.
void OP_Driver::OCR3A_ISR()
{
    // Reset the timer, start counting again
    TCNT3 = 0;

    // We keep skipping until we reach DriveSkipNum number of skips. 
    if (++DriveSkipCount >= DriveSkipNum)
    {   // Then reset the skip count, and increment our ramped drive speed
        DriveSkipCount = 0;
        RampedDriveSpeed += DriveRampStep;
    }
    
    if (++ThrottleSkipCount >= ThrottleSkipNum)
    {
        ThrottleSkipCount = 0;
        RampedThrottleSpeed += ThrottleRampStep;
    }
}

// GetDriveSpeed - this is where we pass a user drive command, and return a drive speed. If acceleration/deceleration constraints are disabled, the drive speed will 
// basically equal the command from the drive stick of the transmitter. Otherwise, the acceleration/deceleration speed will be whatever amount the user specified in 
// OP Config. On top of that, we can optionally implement refinements to these constraints through the use of presets. Presets represent a discrete set of 
// rules. For example, one preset might default to the user's acceleration constraint, but if the drive command is very far away from the current drive speed, we 
// accelerate a bit faster - and then, when the actual drive speed approaches the command, we accelerate a bit slower than default. As discussed above, acceleration/deceleration
// speed can be maninpulated by changing either the number of interrupts to skip between drive speed updates (interrupts occur 256 times a second but you don't have to update on each one),
// or by changing the amount or "step" change that occurs on an interrupt. 

// Hopefully the community will develop some interesting presets within this framework.  

int OP_Driver::GetDriveSpeed(int DriveCMD, int LastDriveSpeed, _driveModes DriveMode, boolean Brake)    // This function returns a DriveSpeed output, adjusted for accel/decel constraints
{
int8_t t_DriveSkipNum = 0;              // Temporary number of skips to reach before incrementing our speed. Initialize to zero, but it will be set to something else below.
int8_t t_DriveRampStep = 0;             // Temporary DriveRampStep value
int8_t RampDir = 1;                     // Is the ramping going up or down
int16_t t_DriveSpeed = 0;               // temp
int16_t FullStopCmd;                    // Any brake command greater than this, will result in an immediate full stop
boolean neg;

// Track recoil 
static boolean TrackRecoilStarted = false;
static uint16_t lastRampSpeed;
static uint16_t lastTRSpeed;
boolean SimpleTrackRecoil = true;		// In January 2020 we added a different track recoil action, set this to true to use the new action

    // First, to avoid repeating a bunch of code for forward & reverse, we convert everything to positive values, then convert them back to signed at the end.
    // We save a "neg" flag if the variable is negative, so we know at the end what sign to set it back to. 
    // Note that track recoil assumes negative movement, so we check that here as well. 
    if (DriveMode == REVERSE || DriveMode == TRACK_RECOIL || (DriveMode == NEUTRALTURN && DriveCMD < 0)) neg = true;
    else neg = false;
    // Now convert to absolute
    DriveCMD = abs(DriveCMD);
    LastDriveSpeed = abs(LastDriveSpeed);

    // Second, we start off by setting final DriveSpeed to DriveCMD to begin with. It may be modified through the process below. 
    t_DriveSpeed = DriveCMD;        

    // Third, we start off with drive ramping disabled. It will be enabled below if appropriate
    DriveRampEnabled = false;

    // Now, calculate actual drive speed: 

    // STOPPED
    // =============================================================================================================================================================>>
    if (DriveMode == STOP)
    {   
        t_DriveSpeed = 0;
    }
    // TRACK RECOIL
    // =============================================================================================================================================================>>    
    else if (DriveMode == TRACK_RECOIL)	// Track recoil is an anomaly and we are going to handle things exceptionally in this case. 
	{	
		if (SimpleTrackRecoil)
		{
			// This is the new, simpler track recoil action added in January 2020. Rather than start with a spike that tapers exponentially, we simply let the user 
			// specificy a speed percent and a time, and we just run in reverse at that speed for that amount of time. 
			if (!TrackRecoilStarted)
			{	
				// Just started track recoil movement
				// The first thing we do, is set the motor speed to KickbackSpeed. Above we already set the motor direction to reverse by setting the "neg" flag
				t_DriveSpeed = DriveCMD = lastTRSpeed = KickbackSpeed;       
				DriveRampEnabled = false;                   // Not using ramping for simple recoil
				RampDir = 0;                       			// No ramping
				TrackRecoilStartTime = millis();			// Record when we started
				TrackRecoilStarted = true;                  // Go
			}
			else
			{
				if ((millis() - TrackRecoilStartTime) >= TrackRecoilDuration) 
				{
					// When this function spits out a speed of 0, the sketch will automatically end the track recoil DriveMode
					t_DriveSpeed = DriveCMD = lastTRSpeed = 0;  
					TrackRecoilStarted = false;		
				}
				else
				{
					// Continue at the same speed
					t_DriveSpeed = DriveCMD = lastTRSpeed = KickbackSpeed;       
				}
			}
		}
		else
		{
			// This is the original track recoil action that starts with an initial reverse command spike that tapers exponentially. 
			if (!TrackRecoilStarted)
			{
				// Just started track recoil movement
				// The first thing we do, is set the motor speed to KickbackSpeed. Above we already set the motor direction to reverse by setting the "neg" flag
				t_DriveSpeed = DriveCMD = lastTRSpeed = KickbackSpeed;       
				DriveRampEnabled = true;                    // Now we will enable ramping although we will not use normally, see below
				RampDir         = 1;                        // Ramp goes up
				t_DriveSkipNum  = 1;                        // No skipping
				t_DriveRampStep = 1;                        // 1 step at a time (256 steps per second)
				RampedDriveSpeed = lastRampSpeed = 1;       // Save the start
				TrackRecoilStarted = true;                  // Go
			}
			else
			{
				// Typically we use the ramping feature to slowly increase or decrease our actual motor speed (RampedDriveSpeed). 
				// In this case, we are going to use RampedDriveSpeed essentially as a counter instead. We set step and skip to 1 such that 
				// RampedDriveSpeed will count to 256 every 1 second. After every 8 steps (1/32nd of a second) we decrement our
				// actual speed manually (t_DriveSpeed) by some deceleration factor set by the user but that falls somewhere in the general 
				// range of 65-98%. In other words, if speed last time was 100 and our factor is 85%, this time speed will be 85%, next
				// time around it will be 72%, etc... In this way we exponentially decrease our speed (not linearly), which results in 
				// a smooth but rapid deceleration from our original kickback speed
				if (RampedDriveSpeed >= 1024)  { t_DriveSpeed = 0; } // We've waited 4 full seconds, that's more than long enough, we're done
				else 
				{
					DriveRampEnabled = true;
					RampDir         = 1;
					t_DriveSkipNum  = 1;
					t_DriveRampStep = 1; 
					if ((RampedDriveSpeed - lastRampSpeed) >= 8)    // Every 1/32nd of a second we decrement speed
					{
						t_DriveSpeed = (int16_t)(((float)lastTRSpeed * DecelerationFactor) + 0.5);  // Decrease speed exponentially by our factor somewhere in the range of ~80-99%
						lastTRSpeed = DriveCMD = t_DriveSpeed;
						lastRampSpeed = RampedDriveSpeed;           // Save this so we know when the next 1/32nd of a second transpires
					}
					else
					{
						t_DriveSpeed = DriveCMD = lastTRSpeed;      // Still waiting for 1/32nd of a second to pass, hold last speed
					}
				}

				// If the speed has approached 0, we consider ourselves done recoiling. 
				// When this function spits out a speed of 0, the sketch will automatically end the track recoil DriveMode
				if (t_DriveSpeed <= 30)     // 30 out of 255 is ~12%. Below this level most gearboxes are moving imperceptibly if at all.
				{
					// We're done
					DriveRampEnabled = false;
					t_DriveSpeed = DriveCMD = lastTRSpeed = 0;  
					TrackRecoilStarted = false;
				}            
			}
		}
    }
    // BRAKING
    // =============================================================================================================================================================>>
    else if (Brake == true)
    {   // We want to decelerate
        RampDir = -1;

        (DriveMode == FORWARD) ? FullStopCmd = Forward_FullStopCmd : FullStopCmd = Reverse_FullStopCmd;
        if (DriveCMD >= FullStopCmd)
        {   // If the stick is near the complete opposite side, we stop immediately (ie, turn off all deceleration ramping)
            t_DriveSpeed = 0;
        }
        else
        {
            // Otherwise we brake slowly. We don't want the brake to be very sensitive, since the model is going to stop on its own
            // pretty quickly anyway. We want to give the user the feeling of really having to brake. 
            
            // We divide the opposite stick command by 16 and add the result to the default ramp step. Increasing the step amount will increase deceleration.
            
            DriveRampEnabled = true;                                        // Enable drive speed ramping
            t_DriveSkipNum = DecelSkipNum;                                  // We use the same skip num as set for regular deceleration. 
            t_DriveRampStep = DRIVE_RAMP_STEP_DEFAULT + (DriveCMD >> 4);    // But we increases the step amount, causing an increase in deceleration
            
            // But frankly, at deceleration levels under 4-5 seconds, you won't hardly have time to do any braking before it stops on its own anyway. 
        }
    }
    else
    {
        // NO CHANGE
        // =============================================================================================================================================================>>
        //if (DriveCMD == LastDriveSpeed)
        //{
            // We are commanding what we are already doing. No ramp step needed. 
        //}
    
        // ACCELERATION
        // =============================================================================================================================================================>>
        if (DriveCMD > LastDriveSpeed)
        {   // We want to accelerate
            if (AccelRampEnabled)
            {
                DriveRampEnabled = true;                            // Enable drive speed ramping 
                RampDir = 1;                                        // Tells us we are accelerating
                t_DriveRampStep = DRIVE_RAMP_STEP_DEFAULT;          // Set step to default
                t_DriveSkipNum = AccelSkipNum;                      // Set interrupts-to-skip number to the user setting 
                
                // Now apply any additional drive speed enhancements
                switch (AccelPreset)
                {
                    case ADP_PRESET_1:
                        // THIS IS AN EXAMPLE OF A PRESET. It modifies the acceleration parameters, 
                        // and the two modifications are "stackable", meaning they can both apply. 
                        if ((DriveCMD - LastDriveSpeed) > 60)       // If we are commanding something far away, we speed up a bit. 
                        {
                            t_DriveSkipNum -= 4;                    // Each unit is equal to 1/2 second increase in rate of acceleration. Smaller numbers mean faster
                        }                                       
                        
                        if (LastDriveSpeed < 20)                    // If we are at extremely low throttle, make more sensitive
                        {
                            t_DriveRampStep += 2;                   // Rather than change the SkipNum, we could also increase the step taken at each interrupt to increase acceleration,
                        }                                           // shown here.
                        break;
                    
                    case ADP_PRESET_2:
                        // Add Preset 2 code here
                        
                        break;
                    
                    case ADP_PRESET_3:
                        // Add Preset 3 code here
                        
                        break;
                }
            }
        }
    
        // DECELERATION
        // =============================================================================================================================================================>>
        else if (DriveCMD < LastDriveSpeed)
        {   
            if (DecelRampEnabled && DriveMode != NEUTRALTURN)       // Even if enabled, we don't apply deceleration ramping to neutral turns, it looks silly.
            {                                                       //          Also, for complicated reasons, it rarely takes effect for neutral turns anyway.
                DriveRampEnabled = true;                            // Enabled drive speed ramping
                RampDir = -1;                                       // Tells us we are decelerating
                t_DriveRampStep = DRIVE_RAMP_STEP_DEFAULT;          // Set step to default to start
                t_DriveSkipNum = DecelSkipNum;                      // Set interrupts-to-skip number to the user setting 
                
                switch (DecelPreset)
                {
                    case DDP_PRESET_1:
                        // THIS IS AN EXAMPLE OF A PRESET. It modifies the deceleration parameters,
                        // but in this case the two conditions are mutually-exclusive. 
                        if (DriveCMD < 50)                          // If we are at low throttle settings, we decelerate more slowly
                        {
                            t_DriveSkipNum += 2;                    // Adding means, take more time
                        }    
                        else if ((LastDriveSpeed - DriveCMD) > 50)  // If we are commanding a large deceleration difference, increase the deceleration a bit
                        {
                            t_DriveSkipNum -= 2;                    // Subtracting means take less time
                        }
                        break;
                    
                    case DDP_PRESET_2:
                        // Add Preset 2 code here
                        
                        break;
                        
                    case DDP_PRESET_3:
                        // Add Preset 3 code here
                    
                        break;
                }
            }
        }
    }

    // Don't let t_DriveRampStep get below 1
    if (t_DriveRampStep < 1) { t_DriveRampStep = 1; }
    
    // Don't let t_DriveSkipNum get below 1
    if (t_DriveSkipNum < 1) { t_DriveSkipNum = 1; }
    
    // Save ramp step and frequency
    if (DriveRampEnabled)
    {
        uint8_t sreg = SREG;                            // Save interrupt register
        cli();                                          // Disable interrupts
            DriveRampStep = t_DriveRampStep * RampDir;  // Multiply by RampDir which is 1 or -1 depending on the direction
            DriveSkipNum = t_DriveSkipNum;              // Number of interrupts to skip before incrementing by DriveRampStep
            // This next piece may look confusing. If ramping is enabled, we of course want to set t_DriveSpeed equal to 
            // RampedDriveSpeed. If ramping is enabled, t_DriveSpeed will right now equal ThrottleCMD. If it is some non-zero number that means we 
            // want to move, but if we are just now starting from a stop, RampedDriveSpeed will still be zero until the next time the interrupt hits. 
            // Since we do not want to return zero, in this case we set RampedDriveSpeed = to the first DriveRampStep. 
            // Afterwards, RampedDriveSpeed will be > 0 so this "if" statement will get skipped. 
            if (t_DriveSpeed > 0 && RampedDriveSpeed == 0) RampedDriveSpeed = DriveRampStep;
            if (DriveMode != TRACK_RECOIL) t_DriveSpeed = RampedDriveSpeed;    // We handle things differently during track recoil
        SREG = sreg;                                    // Restore register
    }
    else
    {
        // No ramping - output will be direct DriveCMD
        uint8_t sreg = SREG;                            // Save interrupt register
        cli();                                          // Disable interrupts
            DriveRampStep = 0;                          // No incremental changes
            RampedDriveSpeed = t_DriveSpeed;            // We also keep RampedDriveSpeed equal to drive speed so when ramping is turned back on, the Ramped value will already be correct to start 
        SREG = sreg;                                    // Restore register     (^ Remember t_DriveSpeed was set to DriveCMD earlier, but may have been changed to zero if braking or stopped).
    }

    SetRampInterrupt();                                 // This function will make sure the ramp interrupt is enabled if needed, but if not, will turn it off
    
    // Do a sanity check on t_DriveSpeed. 
    // Recall t_DriveSpeed is either DriveCMD (set at the beginning), or RampedDriveSpeed, or possibly 0 if we are braking
    if (Brake == true)
    {   // The most we can do when braking, is arrive at a stop. If abs(t_DriveSpeed) gets less than zero, it will overshoot the stop and change direction,
        // which we don't want. 
        if (t_DriveSpeed < 0) { t_DriveSpeed = 0; }
    }   
    else
    {
        if (RampDir == 1)          // ACCEL
        {   
            // If we are accelerating, we are trying to reach DriveCMD. We don't want to accelerate PAST what we have commanded. 
            t_DriveSpeed = constrain(t_DriveSpeed, 0, DriveCMD);    // (we already turned DriveCMD into abs() at the start, so this works for forward or reverse acceleration)
        }
        else if (RampDir == -1)    // DECEL
        {
            // If we are decelerating, that means we are commanding less than where we are presently. 
            // We don't let the drive speed decrease to less than what we are commanding. 
            // Obviously we don't let it be more than the maximum amount either, this is less likely. 
            
            // We can have different max speeds for forward and reverse, so we select the appropriate one here
            if (DriveMode == FORWARD)
            {    
                t_DriveSpeed = constrain(t_DriveSpeed, DriveCMD, MOTOR_MAX_FWDSPEED); // (we already turned DriveCMD into abs() at the start, so this works for forward or reverse deceleration)
            }
            else if (DriveMode == REVERSE)
            {   
                t_DriveSpeed = constrain(t_DriveSpeed, DriveCMD, abs(MOTOR_MAX_REVSPEED)); // (we already turned DriveCMD into abs() at the start, so this works for forward or reverse deceleration)
            }
        }
    }

    // CONVERT BACK FROM ABSOLUTE VALUES
    // -------------------------------------------------------------------------------------------------------------------------------------------------->    
        if (neg) t_DriveSpeed *= -1;

    return t_DriveSpeed;
}

// GetThrottleSpeed - this one is similar to GetDriveSpeed, except now we are setting our "virtual" engine throttle speed, not the tank's actual movement speed. 
// Throttle speed does *NOT* go to the drive motors - it will be used instead to set the speed of the mechanical smoker, and most importantly, the sound unit. 
// By divorcing the "engine" speed from actual drive speed, we can do simple things like rev the engine without moving the tank, or play a special sound on high acceleration, or 
// even potentially more sophisticated effects. 

// As with drive speed, we have the option of "ramping" throttle speed using similar parameters. The same 256 Hz interrupt is used, and we can set the amount the throttle speed
// changes each time (ThrottleRampStep), or how many interrupts most occur before an increment is allowed (ThrottleSkipNum). Of course, throttle speed could also be set directly
// instead of ramped. 

// For now, all we do here is try to prevent the throttle sound from stopping suddenly. 
int OP_Driver::GetThrottleSpeed(int ThrottleCMD, int LastThrottleSpeed, int DriveSpeed, _driveModes DriveMode, boolean Brake)
{
    int8_t t_ThrottleSkipNum = 0;           // Temporary number of interrupts to skip before incrementing throttle speed. Initialize to zero, but it needs to be set to something else below (if ramping is used). 
    int8_t t_ThrottleRampStep = 0;          // Temporary ThrottleRampStep value
    int8_t RampDir = 1;                     // Is the ramping going up or down
    int16_t t_ThrottleSpeed = 0;            // Temp throttle speed
    int16_t FullStopCmd;                    // Any brake command greater than this, will result in an immediate full stop
    
    // First of all, the engine only runs one direction, so we don't need negative numbers
    Brake ? ThrottleCMD = 0 : ThrottleCMD = abs(ThrottleCMD);   // Also, if we are braking, by definition we have let off the gas, so throttle speed will be zero (actually idle)
    DriveSpeed = abs(DriveSpeed);                               // DriveSpeed is not presently used in this routine, but is passed anyway for some future effect that may want to know it. 
    //LastThrottleSpeed should already come in as a positive number

    // Second, we start by initializing final throttle speed to the throttle command speed (essentially the throttle stick position), but this may change by the end of the routine
    t_ThrottleSpeed = ThrottleCMD;          

    // Third, we start off with throttle ramping disabled. It will be enabled below if appropriate
    ThrottleRampEnabled = false;

    // Now, calculate actual throttle speed: 


    // BRAKING or DECELERATING
    // =============================================================================================================================================================>>
    if (Brake == true || ThrottleCMD < LastThrottleSpeed)
    {   // We want to decelerate

        (DriveMode == FORWARD) ? FullStopCmd = Forward_FullStopCmd : FullStopCmd = Reverse_FullStopCmd;
        if (Brake && ThrottleCMD >= FullStopCmd)
        {   // This is the same calculation as we did above for drive speed, this means a full stop brake command. 
            // In the drive speed case, we set the tank to stop immediately. Here, we could play a special hard braking sound
            // or whatever (you would have to pass in a pointer to the OP_Sound object to this function)
        }

        // While the tank may be able to slow down very quickly, there is no "brake" on the engine RPM. Think of a real car, 
        // you can have the pedal to the floor and then let go, but it will still take a moment for the engine speed to return 
        // to idle on its own. 
        
        // What we're trying to do on deceleration, is prevent the engine sound from stopping too suddenly, because that will sound strange. 
        // To ease it back to idle, we use ramping. 
        ThrottleRampEnabled = true;                         // Enable throttle ramping
        RampDir = -1;                                       // Tells us we are decelerating
        t_ThrottleRampStep = DRIVE_RAMP_STEP_DEFAULT;       // Set step to default (2 steps per active interrupt)
        
        // This is just hard-coded for now. 
        // Combined these two statements result in an engine that takes ~1.4 seconds to return to idle from full speed
        if (LastThrottleSpeed > 50)                         
        {   // This rate only appplies to throttle speeds above 50, and this portion will take approximately 1 second
            t_ThrottleRampStep = 3;
            t_ThrottleSkipNum = 4;
        }
        else
        {   // At throttle speeds below 50, we slow down a bit more slowly. From 50 to 0 these settings will take ~0.4 seconds
            t_ThrottleRampStep = 2;
            t_ThrottleSkipNum = 4;      
        }                               
    }
   
    // ACCELERATION
    // =============================================================================================================================================================>>
    else if (ThrottleCMD > LastThrottleSpeed) // Acceleration 
    {   // Here we do use a very small bit of ramping on acceleration. If you step on the gas, the engine RPM responds immediately (even though tank speed won't respond so quickly),
        // and we want to keep that responsive feel. But some sound cards like the Taigen will sound odd if you jump straight to full speed so we do introduce a slight delay. 

        // Don't limit anything at a low level so we get a responsive throttle sound. But above that we do limit just a bit so it doesn't sound jerky, the settings here
        // will take approximately 3/4 of a second.
        if (LastThrottleSpeed > 60)
        {
            ThrottleRampEnabled = true;
            RampDir = 1;
            t_ThrottleRampStep = 2;
            t_ThrottleSkipNum = 2;
        }
        // else if throttle speed < 60 then there will be no ramping
    }

    // Don't let t_ThrottleRampStep get below 1
    if (t_ThrottleRampStep < 1) { t_ThrottleRampStep = 1; }
    
    // Don't let t_ThrottleSkipNum get below 1
    if (t_ThrottleSkipNum < 1) { t_ThrottleSkipNum = 1; }
        
    // Save ramp step 
    if (ThrottleRampEnabled) 
    {   
        uint8_t sreg = SREG;                        // Save interrupt register
        cli();                                      // Disable interrupts
            ThrottleRampStep = t_ThrottleRampStep * RampDir;    // Multiply by RampDir which is 1 or -1 depending on the direction
            ThrottleSkipNum = t_ThrottleSkipNum;    // Number of interrupts to skip before incrementing by ThrottleRampStep
            t_ThrottleSpeed = RampedThrottleSpeed;  // We take the ramped value, actually from last update. 
        SREG = sreg;                                // Restore register
    }
    else
    {
        // No ramping - output will be direct ThrottleCMD
        uint8_t sreg = SREG;                        // Save interrupt register
        cli();                                      // Disable interrupts
            ThrottleRampStep = 0;                   // No incremental changes
            RampedThrottleSpeed = t_ThrottleSpeed;  // We also keep RampedThrottleSpeed equal to throttle so when ramping is turned back on, the Ramped value will already be correct to start 
        SREG = sreg;                                // Restore register     
    }

    SetRampInterrupt();                             // This function will make sure the ramp interrupt is enabled if needed, but if not, will turn it off

    // Do a sanity check on t_ThrottleSpeed. 
    // Recall t_ThrottleSpeed is either ThrottleCMD (set at the beginning), or RampedThrottleSpeed, or possibly 0 if we are braking
    if (Brake == true)
    {   // The most we can do when braking, is arrive at a stop. If abs(t_ThrottleSpeed) gets less than zero, it will overshoot the stop and start accelerating again,
        // which we don't want. 
        if (t_ThrottleSpeed < 0) { t_ThrottleSpeed = 0; }
    }   
    else
    {
        if (RampDir == 1)          // ACCEL
        {
            // If we are accelerating, we are trying to reach ThrottleCMD. We don't want to accelerate PAST what we have commanded. 
            t_ThrottleSpeed = constrain(t_ThrottleSpeed, 0, ThrottleCMD);    // (we already turned ThrottleCMD into abs() at the start, so this works for forward or reverse acceleration)
        }
        else if (RampDir == -1)    // DECEL
        {
            // If we are decelerating, that means we are commanding less than where we are presently. 
            // We don't let the throttle speed decrease to less than what we are commanding. 
            // Obviously we don't let it be more than the maximum amount either, this is less likely. 
            t_ThrottleSpeed = constrain(t_ThrottleSpeed, ThrottleCMD, MOTOR_MAX_FWDSPEED); // (we already turned ThrottleCMD into abs() at the start, so this works for forward or reverse deceleration)
        }
    }

    return t_ThrottleSpeed;
}

// Determine the drive mode being *commanded* as interpreted by the position of the throttle stick
_driveModes OP_Driver::GetDriveMode(int ThrottleCMD, int TurnCMD)
{
    if (ThrottleCMD       >  0) { return FORWARD;     }
    else if (ThrottleCMD  <  0) { return REVERSE;     }
    else if (TurnCMD      == 0) { return STOP;        }
    else                        { if (NeutralTurnAllowed) {return NEUTRALTURN;} else {return STOP;} }   // Neutral turn commands are converted to STOP when not allowed
}

// But just because the throttle is forward or reverse, doesn't necesarily mean the user is commanding forward or reverse - 
// they may actually be commanding brake. 
boolean OP_Driver::GetBrakeFlag(_driveModes DriveModePrev, _driveModes DriveModeCMD) 
{   
    boolean Brake = false;
    
    // This function basically compares the drive mode we currently exist in, 
    // with the Commands being received
    // We then determine if a braking command is being given
    
    if (DriveModePrev == FORWARD && DriveModeCMD == REVERSE)
    {
        Brake = true;
    }
    if (DriveModePrev == REVERSE && DriveModeCMD == FORWARD)
    {
        Brake = true;
    }
    return Brake;
}    


// The user has the option of limiting turn speed for neutral turns (in tank mode), or the amount of turn command that gets
// sent to the rear treads in halftrack mode. 
// This function scales the turn command to some reduced amount. 
int OP_Driver::ScaleTurnCommand(int TurnCMD, int MaxTurn)
{
    if (TurnCMD < 0)
    {    
        return map(TurnCMD, MOTOR_MAX_REVSPEED, -1, -MaxTurn, -1);  
    }
    else if (TurnCMD > 0)
    {    
        return map(TurnCMD, 1, MOTOR_MAX_FWDSPEED, 1, MaxTurn);    
    }
    else
    {
        return 0;
    }
}

// The user selects the turn mode in the OP Config program, but we also allow it to be changed on the fly. 
void OP_Driver::setTurnMode(uint8_t tm)
{
    // Only change the mode if a valid number is passed
    if (tm >= 1 && tm <= NUM_TURN_MODES)
    {
        TurnMode = tm;
    }
}

uint8_t OP_Driver::getTurnMode(void)
{
    return TurnMode;
}

// The user can select the neutral turn allowed status in OP Config, but we also allow them to change it on the fly.
void OP_Driver::setNeutralTurnAllowed(boolean allowed)
{
    NeutralTurnAllowed = allowed; 
}

boolean OP_Driver::getNeutralTurnAllowed(void)
{
    return NeutralTurnAllowed; 
}

// This mixes throttle and turn commands and spits out left and right tread commands. We can manipulate the mix formula in different ways, 
// we call these "Turn Modes" and the user has several to select from. 
void OP_Driver::MixSteering(int DriveSpeed, int TurnAmount, int *RightSpeed, int *LeftSpeed) 
{   
    // To make the calculations easy, we think of the results as inner and outer track. Inner track will be moving slower than
    // outer track in a turn. We convert these back to left and right at the end. 
    int InnerTrack;
    int OuterTrack;

    // Absolute values of DriveSpeed and TurnAmount. We deal in absolutes to make the calculations easier, 
    // then convert them back to forward/reverse at the end. 
    int Drive;
    int Turn;

    // Temp vars
    float TempFloat1 = 0.0;
    
    // Ultimate left and right outputs
    int s_Right = 0;
    int s_Left = 0;
    
    
    if (TurnAmount == 0)
    {
        // ------------- STOP -------------------------------------------->
        if (DriveSpeed == 0)
        {
            // No turn, no speed - set motors to stop
            s_Left = 0;
            s_Right = 0;
        }
        // ------------- FWD/REV + STRAIGHT ------------------------------>
        else
        {
            // We have speed, but no turn - set both motors equally
            s_Left = DriveSpeed;
            s_Right = DriveSpeed;
        }
    }
    else
    {
        // ------------- NEUTRAL TURN ------------------------------------>
        if (DriveSpeed == 0)
        {
            if (NeutralTurnAllowed)   
            {
                // We have no speed - this is a turn-in-place, we simply set each motor opposite to each other
                // EX: -255 = Full Left Turn. s_Left = -255, meaning reverse. s_Right = 255 meaning forward
                // EX: +200 = Right Turn. s_Left = 200 meaning forward. s_Right = -200 meaning reverse
                s_Left = TurnAmount;
                s_Right = -TurnAmount;
            }
        }
        else // TURN AND SPEED
        {
        // ------------- FWD/REV + TURN ---------------------------------->

        // Start off with absolute (positive) variables for drive and turn. This makes our calcs easier. 
        // We will convert everything back at the end. 
        Drive = abs(DriveSpeed);
        Turn = abs(TurnAmount);

        switch (TurnMode)
        {
            // MODE 1 = Most simple, but works well. 
            //          Outer track maintains drive speed. Turn is subtracted from inner track. Inner track can't go slower than stop.
            //          We scale the turn signal to drive speed so it always takes the same amount of turn stick movement to do the same turn at all speeds. 
            case 1:
                OuterTrack = Drive;
                
                if (DriveSpeed > 0)
                {   InnerTrack = Drive - map(Turn,0,MOTOR_MAX_FWDSPEED,0,Drive); }
                else
                {   InnerTrack = Drive - map(Turn,0,abs(MOTOR_MAX_REVSPEED),0,Drive); }
                
                break;
                
                
                
            // MODE 2 = Same as above, but we mix a bit of the steering into throttle. 
            //          In other words, as the turn command increases, so does the total drive speed. It's like every time the driver does a turn, 
            //          he also steps on the gas. This is not entirely realistic but makes for smoother driving, because the tank is not
            //          slowing down every time we turn. 
            case 2:
                TempFloat1 = (float)Turn/(float)MOTOR_MAX_FWDSPEED;     // What is our turn command in percent of total turn
                Drive = Drive + (int)((float)Drive * TempFloat1);       // Add this percentage to our Drive speed
                
                if (DriveSpeed > 0)                                     // But of course, we can't add so much that we exceed our max drive speed
                {   
                    Drive = constrain(Drive,0,MOTOR_MAX_FWDSPEED);
                    InnerTrack = Drive - map(Turn,0,MOTOR_MAX_FWDSPEED,0,Drive);
                }
                else
                {   
                    Drive = constrain(Drive,0,abs(MOTOR_MAX_REVSPEED));
                    InnerTrack = Drive - map(Turn,0,abs(MOTOR_MAX_REVSPEED),0,Drive); 
                }
                OuterTrack = Drive;
                
                break;
                
                

            // MODE 3 = This is a slightly milder form of Mode 2. Instead of increasing speed across the board, we divided the turn in half, and split the difference across the tracks. 
            //          Half of the turn is applied as an *increase* to the outer track, half is applied as a *decrease* to the inner track. 
            //          The nice side-effect of this mode, is that the turning radius increases as speed increases. Because we never subtract more than half of the turn
            //          from the inside track, at full speed the slowest the inner track can go is half speed. But the turn tightens as speed decreases. 
            case 3:
                int HalfTurn;
                                
                // First, scale turn to throttle speed for consistent stick movement at all speeds
                if (DriveSpeed > 0)
                {   Turn = map(Turn,0,MOTOR_MAX_FWDSPEED,0,Drive); }
                else
                {   Turn = map(Turn,0,abs(MOTOR_MAX_REVSPEED),0,Drive); }
                
                // Half of turn
                HalfTurn = Turn >> 1;
                
                // Apply half of turn to each track
                InnerTrack = Drive - HalfTurn;
                OuterTrack = Drive + HalfTurn;
                
                // Constrain outputs
                if (DriveSpeed > 0)
                {   InnerTrack = constrain(InnerTrack,0,MOTOR_MAX_FWDSPEED);
                    OuterTrack = constrain(OuterTrack,0,MOTOR_MAX_FWDSPEED);
                }
                else
                {   InnerTrack = constrain(InnerTrack,0,abs(MOTOR_MAX_REVSPEED));
                    OuterTrack = constrain(OuterTrack,0,abs(MOTOR_MAX_REVSPEED));
                }

                break;

            }   // END SWITCH CASE STATEMENT
            
            // Now apply inner and outer tracks to right or left
            if (TurnAmount < 0)
            {   // Left turn
                s_Left = InnerTrack;
                s_Right = OuterTrack;
            }
            else
            {   // Right turn
                s_Left = OuterTrack;
                s_Right = InnerTrack;
            }
            // Now convert the absolute values back forward/reverse
            if (DriveSpeed < 0)
            {   // Reverse
                s_Left *= -1;
                s_Right *= -1;
            }
        }
    }
                
    // Return speeds
    *RightSpeed = s_Right;
    *LeftSpeed =  s_Left;

    return; 
}



