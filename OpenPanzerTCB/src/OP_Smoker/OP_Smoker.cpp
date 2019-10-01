/* OP_Smoker.cpp     Library handling various types of model smokers
 * Source:           openpanzer.org      
 * Authors:          Luke Middleton
 *   
 *
 * LICENSE
 * ===============================================================================================================
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
 
#include "OP_Smoker.h"

// Return a character string of the name of the smoker type, used for printing
const __FlashStringHelper *ptrSmokerType(Smoker_t sType) {
  if(sType>LAST_SMOKERTYPE) sType=LAST_SMOKERTYPE+1;
  const __FlashStringHelper *Names[LAST_SMOKERTYPE+2]={
  F("Combined Heat & Fan"), 
  F("Separate Heat & Fan"), 
  F("Experimental"), 
  F("Unknown")};
  return Names[sType];
};


void OP_Smoker::begin(void)
{

    // Set the internal speed range (min, max, middle). We are using 8 bit PWM so the values are 0-255
    // The smoker output has no reverse, so minimum is always 0. We also set middle to 0 so the full range is only one-sided.
    // One complication involves the fact that some users may actually want the smoke to be less at full speed, and more at idle, 
    // which is somewhat backwards from the normal operation, but could be considered more realistic. 
    // The result is that we can't blindly set MaxSpeed (the speed of the smoker at max throttle) to the max of the range, because
    // in face Idle may be faster. So we do a check of all three fan values (Idle, FastIdle, and MaxSpeed) and use the one that is the 
    // greatest for our internal maximum level. 
    // In practice, the map_Range function will then take care of dealing with increasing or decreasing speed relative to idle. 
        int s;
        if      (MaxSpeed > Idle     && MaxSpeed > FastIdle)    s = MaxSpeed;
        else if (FastIdle > MaxSpeed && FastIdle > Idle)        s = FastIdle;
        else if (Idle     > MaxSpeed && Idle     > FastIdle)    s = Idle;
        // We now have the maximum speed the smoker fan can turn, set our range. 
        set_InternalSpeedRange(0, s, 0);
        set_DefaultInternalSpeedRange(0, s, 0);

    // Now do the same thing for the heat range
        if      (HeatAmtMax      > HeatAmtIdle && HeatAmtMax      > HeatAmtFastIdle)    s = HeatAmtMax;
        else if (HeatAmtFastIdle > HeatAmtMax  && HeatAmtFastIdle > HeatAmtIdle)        s = HeatAmtFastIdle;
        else if (HeatAmtIdle     > HeatAmtMax  && HeatAmtIdle     > HeatAmtFastIdle)    s = HeatAmtIdle;
        set_InternalHeatRange(0, s, 0);
        set_DefaultInternalHeatRange(0, s, 0);

    // Smoker effect - none right now
        smoker_effect = NONE;

    // Reversed - no
        reversed = false;

    switch (SmokerType)
    {
        case SMOKERTYPE_ONBOARD_STANDARD:
            // In this case both the fan and heat are controlled by a single source, which is the SMOKER output
            // The onboard smoker controller uses Timer5 for PWM. See OP_Settings.h for details. 
            pinMode (OB_SMOKER_PWM, OUTPUT); 
            OB_SMOKER_OCR = 0;              // For safety's sake, set the output duty cycle to 0 before starting. This should also already have been done in SetupTimer5()
            break;
        
        case SMOKERTYPE_ONBOARD_SEPARATE:
            // In this case the fan and heat are controlled separately. Fan will now move to the AUX output, and the heating element will be
            // on the SMOKER output
            // Heat - same setup as above
            pinMode (OB_SMOKER_PWM, OUTPUT); 
            OB_SMOKER_OCR = 0;              // For safety's sake, set the output duty cycle to 0 before starting. This should also already have been done in SetupTimer5()
            // Fan - aux output
            pinMode(pin_AuxOutput, OUTPUT);
            analogWrite(pin_AuxOutput, 0);  // For safety's sake, set the output duty cycle to 0 before starting. 
            break;
        
        case SMOKERTYPE_SERIAL:

            // Initialize smoker serial

            // Baud Rates
            // The Serial Smoker should always initializes to a baud rate of 38400. It can function at any baud rate up to 115200 but in order to command it to a different baud, we need to tell it 
            // to change and that message needs to go at 38400. 
            if (*smokerBaud != 38400)           // If the desired rate is 38400 we know the Serial Smoker already booted to that, so no change is needed. Otherwise proceed. 
            {
                smokerPort->begin(38400);       // Temporarily change serial motor baud rate on the TCB to 38400, so we can communicate with the Scout.
                delay(20);
                switch (*smokerBaud)             // Now at a rate of 38400, tell the Serial Smoker what baud rate we want it to actually go to. 
                {
                    case 2400:   OP_Smoker::command(SMOKER_CMD_BAUD_RATE,SMOKER_BAUD_CODE_2400);   break;
                    case 9600:   OP_Smoker::command(SMOKER_CMD_BAUD_RATE,SMOKER_BAUD_CODE_9600);   break;
                    case 19200:  OP_Smoker::command(SMOKER_CMD_BAUD_RATE,SMOKER_BAUD_CODE_19200);  break;
                  //case 38400:  OP_Smoker::command(SMOKER_CMD_BAUD_RATE,SMOKER_BAUD_CODE_38400);  break; // This case is unnecessary because we already checked above to make see if the baud was anything but 38400
                    case 57600:  OP_Smoker::command(SMOKER_CMD_BAUD_RATE,SMOKER_BAUD_CODE_57600);  break;
                    case 115200: OP_Smoker::command(SMOKER_CMD_BAUD_RATE,SMOKER_BAUD_CODE_115200); break;
                }
                smokerPort->flush();            // Wait for transmission to complete
                smokerPort->begin(*smokerBaud); // Now return the TCB to our originally-desired baud rate (which the Serial Smoker should also have switched to)
                delay(20);                      // Give the Serial Smoker time to change rates
            }                                   // We can now proceed at the new rate and the Serial Smoker should be on the same page

            // SerialWatchdog 
            // We want to enable the serial timeout feature on the Serial Smoker. The function for converting watchdog time to command data is Value = desired time in mS / 100 
            OP_Smoker::command(SMOKER_CMD_SERIAL_WATCHDOG, (OP_Smoker_WatchdogTimeout_mS / 100));    

            break;
    }
}

void OP_Smoker::setSpeed(int s)
{
    int f; 
    int h; 
    
    clearSmokerEffect();                    // Clear any special effect - they only run when manual commands are not being given

    switch (SmokerType)
    {
        case SMOKERTYPE_ONBOARD_STANDARD:    
            f = map_RangeFan(abs(s));       // Make sure we are using the internal range. We don't do reverse so negative values are converted to positive with abs()
            
            // Set the PWM duty cycle of the combined fan and heat output
            OB_SMOKER_OCR = f;              // OB_SMOKER_OCR is defined in OP_Settings.h
            curspeed = f;                   // Save the speed, it is the same for fan and heat
            curheat = f;
            break;
        
        case SMOKERTYPE_ONBOARD_SEPARATE:
            f = map_RangeFan(abs(s));       // Make sure we are using the internal range. We don't do reverse so negative values are converted to positive with abs()
            h = map_RangeHeat(abs(s));      // Now do heat

            // Set the PWM duty cycle of the individual fan and heat outputs
            analogWrite(pin_AuxOutput, f);  // Fan - Aux output
            OB_SMOKER_OCR = h;              // Heat- Smoker output (OB_SMOKER_OCR is defined in OP_Settings.h)
            curspeed = f;                   // Fan - speed
            curheat = h;                    // Heat - level
            break;
            
        case SMOKERTYPE_SERIAL:
            f = map_RangeFan(abs(s));       // Make sure we are using the internal range. We don't do reverse so negative values are converted to positive with abs()
            h = map_RangeHeat(abs(s));      // Now do heat
            
            // Send the serial commands for the individual fan and heat outputs
            OP_Smoker::setLevelSerial(SMOKER_CMD_FAN_SPEED, f);
            OP_Smoker::setLevelSerial(SMOKER_CMD_HEATER_LEVEL, h);
            curspeed = f;
            curheat = h;
            break;
    }       

    LastUpdate_mS = millis();               // Save the time
}

void OP_Smoker::setSpeed_wEffect(int s)
{   // The only difference here from setSpeed is that we don't clear the smoker effect, and we also don't call this function publicly, 
    // only privately
    int f; 
    int h; 

    switch (SmokerType)
    {
        case SMOKERTYPE_ONBOARD_STANDARD:    
            f = map_RangeFan(abs(s));       // Make sure we are using the internal range. We don't do reverse so negative values are converted to positive with abs()
            
            // Set the PWM duty cycle of the combined fan and heat output
            OB_SMOKER_OCR = f;              // OB_SMOKER_OCR is defined in OP_Settings.h
            curspeed = f;                   // Save the speed, it is the same for fan and heat
            curheat = f;
            break;
        
        case SMOKERTYPE_ONBOARD_SEPARATE:
            f = map_RangeFan(abs(s));       // Make sure we are using the internal range. We don't do reverse so negative values are converted to positive with abs()
            smoker_effect == SHUTDOWN ? h = 0 : h = map_RangeHeat(abs(s)); // Now do heat, except heat always goes directly to 0 if we are in the shutdown effect

            // Set the PWM duty cycle of the individual fan and heat outputs
            analogWrite(pin_AuxOutput, f);  // Fan - Aux output
            OB_SMOKER_OCR = h;              // Heat- Smoker output (OB_SMOKER_OCR is defined in OP_Settings.h)
            curspeed = f;                   // Fan - speed
            curheat = h;                    // Heat - level
            break;
            
        case SMOKERTYPE_SERIAL:
            f = map_RangeFan(abs(s));       // Make sure we are using the internal range. We don't do reverse so negative values are converted to positive with abs()
            smoker_effect == SHUTDOWN ? h = 0 : h = map_RangeHeat(abs(s)); // Now do heat, except heat always goes directly to 0 if we are in the shutdown effect

            // Send the serial commands for hte individual fan and heat outputs 
            OP_Smoker::setLevelSerial(SMOKER_CMD_FAN_SPEED, f);
            OP_Smoker::setLevelSerial(SMOKER_CMD_HEATER_LEVEL, h);
            curspeed = f;
            curheat = h;
            break;
    }       

    LastUpdate_mS = millis();               // Save the time
}

void OP_Smoker::stop(void)
{                           
    clearSmokerEffect();                    // Clear any special effect - they only run when manual commands are not being given
    
    restore_Speed();                        // For safety's sake, we restore our internal range to default values, where the minimum possible speed is 0 (off)
                                            // This restores both the fan and heater ranges
    switch (SmokerType)
    {
        case SMOKERTYPE_ONBOARD_STANDARD: 
            OB_SMOKER_OCR = 0;              // Now we explicitly set duty cycle to 0, pin low - output is off. 
            curspeed = 0;                   // Save current speed, it is the same for fan and heat
            curheat = 0;
            break;
        
        case SMOKERTYPE_ONBOARD_SEPARATE:
            analogWrite(pin_AuxOutput, 0);  // Fan - Aux output - off
            OB_SMOKER_OCR = 0;              // Heat- Smoker output - off
            curspeed = 0;                   // Fan - speed = 0
            curheat = 0;                    // Heat - level = 0
            break;

        case SMOKERTYPE_SERIAL:
            OP_Smoker::setLevelSerial(SMOKER_CMD_FAN_SPEED, 0);
            OP_Smoker::setLevelSerial(SMOKER_CMD_HEATER_LEVEL, 0);
            curspeed = 0;
            curheat = 0;
            break;        
    }

    LastUpdate_mS = millis();               // Save the time    
}

void OP_Smoker::preHeat(void)
{
    // Here we turn the heating element to full on
    clearSmokerEffect();                    // Clear any special effect - they only run when manual commands are not being given

    switch (SmokerType)
    {
        case SMOKERTYPE_ONBOARD_STANDARD:    
            // This case should not happen - we only permit pre-heat for smokers that have seperate control of the heating element and fan.
            break;
        
        case SMOKERTYPE_ONBOARD_SEPARATE:
            // Set the heater to full on
            OB_SMOKER_OCR = MOTOR_MAX_FWDSPEED; // Heat- Smoker output (OB_SMOKER_OCR is defined in OP_Settings.h)
            curheat = MOTOR_MAX_FWDSPEED;   // Heat - level
            break;
            
        case SMOKERTYPE_SERIAL:
            // Send the serial command to turn on the heater
            OP_Smoker::setLevelSerial(SMOKER_CMD_HEATER_LEVEL, MOTOR_MAX_FWDSPEED);
            curheat = MOTOR_MAX_FWDSPEED;
            break;
    }       

    LastUpdate_mS = millis();               // Save the time    
}

void OP_Smoker::Startup(boolean engaged)
{
    clearSmokerEffect();                    // Clear any special effect - they only run when manual commands are not being given

    // For serial smokers, send the startup command so it knows to handle any special startup effect
    if (SmokerType == SMOKERTYPE_SERIAL) OP_Smoker::command(SMOKER_CMD_STARTUP); 

    // And for all cases, set the idle speed depending on the transmission engaged status
    engaged ? OP_Smoker::setFastIdle() : OP_Smoker::setIdle();

    LastUpdate_mS = millis();               // Save the time    
}

void OP_Smoker::setIdle(void)
{
    clearSmokerEffect();                    // Clear any special effect - they only run when manual commands are not being given
    
    // When idling, we want the minimum speed (speed at input of 0) to be IdleSpeed
    // The easiest way to do this is just change the internal range. 
    this->set_InternalSpeedRange(Idle, MaxSpeed, Idle);
    this->set_InternalHeatRange(HeatAmtIdle, HeatAmtMax, HeatAmtIdle);
    this->setSpeed(0);

    LastUpdate_mS = millis();               // Save the time
}

void OP_Smoker::setFastIdle(void)
{
    clearSmokerEffect();                    // Clear any special effect - they only run when manual commands are not being given
    
    // At fast idle (idle with transmission disengaged) we want the minimum speed (speed at input of 0) to be FastIdleSpeed
    // The easiest way to do this is just change the internal range. 
    this->set_InternalSpeedRange(FastIdle, MaxSpeed, FastIdle);
    this->set_InternalHeatRange(HeatAmtFastIdle, HeatAmtMax, HeatAmtFastIdle);
    this->setSpeed(0);

    LastUpdate_mS = millis();               // Save the time
}

void OP_Smoker::Shutdown(boolean engaged)
{
    // For serial smokers we let the external device handle the shutdown effect on its own, we just give it the shutdown command
    // and set our internal variables to zero. 
    if (SmokerType == SMOKERTYPE_SERIAL) 
    {
        OP_Smoker::command(SMOKER_CMD_SHUTDOWN); 
        curspeed = 0;
        curheat = 0;
    }
    else    
    {
        // Here we are going to slowly turn off the smoker. We can't use the ramp feature of ThrottleSpeed in the main sketch because the smoker
        // treats 0 throttle as idle speed (non-zero). In other words, ThrottleSpeed can only ramp us down to idle (or fast idle, depending), but not
        // to zero. However when the user turns off the engine, smoker speed is going to have to go from idle (or fast idle) to a complete stop, and
        // we don't want this to be abrupt. That is why we've created smoker effects, though they may have other uses as well. 

        // Restore the internal range to full so we can actually get down to zero speed
        restore_Speed();    // This restores both the fan and heater ranges

        // But now we need to modify our curspeed variable so the shutdown effect knows where to start from. Under normal operation we modify the internal speed range of the 
        // smoker object, setting the minimum to something greater than zero, which becomes idle. Under that scheme, curspeed of 0 actually represents some non-zero smoker speed. 
        // Now that we are using the full speed scale range where 0 actually means 0 (stopped), we need to adjust our prior curspeed variable to the new range. The adjustment 
        // depends on whether the transmission was engaged or not. We don't need to do this to the heat level variable because that one goes straight to full off on shutdown with 
        // no gradual slowing down. 
        if (curspeed == 0)
        {
            engaged == true ? curspeed = Idle : curspeed = FastIdle; 
        }

        // Set the active effect
        smoker_effect = SHUTDOWN;
        
        // Now the sketch will poll the update() function which will take care of the effect
    }
}

void OP_Smoker::update(void)
{
    if (smoker_effect != NONE)
    {   
        if (millis() - LastUpdate_mS > smoker_update_rate_mS)
        {
            switch (smoker_effect)
            {
                case SHUTDOWN:
                    // This effect simply turns off the smoker gradually instead of all at once. Each update through we reduce the speed by 1. 
                    // In most cases we assume the user decides to turn off the engine while the tank is stopped, so already the smoker should 
                    // probably be down to idle speed, which is not very fast to begin with. Therefore even with only reducing the speed by 1 step
                    // per pass it won't take long to get to zero.
                    // As for heat, when the user wants to stop the smoker, we turn off the heater immediately (no ramping down)
                    curspeed -= 1;
                    curheat   = 0;
                    if (curspeed <= 0) 
                    {
                        OP_Smoker::stop(); // Stop the smoker - this will also clear the effect so we don't keep coming back here. 
                    }
                    else
                    {
                        OP_Smoker::setSpeed_wEffect(curspeed); // We use the private "_wEffect" version of setSpeed so it doesn't clear the effect
                    }
                    break;
                
                default:
                    break;
            }
        }
        // LastUpdate_mS will get updated through either the stop() or setSpeed_wEffect() functions called above
    }
    else 
    {
        // No effect active - but we also send a routine speed/level update if this is a serial smoker and we are nearing the watchdog timeout time
        if (SmokerType == SMOKERTYPE_SERIAL && (millis() - LastUpdate_mS > (OP_Smoker_WatchdogTimeout_mS - 100)))
        {
            OP_Smoker::setLevelSerial(SMOKER_CMD_FAN_SPEED, curspeed);
            OP_Smoker::setLevelSerial(SMOKER_CMD_HEATER_LEVEL, curheat);
            LastUpdate_mS = millis();               // Save the time    
        }
    }
}


// This function will cut motor speed by whatever percent is passed. It can be used to temporarily modify the maximum
// speed a motor can go (useful for battle damage). We can always restore the motor to whatever its full speed is by 
// calling restore_Speed();
// In this case, we take the cut and apply it equally to min and max (forward and backward - even though here we only have forward).
void OP_Smoker::cut_SpeedPct(uint8_t cut_pct)
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
void OP_Smoker::set_MaxSpeedPct(uint8_t max_pct)
{
    if (max_pct > 100) return;                          // Max speed can't exceed 100
    max_pct = constrain(max_pct, 0, 100);
    this->cut_SpeedPct(100-max_pct);
}


// This function will cut heat level by whatever percent is passed. It can be used to temporarily modify the maximum
// level the heater can go. We can always restore the heater to whatever its full level is by calling restore_Heat();
// In this case, we take the cut and apply it equally to min and max (forward and backward - even though here we only have forward).
void OP_Smoker::cut_HeatPct(uint8_t cut_pct)
{ 
    if (cut_pct == 0) return;                           // If cut is 0, then there is nothing to cut. 

    // We make sure cut is a "percent" between 1 - 100
    // The higher the number, the greater the speed will be cut
    cut_pct = constrain(cut_pct, 1, 100);
    unsigned long diff;                                 // Use a long because numbers could get big here for a moment
    diff = (this->di_maxheat - this->di_minheat);       // First we calculate the entire difference between min and max heat level
    diff = (unsigned long)cut_pct * diff;               // Next we multiply this difference by our "percent" 1-100
    diff = diff / 200;                                  // Dividing by 200 gives us half the percentage value of our heat range
    this->i_minheat = this->di_minheat + (int)diff;     // We apply half to the minimum heat range
    this->i_maxheat = this->di_maxheat - (int)diff;     // and the other half to the maximum heat range
}
// Just a different way of doing the above. Instead of passing the percent to cut, 
// we pass the max level possible, and it cuts the rest. 
void OP_Smoker::set_MaxHeatPct(uint8_t max_pct)
{
    if (max_pct > 100) return;                           // Max heat can't exceed 100
    max_pct = constrain(max_pct, 0, 100);
    this->cut_HeatPct(100-max_pct);
}


// SERIAL FUNCTIONS
// ---------------------------------------------------------------------------------------------------------------------------------------->>
void OP_Smoker::command(byte command, byte value) const
{
  smokerPort->write(SMOKER_ADDRESS);
  smokerPort->write(command);
  smokerPort->write(value);
  smokerPort->write((SMOKER_ADDRESS + command + value) & B01111111);
}

void OP_Smoker::setLevelSerial(byte output, int level) const
{
    if (output != SMOKER_CMD_FAN_SPEED && output != SMOKER_CMD_HEATER_LEVEL) { return; }
    level = constrain(level, -255, 255);
    this->command(output, (byte)abs(level));
}