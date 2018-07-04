
// Include the Open Panzer Scout library
#include "OP_Scout.h"


// Declare the Scout object. 
// Pass the address you want to use (A or B) as well as the hardware serial port
// For most Arduinos there will only be one serial port, but some like the Mega may have multiple (Serial1, Serial2, etc.)
OP_Scout Scout(SCOUT_ADDRESS_A, &Serial);



void setup() 
{
    // Scout communicates at 38400 baud by default
    Serial.begin(38400); 

    // Give the Scout some time to startup
    delay(1000); 


    
    // SERIAL WATCHDOG 
    // This is optional. You can choose to enable the Serial Watchdog feature on the Scout (it is disabled by default). 
    // The watchdog will expect a serial command within an interval you set, if none is received in that time the motors will 
    // be shut down. This can be a useful safety feature but it also requires your control sketch to send the command at routine 
    // intervals even if the command hasn't changed. 
    
    // The watchdog timeout can range from 100mS to 25.5 seconds in 100mS increments
    // The function for converting desired watchdog time to data byte is 
    //      Data byte = Desired time in mS / 100
    // Let's enable the serial watchdog with a timeout of 1 second (1000mS), so data byte will be:
    //      1000 / 100 = 10
    
    Scout.EnableWatchdog(10); 



    // CURRENT LIMITS
    // By default the Scout will limit current to 12 amps per motor, since this is roughly the amount of continuous current draw the board can handle
    // without extra heatsinking/cooling. The absolute limit of the motor driver chips is 30 amps per motor, but that will definitely require the 
    // addition of heatsinks and fan. If the chips become too hot, they are over-temperature protected and will automatically shut down. 
    // You may also want to limit the current to an even lower level than default, for example to protect a sensitive drive-train. 
    // You can use the SetMaxCurrent function to specify the maximum current (per motor) that the Scout will permit, but beyond which it will turn off the motors.
    // Note: if even one motor exceeds the current limit, both motors will be turned off. 
    // Note: any custom current limit setting is not retained by the Scout, it always boots to the default of 12 amps, so if you want a different value send it each time on boot
    
    Scout.SetMaxCurrent(5);     // This would limit the maximum current per motor to 5 amps. Acceptable values are 1 to 30
    
}


void loop() 
{
    // TEST MOTORS
    // ================================================================================================================================>>
    // Uncomment this routine to cycle both motors
        WaveMotors();
        

    // TEST DIRECT FAN CONTROL
    // ================================================================================================================================>>
    // Uncomment this section to test direct fan control, which could be used as a third channel, uni-directional, low current ESC (1 amp), running at VBatt
    /*
        Scout.SetFanSpeed(127);     // 50%
        delay(4000);
        Scout.SetFanSpeed(255);     // 100%
        delay(4000);
        Scout.SetFanSpeed(0);       // Stop
        delay(4000);
    
        // We can also revert back to auto fan control, where the Scout manages the fan in order to cool itself
        // Auto control is the default mode and the Scout will initialize to that mode unless and until an explicity SetFanSpeed function call is made. 
        AutoFanControl();
    */
    

    // TEST SERIAL WATCHDOG
    // ================================================================================================================================>>    
    // Uncomment this to section to test the serial watchdog feature
    /*    
        // Set Motor 1 to half speed forward
        Scout.motor(1, 64);

        // Now delay for 5 seconds
        delay(5000);

        // In the setup above we told the Scout to time-out after 1 second without a command. After 1 second, the Scout should automatically stop Motor 1
        // and start flashing the blue LED rapidly. 

        // Now let's disable the serial watchdog:
        Scout.DisableWatchdog();

        // Then set Motor 1 back to half speed forward
        Scout.motor(1, 64); 

        // Now delay again for 5 seconds
        delay(5000); 

        // In this case the Scout should just keep running the motor indefinitely. After a short amount of time the blue LED on the Scout will start to blink 
        // slowly (not a quick flash). The slow blink means it is waiting for a signal, but in the meantime it is still going to maintain the last command. 
    */

}



// This routine will gradually run up each motor to full speed and then back down, first in one direction, and then in the other direction, 
// alternating between motor 1 and 2 each time
void WaveMotors(void)
{
#define             StepDelay           10
static uint32_t     TimeLastStep =      0;
static byte         CurrentMotor =      1;
static int8_t       CurrentStep =       0;
static byte         CurrentDirection =  1;       // 1 means go forward, 0 means go reverse
static boolean      GoingUp =           true;    // Are we accelerating, or decelerating
static boolean      RampStarted =       false;

    if (millis() - TimeLastStep > StepDelay)
    {
        switch (CurrentStep)
        {
            case 0:
                if (RampStarted)
                {
                    // This cycle is over
                    if (CurrentMotor == 1)
                    {
                        CurrentMotor = 2;
                    }
                    else
                    {
                        // Start over with motor 1, but now go in the opposite direction
                        CurrentMotor = 1;
                        CurrentDirection ? CurrentDirection = 0 : CurrentDirection = 1;
                    }
                    RampStarted = false;
                }
                else
                {   
                    // We are just starting to ramp up, set the motor in the correct direction             
                    RampStarted = true;
                    GoingUp = true;
                    CurrentDirection ? CurrentStep += 1 : CurrentStep -= 1;
                }
                break;

            case 127:
                // We were accelerating in the forward direction, now decelerate
                GoingUp = false;
                CurrentDirection ? CurrentStep -= 1 : CurrentStep += 1;
                break;

            case -127:
                // We were accelerating in the reverse direction, now decelerate
                GoingUp = false;
                CurrentDirection ? CurrentStep -= 1 : CurrentStep += 1;
                break;

            default: 
                if (GoingUp)
                {
                    CurrentDirection ? CurrentStep += 1 : CurrentStep -= 1;
                }
                else
                {
                    CurrentDirection ? CurrentStep -= 1 : CurrentStep += 1;
                }
        }

        Scout.motor(CurrentMotor, CurrentStep);
        TimeLastStep = millis();
    }

}

