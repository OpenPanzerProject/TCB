/* OP_Motors.cpp    Open Panzer Motors - a library of motor functions
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
 

#include <OP_Motors.h>


// Return a character string of the name of the weight class, used for printing
const __FlashStringHelper *ptrDriveType(Drive_t dType) {
  if(dType>LAST_DRIVE_TYPE) dType=LAST_DRIVE_TYPE+1;
  const __FlashStringHelper *Names[LAST_DRIVE_TYPE+2]={
  F("Servo 8"), 
  F("Servo 7"), 
  F("Servo 6"), 
  F("Servo 5"), 
  F("Servo 4"), 
  F("Servo 3"), 
  F("Servo 2"), 
  F("Servo 1"), 
  F("Pololu Serial"), 
  F("Sabertooth Serial"), 
  F("Open Panzer Scout Serial ESC"),
  F("Onboard Motor Driver"),
  F("Servo/ESC"), 
  F("Pan Servo"), 
  F("Recoil Servo"),
  F("Unknown")};
  return Names[dType];
};



// ------------------------------------------------------------------------------------------------------------------>>
// GENERIC MOTOR CONTROL FUNCTIONS
// ------------------------------------------------------------------------------------------------------------------>>
// This function will cut motor speed by whatever percent is passed. It can be used to temporarily modify the maximum
// speed a motor can go (useful for battle damage). We can always restore the motor to whatever its full speed is by 
// calling restore_Speed();
// In this case, we take the cut and apply it equally to min and max (forward and backward). 
void Motor::cut_SpeedPct(uint8_t cut_pct)
{ 
    if (cut_pct == 0) return;                           // If cut is 0, then there is nothing to cut. 

    // We make sure cut is a "percent" between 1 - 100
    // The higher the number, the greater the speed will be cut
    cut_pct = constrain(cut_pct, 1, 100);
    unsigned long diff;                                 // Use a long because numbers could get big here for a moment
    diff = (this->di_maxspeed - this->di_minspeed);     // First we calculate the entire difference between min and max speed
    diff = (unsigned long)cut_pct * diff;               // Next we multiply this difference by our "percent" 1-100
    diff = diff / 200;                                  // Dividing by 200 gives us half the percentage value of our speed range
    this->i_minspeed = this->di_minspeed + (int)diff;   // We apply half to the minimum speed
    this->i_maxspeed = this->di_maxspeed - (int)diff;   // and the other half to the maximum speed
}
// Just a different way of doing the above. Instead of passing the percent to cut, 
// we pass the max speed possible, and it cuts the rest. 
void Motor::set_MaxSpeedPct(uint8_t max_pct)
{
    if (max_pct > 100) return;                          // Max speed can't exceed 100
    max_pct = constrain(max_pct, 0, 100);
    this->cut_SpeedPct(100-max_pct);
}



// ------------------------------------------------------------------------------------------------------------------>>
// OPEN PANZER SCOUT ESC 
// ------------------------------------------------------------------------------------------------------------------>>
void OPScout_SerialESC::begin(void)
{
    // Initialize motor serial
    // The Scout ESC doesn't have auto-baud, so we assume the user has set the TCB (via OP Config) to the correct rate (38400) 

    // Enable SerialWatchdog with a timeout of 1/2 second
    // The function for converting watchdog time to command data is Value = (desired time in mS - 50) / 10 
    // OPScout_WatchdogTimeout_mS is defined in OP_Settings.h
    OPScout_SerialESC::command(SCOUT_CMD_ENABLE_SERIAL_WATCHDOG, (OPScout_WatchdogTimeout_mS - 50) / 10);    
    
    // Set the internal speed range (min, max). The Scout accepts speed commands from -127 to 127 with a middle point of 0
    set_InternalRange(-127,127, 0);
    set_DefaultInternalRange(-127,127, 0);
}

void OPScout_SerialESC::setSpeed(int s)
{
    // save current speed
    curspeed = s;
    
    // make sure we are using the internal range
    s = map_Range(s);
    
    if (ESC_Position == SIDEA)
    {   //SIDEA - Shown as "M1" on the Scout board
        // Use for Left tread, or turret Rotation motor
        OPScout_SerialESC::motor(1, s);
    }
    else if (ESC_Position == SIDEB)
    {   //SIDEB - Shown as "M2" on Scout board
        // Use for Right tread, or turret Elevation motor
        OPScout_SerialESC::motor(2, s);
    }
    
    LastUpdate_mS = millis();           // Save the time
}    

void OPScout_SerialESC::stop(void)
{
    curspeed = 0;
    OPScout_SerialESC::allStop();
    LastUpdate_mS = millis();           // Save the time
}

void OPScout_SerialESC::update(void)
{
    if (millis() - LastUpdate_mS > MotorSignal_Repeat_mS)
    {
        OPScout_SerialESC::setSpeed(curspeed);
    }
}



// ------------------------------------------------------------------------------------------------------------------>>
// DIMENSION ENGINEERING SABERTOOTH CONTROLLERS
// ------------------------------------------------------------------------------------------------------------------>>
boolean Sabertooth_SerialESC::sendAutobaud = false;     // Declare static variables globally

void Sabertooth_SerialESC::begin(void)
{
    // Initialize motor serial
    // For the 2x5, valid baud rates for packetized serial are 2400, 9600, 19200, and 38400. Some of the other controllers (2x60 for example)
    // can also work at 115200. For "simple serial" the baud rate can be set manually with dip-switches on the Sabertooth device, but 
    // we are not using "simple serial" mode, we are using packetized serial. Those same switches in packetized serial mode now set the address! 
        
    // 9600 is the default baud rate for Sabertooth packet serial for all their products, however, it does not work well with Arduino! 
    // 38400 works fine in testing. 19200 may also work but faster is better. 
    // For purposes of Open Panzer, 38400 is the recommended baud rate (for all serial controllers).
        if (!sendAutobaud)  // We might have multiple Sabertooth objects, but we only need to do this part once. 
        {
            Sabertooth_SerialESC::autobaud(true);   //The "true" means skip the long waiting. 
            sendAutobaud = true;
        }
        // Send the autobaud command to the Sabertooth controller(s).
         // NOTE: *Not all* Sabertooth controllers need this command.
         //       It doesn't hurt anything, but V2 controllers use an
         //       EEPROM setting (changeable with the function setBaudRate) to set
         //       the baud rate instead of detecting with autobaud.
         //       If you have a 2x12, 2x25 V2, 2x60 or SyRen 50, you can remove
         //       the autobaud line and save yourself two seconds of startup delay.
         // But most will use the 2x5 so keep it in. You can pass "true" as the dontWait parameter and the startup will go faster

    // Set the internal speed range (min, max). Sabertooth serial devices using packetized serial
    // accept commands from -127 to 127 with a middle point of 0
    set_InternalRange(-127,127, 0);
    set_DefaultInternalRange(-127,127, 0);
}

void Sabertooth_SerialESC::setSpeed(int s)  
{
    // save current speed
    curspeed = s;
    
    // make sure we are using the internal range
    s = map_Range(s);
    
    if (ESC_Position == SIDEA)
    {   //SIDEA - Shown as "M1" on Sabertooth board
        // Use for Left tread, or turret Rotation motor
        Sabertooth_SerialESC::motor(1, s);
    }
    else if (ESC_Position == SIDEB)
    {   //SIDEB - Shown as "M2" on Sabertooth board
        // Use for Right tread, or turret Elevation motor
        Sabertooth_SerialESC::motor(2, s);
    }
    
    LastUpdate_mS = millis();           // Save the time
}

void Sabertooth_SerialESC::stop(void)
{
    curspeed = 0;
    Sabertooth_SerialESC::allStop();
    LastUpdate_mS = millis();           // Save the time
}

void Sabertooth_SerialESC::update(void)
{
    if (millis() - LastUpdate_mS > MotorSignal_Repeat_mS)
    {
        Sabertooth_SerialESC::setSpeed(curspeed);
    }
}



// ------------------------------------------------------------------------------------------------------------------>>
// POLOLU QIK CONTROLLERS
// ------------------------------------------------------------------------------------------------------------------>>
boolean Pololu_SerialESC::sendAutobaud = false;     // Declare static variables globally
void Pololu_SerialESC::begin(void)
{
    // Initialize motor serial
    // Pololu controllers can work at baud rates from 1200 to 115,200. The baud rate can be hard-set manually by using one of the baud 
    // jumpers. On the 2s12v10 the jumper can select 9600, 38400, or 115200. On the 2s9v1, the jumper will only select 38400. 
    // For purposes of Open Panzer, 38400 is the recommended baud rate (for all serial controllers).
    // It is easier to tell the user not to worry about jumpers, and we will just send the autobaud command. 
    if (!sendAutobaud)  // We might have multiple Pololu objects, but we only need to do this part once. 
    {
        Pololu_SerialESC::autobaud(true);  // This simply sends 0xAA. The "true" means skip the long waiting. 
        sendAutobaud = true;
    }

    // Set the internal speed range (min, max). Pololu Qik devices in 7-bit mode accept commands from -127 to 127
    // Although the Pololu Qik controllers also have an 8 bit mode, we use 7-bit for consistency with the Sabertooth,
    // and also because it affords us the highest PWM frequncy (19.7kHz, which is ultrasonic).
    // If you use 8-bit mode on the Qiks the highest PWM frequency you can achieve is 9.8kHz
    set_InternalRange(-127,127, 0);         // Range of -127 to 127 with middle of 0
    set_DefaultInternalRange(-127,127, 0);
}

void Pololu_SerialESC::setSpeed(int s)  
{
    // save current speed
    curspeed = s;
    
    // make sure we are using the internal range
    s = map_Range(s);

    if (ESC_Position == SIDEA)
    {   //SIDEA - Shown as "M0" on Pololu board
        // Use for Left tread, or turret Rotation motor
        Pololu_SerialESC::motor(1, s);
    }
    else if (ESC_Position == SIDEB)
    {   //SIDEB - Shown as "M1" on Pololu board
        // Use for Right tread, or turret Elevation motor
        Pololu_SerialESC::motor(2, s);
    }
    
    LastUpdate_mS = millis();           // Save the time
}

void Pololu_SerialESC::stop(void)
{
    curspeed = 0;
    Pololu_SerialESC::allStop();
    LastUpdate_mS = millis();           // Save the time
}

void Pololu_SerialESC::update(void)
{
    if (millis() - LastUpdate_mS > MotorSignal_Repeat_mS)
    {
        Pololu_SerialESC::setSpeed(curspeed);
    }
}



// ------------------------------------------------------------------------------------------------------------------>>
// ONBOARD MOTOR CONTROLLERS A & B
// ------------------------------------------------------------------------------------------------------------------>>
void Onboard_ESC::begin(void)
{
    // Set the internal speed range (min, max, middle), this is defined by the TOP value for our PWM calculations, see OP_Settings.h 
        set_InternalRange(-MOTOR_PWM_TOP, MOTOR_PWM_TOP, 0);
        set_DefaultInternalRange(-MOTOR_PWM_TOP, MOTOR_PWM_TOP, 0);

    // The onboard motor controller uses Timer5 for PWM. See OP_Settings.h for details. 

    // CONTROL PINS
        pinMode (OB_MA1,    OUTPUT); 
        pinMode (OB_MA2,    OUTPUT); 
        pinMode (OB_MA_PWM, OUTPUT); 
        pinMode (OB_MB1,    OUTPUT); 
        pinMode (OB_MB2,    OUTPUT); 
        pinMode (OB_MB_PWM, OUTPUT); 

    // PWM
        if (ESC_Position == SIDEA)
        {   // For safety's sake, set the output duty cycle to 0 before starting. This should also already have been done in SetupTimer5()
            OB_MA_OCR = 0;          
        }
        if (ESC_Position == SIDEB)
        {
            OB_MB_OCR = 0;          
        }
}

void Onboard_ESC::setSpeed(int s)
{
    // save current speed
    curspeed = s;

    // make sure we are using the internal range
    s = map_Range(s);
    
    // According to the L298 datasheet: 
    // if s = 0, then the L298 doesn't care what the two input pins are, the motor is allowed to freewheel
    // if s > 0, then the two input pins should be opposite values of each other (swap to go forward or reverse)
    // if s > 0, and two input pins equal each other (either both high, or both low), then the motor is set to hard stop
    // We are not utilizing this last feature, but could.
    // Note that "s" here is speed but is actually a PWM duty cycle applied to the EN (enable) pin of the motor driver. 
    // However according to the Atmega2560 datasheet (pg 156), the two extremes of the duty cycle setting are special cases:
    // If s = 0 then the pin will be set to low (for non-inverted mode, which is what we're using). In other words, no PWM. 
    // Likewise if s = MAX (whatever our TOP value is) then no PWM will be coming out the pin, it will simply be set to output high. 
    if (ESC_Position == SIDEA)
    {   //SIDEA - motor A
        if (s < 0)
        {   //Reverse
            digitalWrite(OB_MA1, LOW);
            digitalWrite(OB_MA2, HIGH);
        }
        else
        {   //Forward
            digitalWrite(OB_MA1, HIGH);
            digitalWrite(OB_MA2, LOW);
        }
        // Now set the PWM, always a positive number
        OB_MA_OCR = abs(s); // OB_MA_OCR is defined in OP_Settings.h
    }
    else if (ESC_Position == SIDEB)
    {   //SIDEB - motor B
        if (s < 0)
        {   //Reverse
            digitalWrite(OB_MB1, LOW);
            digitalWrite(OB_MB2, HIGH);
        }
        else
        {   //Forward
            digitalWrite(OB_MB1, HIGH);
            digitalWrite(OB_MB2, LOW);
        }
        // Now set the PWM, always a positive number
        OB_MB_OCR = abs(s); // OB_MB_OCR is defined in OP_Settings.h
    }   
}

void Onboard_ESC::stop(void)
{
    if (ESC_Position == SIDEA)
    {
        OB_MA_OCR = 0;
    }
    else if (ESC_Position == SIDEB)
    {
        OB_MB_OCR = 0;
    }
}



// ------------------------------------------------------------------------------------------------------------------>>
// ONBOARD SMOKER CONTROLLER
// ------------------------------------------------------------------------------------------------------------------>>
void Onboard_Smoker::begin(void)
{
    // Set the internal speed range (min, max). We are using 8 bit PWM so the values are 0-255
    // The smoker output has no reverse, so minimum is always 0. We also set middle to 0 so the full range is only one-sided.
        set_InternalRange(0,255, 0);
        set_DefaultInternalRange(0,255, 0);

    // The onboard smoker controller uses Timer5 for PWM. See OP_Settings.h for details. 

    // CONTROL PINS
        pinMode (OB_SMOKER_PWM, OUTPUT); 

    // PWM
        if (ESC_Position == SIDEA)  // We only do SIDEA for the Smoker, see ObjectSetup tab of the main sketch
        {   // For safety's sake, set the output duty cycle to 0 before starting. This should also already have been done in SetupTimer5()
            OB_SMOKER_OCR = 0;  
        }
}

void Onboard_Smoker::setSpeed(int s)
{
    // make sure we are using the internal range
    // We don't do reverse so negative values are converted to positive with abs()
    s = map_Range(abs(s));
    
    // Set the PWM duty cycle
    OB_SMOKER_OCR = (s);    // OB_SMOKER_OCR is defined in OP_Settings.h
}

void Onboard_Smoker::stop(void)
{                           // This first line is probably not necessary, but for safety's sake, 
    restore_Speed();        // we restore our internal range to default values, where the minimum possible speed is 0 (off)
    OB_SMOKER_OCR = 0;      // Now we explicitly set duty cycle to 0, pin low - output is off. 
}

void Onboard_Smoker::setIdle(void)
{
    // When idling, we want the minimum speed (speed at input of 0) to be IdleSpeed
    // The easiest way to do this is just change the internal range. 
    set_InternalRange(_IdleSpeed, 255, _IdleSpeed);
    this->setSpeed(0);
}

void Onboard_Smoker::setFastIdle(void)
{
    // At fast idle (idle with transmission disengaged) we want the minimum speed (speed at input of 0) to be FastIdleSpeed
    // The easiest way to do this is just change the internal range. 
    this->set_InternalRange(_FastIdleSpeed, 255, _FastIdleSpeed);
    this->setSpeed(0);
}



// ------------------------------------------------------------------------------------------------------------------>>
// ABSTRACT SERVO CONTROLLER
// ------------------------------------------------------------------------------------------------------------------>>
void Servo_ESC::begin(void)
{
    this->attach(ESC_Position);
    
    // Set the internal range (min, max, middle)
    set_InternalRange(1000, 2000, 1500);
    set_DefaultInternalRange(1000, 2000, 1500);
}

void Servo_ESC::setPos(int s)
{   //Set a servo's position *position*
    
    // save current speed
    curspeed = s;
    
    // set ESC to speed. In this case these are servos so we just writeMicroseconds
    // We also map the speed to our internal speed range. 
    this->writeMicroseconds(ESC_Position, map_Range(s));
}

void Servo_ESC::stop(void)
{
    curspeed = 0;
    
    this->writeMicroseconds(ESC_Position, SERVO_ESC_STOP);
}



// ------------------------------------------------------------------------------------------------------------------>>
// PAN SERVO CONTROLLER
// ------------------------------------------------------------------------------------------------------------------>>
void Servo_PAN::begin(void)
{
    this->attach(ESC_Position);
    
    // Set the internal range (min, max, middle)
    
    // In normal operation we don't set a pan servo's position directly, nor will we know exactly what its position is without querying it. 
    // Instead we set a speed and a direction, and tell it to start moving. It will only stop when it reaches the servo limit (not the motor object limit),
    // or if we give it a different speed/direction command. 
    
    // For that reason, the internal motor range isn't very useful for the pan servo, in the usual sense. We still use it,
    // but it is the range of *speeds* that we allow, from slow to fast - not the range of positions
    set_InternalRange(-SERVO_MAXRAMP_TICKSTEP, SERVO_MAXRAMP_TICKSTEP, 0);
    set_DefaultInternalRange(-SERVO_MAXRAMP_TICKSTEP, SERVO_MAXRAMP_TICKSTEP, 0);
    
    // But we can still use limits, by setting them on the servo object. 
    // We default to the same limits the servo object will already have been initialized to. 
    setLimits(SERVO_OUT_MINPULSE, SERVO_OUT_MAXPULSE);
    
    // Initialize fixed position to center
    fixedPos = SERVO_OUT_CENTERPULSE;
    canSetFixedPos = false;
}

void Servo_PAN::setPos(int s)
{   // Set a pan servo's *position*. This is a special function that isn't used in normal operation (setSpeed is the function to use
    // when we are treating this like any other motor object). But in certain special occasions we may want to directly
    // set the position. It comes in handy for example in the special menu where we adjust end-points (see the sketch, ServoSetup tab)
    
    // save current speed
    curspeed = s;
    
    // There is no range checking for this function, but the servo object will constrain anything we send it to its own limits.
    this->writeMicroseconds(ESC_Position, s);
}

void Servo_PAN::setSpeed(int s)
{
    // save current speed
    curspeed = s;
        
    // All the other classes use map_Range to map the passed speed parameter to the internal range of the motor object. 
    // But this is a unique case, we actually want to map the passed speed parameter to the range of allowable speeds we let the servo move.
    // This is not the same as the servo's endpoints, those are saved in the servo object. 
    // So here we are not writing a specific position to the servo, we are telling it how *fast* to move (pan). 
    // It will keep moving in this direction at this rate until we change the speed, stop the ramping, or it reaches the servo object limit. 
    if (s == this->e_middlespeed)
    { 
        // Tell the servo to stop moving.
        this->stopRamping(ESC_Position); 
        // Now figure out where the servo stopped, and save this position for future reference. 
        // When the user stops moving the servo (s == middlespeed) we know it is in the position where the user wants it to stay. 
        // If this pan servo is being used for barrel stabilization, we can refer to this fixedPos as the point where it needs to be stabilized to. 
        if (canSetFixedPos) 
        {
            fixedPos = this->getPulseWidth(ESC_Position); 
            canSetFixedPos = false;     // Don't allow us to set another fixed position until the servo has changed position
        }
    }
    else 
    { 
        this->setRampStepPerFrame(ESC_Position, map_Range(s)); 
        canSetFixedPos = true;
    }
}

void Servo_PAN::stop(void)
{
    curspeed = 0;
    
    // Stopping in this case is not returning to center, it is stopping at whatever position
    // we happen to be in
    this->stopRamping(ESC_Position);

    // Same as setSpeed above
    if (canSetFixedPos) 
    {
        fixedPos = this->getPulseWidth(ESC_Position); 
        canSetFixedPos = false;     // Don't allow us to set another fixed position until the servo has changed position
    }
}

int16_t Servo_PAN::PulseWidth(void)
{
    return this->getPulseWidth(ESC_Position);
}

// This sets limits on the servo object, which is useful in the case of a pan servo, since we only pass speeds to the servo object, 
// not positions. This way the servo object will stop automatically at these limits when it reaches them. 
void Servo_PAN::setLimits(uint16_t min, uint16_t max)
{
    // This will set how far the servo will travel in either direction (end-point)
    this->setMinPulseWidth(ESC_Position, min);
    this->setMaxPulseWidth(ESC_Position, max);
}



// ------------------------------------------------------------------------------------------------------------------>>
// RECOIL SERVO CONTROL
// ------------------------------------------------------------------------------------------------------------------>>
void Servo_RECOIL::begin(void)
{
    this->attach(ESC_Position);
    
    this->setupRecoil_mS(ESC_Position, _RecoilmS, _ReturnmS, _Reversed);
    
    // This servo also needs to be initialized to its end position. 
    // Because it looks cool we will ramp it to battery, although this will also block code. 
    uint16_t p; 
    if (_Reversed)
    {   // Use this instead to go straight to the end position
        //this->writeMicroseconds(ESC_Position, this->getMinPulseWidth(this->ESC_Position));              
        
        p = this->getMinPulseWidth(this->ESC_Position);
        this->setRampSpeed_mS(ESC_Position, _ReturnmS, true);
        do {delay(1);}
        while (this->getPulseWidth(ESC_Position) > p);
        this->stopRamping(ESC_Position);
        this->writeMicroseconds(ESC_Position, p);
    }
    else
    {   // Use this instead to go straight to the end position
        //this->writeMicroseconds(ESC_Position, this->getMaxPulseWidth(this->ESC_Position));

        p = this->getMaxPulseWidth(this->ESC_Position);
        this->setRampSpeed_mS(ESC_Position, _ReturnmS, false);
        do {delay(1);}
        while (this->getPulseWidth(ESC_Position) < p);
        this->stopRamping(ESC_Position);    
        this->writeMicroseconds(ESC_Position, p);
    }
   
    // We don't need to set anything else, unless the user wants to modify the endpoints
}

// We also don't need a "Recoil" method because that is exposed by being a member of the OP_Servos class. Just call
// Servo_RECOIL(object).Recoil();

void Servo_RECOIL::setSpeed(int s)
{
    // This function does nothing
    return;
}

void Servo_RECOIL::stop(void)
{
    // This function does nothing
    return;
}

// This sets limits on the servo object, rather than the motor's internal range. 
void Servo_RECOIL::setLimits(uint16_t min, uint16_t max)
{
    this->setMinPulseWidth(ESC_Position, min);
    this->setMaxPulseWidth(ESC_Position, max);
}

