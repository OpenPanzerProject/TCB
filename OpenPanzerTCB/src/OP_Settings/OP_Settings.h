/* OP_Settings.h    Open Panzer Settings file - a header file that defines many of the hardware elements of the TCB Board
 * Source:          openpanzer.org              
 * Authors:         Luke Middleton
 *
 * These values should not need to be modified by general users. These are basically hardware specific settings for the TCB board. 
 *   
 */ 


#ifndef OP_SETTINGS_H
#define OP_SETTINGS_H

#include <Arduino.h>

// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// OPEN PANZER TANK CONTROL BOARD (TCB) - FIRMWARE VERSION NUMBER
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // This is the firmware version that will be returned to the desktop application when it checks. It will be split into three, two-digit numbers
    #define FIRMWARE_VERSION        "0.92.19"      // version. Last update 1/13/2018
    
    #define MIN_OPCONFIG_VERSION    "0.92.14"      // Minimum version of OP Config this version of firmware requires

// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// PROGMEM
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // This is where we stick custom progmem tables, way out beyond program space
    #define PROGMEM_FAR  __attribute__((section(".fini7")))     // The .finiN sections are executed in descending order from 9 to 0. I suppose would could also change this to .fini9,
                                                                // but in themselves they don't define a specific address location, that has to be accomplished through linker directives. 
  
    // Custom strcpy_P routine that works on far progrmem. You need to pass it both the 32 bit pointer to the location of the string you want (base), and if the string is in an array, 
    // then you also need to provide the offset to the array element. For an example of calculating these see the DumpFunctionTriggers() function on the Utilities tab of the main sketch. 
    inline char* strcpy_PFAR(char* des, uint32_t base, uint32_t offset)
    {
        uint32_t p = base + offset;
        char* s = des;
        do 
        {
            *s = pgm_read_byte_far(p++);
        }
        while(*s++);
        return des;
    } 

// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// SERIAL PORTS
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // For Serial0, we just use "Serial". But we rarely call that directly. 
    
    // Serial port used for external motor control, in the event we are controlling any serial-based ESCs (Pololu, Sabertooth, etc)
    // Assuming each device has unique serial addresses/protocols, multiple serial ESCs can be attached to the same serial port. 
    #define MotorSerial                 Serial2

    // We also have access to the hardware Serial 1 port. We reserve it for auxillary functions. It has the complete pinout needed for an FTDI connection,
    // which also makes it compatible with the Adafruit Bluefruit EZ Link. We may also use this port for UART-access to our IMU since I2C didn't seem to work out.
    #define AuxSerial                   Serial1
    
    // The original plan for Serial 3 was to reserve it for a serial LCD. The connector on the board is compatible with quite a few Adafruit and SparkFun serial LCDs
    // that all have a 3-pin JST-PH connector (+5v, Gnd, and Tx). Later we added SBus support and we read SBus on the Serial 3 input (Rx) at 100000 baud, which is 
    // non-standard. If we use an SBus receiver, the Serial 3 Tx line is disabled in code, so an LCD on that port will no longer work. And in four years of development,
    // we never needed to use an LCD anyway. 
    // We've left the Serail 3 Tx connector on the TCB board for the fun of it, and it may be of some use in certain situations. But if you want to use an SBus receiver
    // and an LCD, you'll have to put the LCD on Serial1 (AuxSerial). 
    #define Serial3Tx                   Serial3     // We call this Serial3Tx because we only have access to the Tx line, not the Rx (which is dedicated to SBus). 

    // At startup, before EEPROM is initalized, set default baud rate to: 
    #define DEFAULTBAUDRATE              38400

    // USB Baud rate is fixed for now. Less confusion for the user.
    #define USB_BAUD_RATE               115200



// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// TIMER 0 - DEFAULT
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // Timer 0 is for now left to the Arduino-default setup. We believe this to be: 
    // Prescaler of 64 with a TOP of 255. This would mean each tick is 4 uS and TCNT0 rolls-over every 256 ticks (roughly 1 mS). 
    // Not sure what WGM mode Arduino default its to. If Normal or Fast PWM, then frequency is likely to be ~976 Hz (F_Osc / Prescaler / TOP)
    // If Phase Correct it could be half that. 
    
    // We use Timer 0 PWM on Arduino Pin 4 (Atmega Pin 1) for the brakelights, which can be dual-brightness. The low frequency could cause LED flicker
    // but we have not noticed an issue with it yet. 
    
    // The IRrecvPCI class uses the built-in Arduino micros() function (dependent on Timer 0) to check the time as IR signals are received (which cause an external 
    // interrupt on the input pin). It uses these times to determine the length of the incoming signals. Since the micros() function only has a resolution of 4 uS, 
    // that is the resolution of the IR read function. 
    
    // We could easily change the IRrecvPCI class to check against Timer 1 (TCNT1) and convert the ticks to uS. Not only would this remove any dependence on the micros()
    // function, it would probably be faster, and it would enhance resolution though that isn't strictly necessary.  

    // OP_SimpleTimer, OP_Button, the ElapsedMillis class, and the main sketch all use calls to the Arduino built-in function millis(), which is also based on Timer 0. 
    // Again, these could be modified to use Timer 1 but so far have not. We would want to write our own custom micros() and millis() functions if so. 
    
    // The advantage would be that we could then change the prescaler and other settings of Timer 0, but that doesn't really net us much. 


// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// TIMER 1
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // Timer 1 is used by several classes:
    // [] PPMDecode - checks the value of TCNT1 everytime the PPM input pin changes (the pin is using an external interrupt). By checking the time since the
    //    last signal, it knows how long the pulse-width is
    // [] OP_Servos - uses Timer 1's Output Compare A to set a timed interrupt to generate servo pulse widths
    // [] IRsendBase - uses Timer 1's Output Compare B to set a timed interrupt to generate infra-red pulses. IRsend also uses Timer 2 for the actual PWM.
    // [] SBusDecode/iBusDecode - uses Timer 1's Output Compare C to set a timed interrupt that we use for error checking of the incoming pulse stream

    // We set up Timer 1 in Normal Mode: count starts from BOTTOM (0), goes to TOP (0xFFFF / 65,535), then rolls over. 
    // We set prescaler to 8. With a 16MHz clock that gives us 1 clock tick every 0.5 uS (0.0000005 seconds).
    // The rollover will occur roughly every 32.7 mS (0.0327 seconds). These settings are dictated by TCCR1A and TCCR1B.
    // We also clear all interrupt flags to start (write 1 to respective bits of TIFR1). 
    // And we start off with all interrupts disabled (write 0 to all bits in TIMSK1). 
    #define SetupTimer1() ({ \  
        TCCR1A = 0x00;       \
        TCCR1B = 0x02;       \
        TIFR1 =  0x2F;       \
        TIMSK1 = 0x00;       \
        TCNT1 = 0; })
    
    // We also need to let these libraries know what the conversion is between ticks and uS
    #define PPM_TICKS_PER_uS        2           // For the PPMDecode class
    #define SBUS_TICKS_PER_uS       2           // For the SBusDecode class
    #define iBUS_TICKS_PER_uS       2           // For the iBusDecode class
    #define IR_uS_TO_TICKS(s)       (s*2)       // For IR sending
    #define SERVO_uS_TO_TICKS(s)    (s*2)       // For converting servo pulse-widths to tick counts
    #define SERVO_TICKS_TO_uS(s)    (s/2)       // For converting servo tick counts to pulse widths
    
    // Each of these libraries still have many hardcoded references to Timer 1, so if you ever do decide to change the timer you will have to do more than
    // just modifing the above...
    

// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// TIMER 2
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // Timer 2 is used to generate the PWM for IR sending. It is used in phase-correct PWM mode, with OCR2A controlling the frequency and 
    // OCR2B controlling the duty cycle. There is no prescaling, so the output frequency is 16MHz / (2 * OCR2A)
    // To turn the output on and off, the PWM is left running, but we connect and disconnect the output pin. 
    // This pin is hardcoded below to Atmega pin 18 / Port H6 / Arduino pin 9 (OC2B).
    // As Ken Shirriff said, a few hours staring at the ATmega documentation and this will all make sense.
    // For more info, see his website "Secrets of Arduino PWM" at http://www.righto.com/2009/07/secrets-of-arduino-pwm.html
    #define IR_SEND_PWM_PIN         9                               // Arduino pin 9 (Atmega Pin 18)
    #define IR_SEND_PWM_START       (TCCR2A |= _BV(COM2B1))         // Macro to connect OC2B to PWM pin
    #define IR_SEND_PWM_STOP        (TCCR2A &= ~(_BV(COM2B1)))      // Macro to disconnect OC2B from PWM pin
    // This sets up the modulation frequency in kilohertz
    #define IR_SEND_CONFIG_KHZ(val) ({ \
                                    const uint8_t pwmval = SYSCLOCK / 2000 / (val); \
                                    TCCR2A = _BV(WGM20);  TCCR2B = _BV(WGM22) | _BV(CS20); \
                                    OCR2A = pwmval; OCR2B = pwmval / 3; })
    

// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// TIMER 3
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // Timer 3 is used for ramping effects: 
    // OP_Driver - uses Timer 3's Output Compare A for acceleration and deceleration ramping of the main drive motors
    

// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// TIMER 4 - DEFAULT
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // Timer 4 is used to generate PWM for the Aux Output (if used in analog mode) and the "Hit Notification LEDs" which are the LEDs typically installed 
    // in the IR apple. These LEDs are dimmed and faded in and out to create various effects. 
    
    // We also use Timer 4's overflow interrupt to generate a stream of data that can control a Taigen sound card. 
    
    // We set Timer 4 to Fast PWM 10-bit (WGM4 3:0 0111) with a prescaler of 1 (CS4 2:0 001). We leave COM4B and COM4C pins connected to PWM (COM 10)
    // but disconnect PWM from COM4A pin (it can still be used as a digital I/O and it controls part of the direction for onboard motor driver A and is 
    // unaffected by what we do here). 
    
    // PWM for the Aux output and Hit Notify LEDs will be ~15.6kHz. That is not actually ultrasonic (>20kHz) but should be relatively quiet if a motor is used on the Aux output. 
    // For switching LEDs even something far lower would be fine.  
    // Timer 4 will tick (TCNT4 increment) once every 0.0625 uS, or in other words, one uS = 16 ticks. 10-bit mode means TOP is equal to 1024, which means Timer 4 will overflow
    // every (1024 ticks * 0.0625uS per tick) = 64 uS. This doesn't sound like a useful number but multiplied by 8 it equals .512mS. This works very well because the data stream 
    // for the Taigen sound cards all involve pulses in increments of ~1/2mS. In fact we could have set the prescaler to 8 and then the overflow would occur every ~1/2mS, which is 
    // more convenient, however this gives us a very low PWM frequency which means a noisy motor on the Aux output if the user chooses to use it for that (some are driving fans from it). 
    // Since we are going to have to count multiple overflows anyway, it is no more work to count 8 more overflows. 

    // NOTE: Our first thought was to use an output compare (OCR4A) to create interrupts at specified times the way we do with generating servo pulses. The problem with this is 
    // that unless you are in Normal or CTC mode, OCRnx doesn't update immediately, making it of little use for this purpose. But using Normal or CTC mode doesn't really leave us
    // with a useful PWM signal which we want for the other two pins (OC4B and OC4C). So setting an overflow every increment of time and counting overflows works well. 
    
    // NOTE: Instead of 10-bit with a TOP of 1024 we could set top to a custom number using ICR4, for example we could set it to 1,000 so each rollover is precisely 1/2mS. 
    // Nothing wrong with that, but actually the Taigen pulses tend to go a little long anyway, so 1024 seems to work just fine. 

    // TCCR4A = 0x2B    // PWM disabled on OCR4A - Fast PWM 10 bit
    // TCCR4B = 0x09    // Fast PWM, 1 (no) prescaler, TOP 1024 - frequency ~16 KHz, tick every 0.0625uS (16 ticks per uS)
    // TIFR4 = 0x2F     // Clear all interrupt flags
    // TIMSK4 = 0x00    // No interrupts enabled - later in OP_TaigenSound.cpp we will enable overflow interrupts by setting the TOIE4 bit in TIMSK4. 
    #define SetupTimer4() ({ \  
        TCCR4A = 0x2B;       \
        TCCR4B = 0x09;       \
        TIFR4 =  0x2F;       \
        TIMSK4 = 0x00;       \
        TCNT4 = 0;           \
        OCR4A = 0;           \
        OCR4B = 0;           \
        OCR4C = 0;  })
    

// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// TIMER 5
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // Timer 5 is used to generate ultra-sonic PWM for variable speed control of three motors: 
    // Arduino pins 46 (OC5A) and 45 (OC5B) for the L298 dual motor controller, and 44 (OC5C) for the MOSFET output to the Heng Long smoker.

    // Discussion:
    // See the Atmega2560 datasheet, chapter 17 (16 bit timers)
    // See also The Secrets of Arduino PWM by Ken Shirriff: http://www.righto.com/2009/07/secrets-of-arduino-pwm.html
    
    // Prescaler settings for PWM frequency:
    // - On the Mega, for 16 bit timers only 1, 3, 4, and 5 (not timers 0 or 2), 
    // - using 16mHz system clock, 
    // - Phase correct PWM with TOP at 255 (8-bit)
    //
    //    If TCCRnB = xxxxx000, no clock, timer stopped
    //    If TCCRnB = xxxxx001, frequency is 31.37kHz - no prescaling                     0.0625 uS per tick
    //    If TCCRnB = xxxxx010, frequency is 3.92 kHz - prescaler 8 (clock/8)             0.5 uS per tick
    //    If TCCRnB = xxxxx011, frequency is 490   Hz - prescaler 64 (Arduino default)    4 uS per tick
    //    If TCCRnB = xxxxx100, frequency is 123   Hz - prescaler 256                     16 uS per tick
    //    If TCCRnB = xxxxx101, frequency is 31    Hz - prescaler 1024                    64 uS per tick
    //    If TCCRnB = xxxxx11x, external clock source, we don't care about. 
    //
    // PWM frequency can be adjusted even more precisely by setting a custom TOP value - for example the Scout ESC uses a custom TOP
    // to generate PWM near 20KHz, but it has an odd duty cycle range of 0-381. We can do the same here if we find 31KHz creates too much heat
    // but the turret motors are low current and in testing so far it has not been an issue. The convenient aspect of this arrangement is that our
    // duty cycle is 0-255 which is the same as our motor speed range. 
    
    // Several settings need to be made 
    // Waveform Generation Mode bits (WGM): split across TCCRnA and TCCRnB. These set the PWM modes, there are three possible (in addition to Normal): 
    //      Fast PWM, Phase Correct PWM, or Phase and Frequency Correct PWM. We will use Phase Correct where TOP is 255
    // Clock Select: three bits which control the prescaler, we want prescaler of 1 (none) to achieve the frequency shown above. Prescaler is set with last three bits of 
    //      TCCRnB (CSn2, CSn1, CSn0)
    // Compare Match Output A,B, C Mode bits (COMnA,B,C): these enable/disable/invert outputs A,B,C
    //      Two bits each, in TCCRnA register
    // To control the duty cycle, we write to OCRnA,B,C a value between 0-255
    
    // So for Timer 5, here is what we need
    // Phase Correct PWM, TOP = 0x00FF (255): WGM53:0 = 0001
    // Prescaler = 1 (no prescaler): CS52:0 = 001
    // COM5A1:0, COM5B1:0, COM5C1:0 = 10 (non-inverted PWM output on Arduino pins 46, 45, 44 / Atmega pins 38, 39, 40)
    // We also clear all Timer 5 flags (by setting bits to 1 in TIFR5 register)
    // And we disable all Timer 5 interrupts (by clearing bits in TIMSK5). PWM is generated in hardware and we don't need interrupts enabled for it. 
    // Finaly, we start off with duty cycle for all three motors at 0 for safety.

    // Then to set duty cycle (motor speed), use the three compare registers:
    //OCR5A = 0-255;    // To set duty cycle on Arduino pin 46
    //OCR5B = 0-255;    // To set duty cycle on Arduino pin 45
    //OCR5C = 0-255;    // To set duty cycle on Arduino pin 44

    #define MOTOR_PWM_TOP 255       // In case we decide to change it later, we won't have to modify the Onboard_ESC object since it will refer to this define.
    #define SetupTimer5() ({ \  
        TCCR5A = 0xA9;       \
        TCCR5B = 0x01;       \
        TIFR5 =  0x2F;       \
        TIMSK5 = 0x00;       \
        TCNT5 = 0;           \
        OCR5A = 0;           \
        OCR5B = 0;           \
        OCR5C = 0;  })


// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// SIMPLE TIMER
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // We use the OP_SimpleTimer class for convenient timing functions throughout the project, it is a modified and improved version 
    // of SimpleTimer: http://playground.arduino.cc/Code/SimpleTimer
    // The class needs to know how many simultaneous timers may be active at any one time. We don't want this number too low or operation will be eratic, 
    // but setting it too high will waste RAM. Each additional slot costs 19 bytes of global RAM. 

    // Our best estimate as of 9/22/2016 (version 00.91.06) is: 
    // Main Sketch:     7       At least 14 slots but shouldn't be more than 7 active at any one time
    // OP_Radio:        3       4 slots but only up to 3 utilized at one time (radio detect, watchdog, elevation & azimuth ignore timers)
    // OP_Tank:         11      At least 15 slots but shouldn't be possible for more than 11 to be active at one time
    // OP_TBS (sound):  5       Prop2, Prop3, plus 3 squeak slots, all could be active simultaneously
    //-----------------------
    // TOTAL:           26   

    #define MAX_SIMPLETIMER_SLOTS       30          // Based on the calculations above, this gives us a few extra slots in case we miscalculated or if we need to add more
                                                    // But any time you add more you should re-visit this list. Sometimes extra timer slots can be used that would only 
                                                    // operate at times when other timers must be inactive, so not all new timers require the creation of new slots. 

// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// SERVO OUTPUTS 
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // What pulsewidth for stopping a "servo" ESC. Should be 1500. 
    #define SERVO_ESC_STOP              1500
    
    // We need to set which of the eight servo outputs will be used for drive motors (left and right) and turret motors, amongst other things.
    // Not every setup will require servo outputs for drive or turret motors, but if they do, they should always map to the same servo number. 
    #define SERVONUM_LEFTTREAD          SERVO_1     // This is the left-most servo output on the board, looking from the top. Servo 1 on the TCB board (Arduino 22, ATmega A0)
    #define SERVONUM_RIGHTTREAD         SERVO_2     // Servo 2 is right tread (Arduino 23, ATmega A1)
    #define SERVONUM_TURRETROTATION     SERVO_3     // Servo 3 is turret rotation (Arduino 24, ATmega A2)
    #define SERVONUM_TURRETELEVATION    SERVO_4     // Servo 4 is turret elevation (Arduino 25, ATmega A3)
    #define SERVONUM_RECOIL             SERVO_5     // Servo 5 is barrel recoil. Will also recoil even if using a mechanical recoil. (Arduino 26, ATmega A4)
    #define SERVONUM_PROP3              SERVO_6     // Proportional 3 - used for the 12-sound trigger switch of the TBS Mini (and maybe something else for future sound devices) (Arduino 27, ATmega A5)
    #define SERVONUM_PROP2              SERVO_7     // Proportional 2 - used as an engine on/off and beep trigger for the TBS Mini (and maybe something else for future sound devices) (Arduino 28, ATmega A6)
    #define SERVONUM_PROP1              SERVO_8     // Proportional 1 - used as throttle output to sound device (Arduino 29, ATmega A7)


// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// MOTOR DRIVERS - SERIAL
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // Multiple Open Panzer Scout ESCs: 
    // The Scout ESC has two possible addresses set by a switch on the device.
    // We use Address A for the drive motors, and B for the turret motors.
    // The Scout uses the same protocol as the Dimension Engineering Sabertooth controllers, and these addresses are valid Sabertooth 
    // addresses, just not the same Sabertooth addresses we use for the TCB. 
    #define OPScout_DRIVE_Address       131     // Set Address switch to Position A
    #define OPScout_TURRET_Address      132     // Set Address switch to Position B
    
    // Multiple Sabertooth devices:
    // Sabertooth ESCs have address values from 128-135 set by dipswitches on the device
    // We use address 128 (default) for the drive motors, 135 for turret motors
    #define Sabertooth_DRIVE_Address    128     // On the Sabertooth, set dipswitches 1-2 OFF, 3-6 ON
    #define Sabertooth_TURRET_Address   135     // On the Sabertooth, set dipswitche 3 ON, all the others OFF

    // Multiple Pololu Qik devices:
    // Pololu Qik ESCs have address (they call it Device ID) values from 0-127
    // They ship with DeviceID = 10 as default
    // We use DeviceID 10 (default) for the drive motors, 13 for turret motors
    #define Pololu_DRIVE_ID             10  
    #define Pololu_TURRET_ID            13  

    // Serial refresh rate
    // Unlike RC signals which repeat many times per second even if the command doesn't change, serial controllers will maintain their last speed command indefinitely.
    // Although unlikely, it is possible a speed command could be sent to a serial controller, and then the cable become disconnected, or some other issue on the master
    // device occur, so that a later stop command is never received. In this case the serial controller will maintain the last speed forever. On the Scout ESC we have
    // the option of enabling a serial watchdog that will automatically stop the motors if no serial command is received within a specific length of time, which we set here. 
    // Some but not all Sabertooth devices also have a timeout function (2x12, 2x25 V2, 2x60), but others (2x5, 2x10, 2x25 V1) do not.     
    #define OPScout_WatchdogTimeout_mS    1000  // in milliseconds
    #define Sabertooth_WatchdogTimeout_mS 1000  // in milliseconds
    
    // Now, to save processing time the TCB usually does not repeat serial commands until the command has changed, but that will no lnoger work. The consequence of a serial
    // watchdog is that we need to routinely send serial commands to the serial controller even if the command hasn't changed from the last time, in order to keep it going. 
    // The time between repeat commands must be less than the serial watchdog timeout: 
    #define MotorSignal_Repeat_mS        500     // in milliseconds
    
    // We set these values fairly large. We are protecting against a case that is highly unlikely to occur. And although 1 second is a long time for a processor, it is plenty
    // fast enough for a model to stop in the event of a disconnected cable. 
    
    // Final note: Pololu controllers don't implement a serial watchdog, and as mentioned some Sabertooth don't either, but it won't hurt them to receive the same signal repeated at routine intervals.
   

// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// MOTOR DRIVERS - GENERAL
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // THESE ARE THE EXTERNAL SPEEDS THAT WILL BE PASSED FROM THE SKETCH TO THE CONTROLLER CLASSES
    // The internal speed ranges (meaning, the ranges used internally by the different controllers) can vary depending on 
    // the type of the controller, and are set in their respective begin() functions
    // We use the same numbers for drive, turret, and smoker. What this number basically sets, is the maximum motor output resolution. 
    // YOU SHOULDN'T NEED TO MODIFY THIS EVER!
    #define MOTOR_MAX_FWDSPEED           255
    #define MOTOR_MAX_REVSPEED          -255
    #define MOTOR_MAX_FWDSPEED_DBL       255.0
    #define MOTOR_MAX_REVSPEED_DBL      -255.0


// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// MOTOR DRIVERS - ONBOARD  (OB for "OnBoard")
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // These are defines for the onboard L298 dual motor driver chip and the smoker MOSFET.
    // The L298 is driven with 6 pins, two each directional pins and one PWM for each motor. The smoker MOSFET is uni-directional so only needs PWM.
    // Motor A
    #define OB_MA1                      10  // This is the Arduino pin for MotorA, direction pin 1 (Atmega pin 23, PortB4)
    #define OB_MA2                      6   // This is the Arduino pin for MotorA, direction pin 2 (Atmega pin 15, PortH3)
    #define OB_MA_PWM                   45  // This is the Arduino pin for MotorA PWM (Atmega pin 39, PortL4)
    #define OB_MA_OCR                OCR5B  // The output compare register associated with this pin
    // Motor B
    #define OB_MB1                      13  // This is the Arduino pin for MotorB, direction pin 1 (Atmega pin 26, PortB7)
    #define OB_MB2                      11  // This is the Arduino pin for MotorB, direction pin 2 (Atmega pin 24, PortB5)
    #define OB_MB_PWM                   46  // This is the Arduino pin for MotorB PWM (Atmega pin 38, PortL3)
    #define OB_MB_OCR                OCR5A  // The output compare register associated with this pin
    // Heng Long Smoker (we will call it SIDEA, there is only one)
    #define OB_SMOKER_PWM               44  // This is the Arduino pin for the smoker mosfet PWM pin (Atmega pin 25, PortB6)
    #define OB_SMOKER_OCR            OCR5C  // The output compare register associated with this pin
    

// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// PINS! 
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>

    // Analog Pins
        #define IOA                      0        // Number of I/O A for use in arrays
        #define IOB                      1        // Number of I/O B for use in arrays
        #define pin_IO_A                 A9       // Input/Output - for future expansion. Can read analog voltage, digital i/o, and pin-change interrupt capability (ATmega K1)
        #define pin_IO_B                 A8       // Input/Output - for future expansion. Can read analog voltage, digital i/o, and pin-change interrupt capability (ATmega K0)
        #define IO_port                  DDRK     // What Atmega port are the IO pins on (Port K on TCB)
        #define IO_A_bit                 1        // What bit in port_IO is IO "A"
        #define IO_B_bit                 0        // What bit in port_IO is IO "B"
        #define pin_BattVoltage          A15      // Input    - Battery voltage monitor through voltage divider (1/3 scaling ratio) (ATmega K7)

    // Dipswitch pins
        #define pin_Dip1                 A6       // Input    - Dipswitch 1 (ATmega F6)
        #define pin_Dip2                 A5       // Input    - Dipswitch 2 (ATmega F5)
        #define pin_Dip3                 A10      // Input    - Dipswitch 3 (ATmega K2)
        #define pin_Dip4                 A11      // Input    - Dipswitch 4 (ATmega K3)
        #define pin_Dip5                 A12      // Input    - Dipswitch 5 (ATmega K4)
    
    // Repair selection switch
        #define pin_RepairTank           33       // Input    - Repair tank selection switch (ATmega C4)

    // Pushbutton
        #define pin_Button               A14      // Input    - Input pushbutton (ATmega K6)

    // Board LEDs
        #define pin_RedLED               A3       // Output   - Red   LED on board (ATmega F3)
        #define pin_GreenLED             A4       // Output   - Green LED on board (ATmega F4)

    // Transistorized Light outputs
        #define pin_Light1               53       // Output   - Light #1 output (headlights) (ATmega B0)
        #define pin_Light2               5        // Output   - Light #2 output (ATmega E3)
        #define pin_Brakelights          4        // Output   - Brake light output. PWM capable. (ATmega G5)
        #define pin_AuxOutput            8        // Output   - Aux output. PWM capable. Has a flyback diode installed, this special output can drive a relay directly, or even a small motor (ATmega H5)
        #define pin_MuzzleFlash          41       // Output   - Trigger output for Taigen High Intensity muzzle flash unit (ATmega G0)
        #define pin_HitNotifyLEDs        7        // Output   - Hit notification LEDs if using the Tamiya apple. PWM capable (ATmega H4)
        //Machine Gun light output - slightly different. This is on Atmega pin 4, but it does not have an Arduino pin number! 
        // Therefore we have to manipulate the port directly, we can't use pinMode() or digitalWrite() functions
        #define MG_PORT                  PORTE    // Port E
        #define MG_DDR                   DDRE     // Data direction register for Port E 
        #define MG_PORTPIN               PE2      // The specific port pin for the machine gun LED (ATmega E2)

        
    // Mechanical Recoil 
        #define pin_MechRecoilMotor      43       // Output   - Controls the mechanical recoil motor (on/off) (ATmega L6)
        
    // OTHER PINS
        // Onboard motor driver pins are defined above in the "MOTOR DRIVERS - ONBOARD" section
        // - Onboard motor driver direction and PWM pins (6 pins total)
        // - Heng Long smoker output

    // The remaining pins are defined in their respective libraries: 
        // - 8 Servos (top of OP_Servo.cpp - all 8 PORTA pins (Arduino 22-29, Atmega 71-78))
        // - PPM input pin (top of OP_PPM.h - Arduino Pin 3/INT1, but in Atmega terms it is Pin 7/INT5)
        // - Mechanical recoil switch (top of OP_Tank.h - Atmega Pin 8/INT6 - not connected on Arduinos!)
        // - IR sending and receiving pins (OP_IRLib library)
        // - Hardware Serial, SPI, and I2C ports (defined by Arduino)




#endif
