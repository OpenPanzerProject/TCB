/* OP_LedHandler.cpp    Open Panzer Led Handler - class for handling LEDs including blink effects, requires use of elapsedMillis
 * Source:              openpanzer.org              
 * Authors:             Luke Middleton
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


#include "LedHandler.h"


void LedHandler::begin(byte p, boolean i /*=false*/)
{
    _pin = p;                   // Save pin number
    _invert = i;                // Save invert status
    pinMode(_pin, OUTPUT);      // Set pin to OUTPUT
    this->pinOff();             // Start with Led off
    _isBlinking = false;        // Initialize
    _curStep = 0;               //
}

void LedHandler::on(void)
{
    if (_isBlinking) this->stopBlinking();
    this->pinOn();
}

boolean LedHandler::isOn(void)
{
    _invert ? !digitalRead(_pin) : digitalRead(_pin);
}

void LedHandler::off(void)
{
    if (_isBlinking) this->stopBlinking();
    this->pinOff();
}

void LedHandler::pinOn(void)
{
    _invert ? digitalWrite(_pin, LOW) : digitalWrite(_pin, HIGH);
}

void LedHandler::pinOff(void)
{
    _invert ? digitalWrite(_pin, HIGH) : digitalWrite(_pin, LOW);
}

void LedHandler::toggle(void)
{
    digitalWrite(_pin, !digitalRead(_pin)); 
}
    
boolean LedHandler::isBlinking(void)
{
    return _isBlinking; 
}

void LedHandler::Blink(uint16_t interval /*=DEFAULT_BLINK_INTERVAL*/)
{
    this->Blink(1, interval);
}

void LedHandler::Blink(uint8_t times, uint16_t interval /*=DEFAULT_BLINK_INTERVAL*/)
{
    ClearBlinker();
    _fixedInterval = true;
    _nextWait = interval;
    _numSteps = (times * 2) - 1;        // multiply by two and minus one to add spaces between the blinks where the LED is off
    this->pinOn();                      // Start with the Led on. User needs to call the handle() function to update the next steps 
    _time = 0;
    _isBlinking = true;
}

void LedHandler::stopBlinking(void)
{
    this->pinOff();
    ClearBlinker();
}

void LedHandler::startBlinking(uint16_t on_interval, uint16_t off_interval)
{
    BlinkStream Blinker;    
    Blinker.interval[0] = on_interval;      // On
    Blinker.interval[1] = off_interval;     // Off
    Blinker.repeat = true;
    this->StreamBlink(Blinker, 2);          // 2 steps
}

void LedHandler::DoubleTap(boolean repeat /*=false*/)
{
    BlinkStream bs;
    bs.interval[0] = 50;                // On
    bs.interval[1] = 40;                // Off
    bs.interval[2] = 110;               // On
    bs.interval[3] = 500;               // Off
    bs.repeat = repeat;
    this->StreamBlink(bs, 4);           // 4 steps in the double-tap
}

void LedHandler::TripleTap(boolean repeat /*=false*/)
{
    BlinkStream bs;
    bs.interval[0] = 120;               // On
    bs.interval[1] = 180;               // Off
    bs.interval[2] = 120;               // On
    bs.interval[3] = 160;               // Off
    bs.interval[4] = 200;               // On
    bs.interval[5] = 600;               // Off
    bs.repeat = repeat;
    this->StreamBlink(bs, 6);           // 6 steps in the triple-tap
}

void LedHandler::StreamBlink(BlinkStream bs, uint8_t numSteps)
{
    ClearBlinker();
    _fixedInterval = false;
    _blinkStream = bs;
    _nextWait = _blinkStream.interval[0];
    if (numSteps > MAX_STREAM_STEPS) numSteps = MAX_STREAM_STEPS; 
    _numSteps = numSteps; 
    this->pinOn();                     // Start with the Led on. User needs to call the handle() function to update the next steps 
    _time = 0; 
    _isBlinking = true;
}

void LedHandler::ClearBlinker(void)
{
    // This will stop any active effects
    _isBlinking = false;
    _curStep = 0;
    _numSteps = 0;
    _nextWait = 0;
    _time = 0;
    _expireTime = 0;
    _isBlinking = false; 
}

void LedHandler::ExpireIn(uint16_t expireTime)
{
    _expireTime = expireTime;
    _expireMe = 0;
}

void LedHandler::update(void)
{
    if (_expireTime > 0 && _expireMe > _expireTime)
    {
        // The user only wanted this blinker to be active for a set length of time, which is now expired. Stop blinking.
        this->pinOff();
        ClearBlinker();
    }
    else 
    {
        if (_nextWait > 0 && _time > _nextWait)
        {
            _curStep += 1; 
            if (_curStep < _numSteps)
            {
                // LED on or off
                if (_curStep & 1) { this->pinOff(); }  // Odd numbers get turned off
                else              { this->pinOn();  }  // Even numbers get turned on
    
                // Calcluate time to next change
                if (!_fixedInterval)
                {
                    if   (_curStep > MAX_STREAM_STEPS) { this->pinOff(); ClearBlinker(); }    // This shouldn't happen, but if it does, stop the blinker
                    else { _nextWait = _blinkStream.interval[_curStep]; _time = 0;   }     // Otherwise reset the time and wait for the next interval
                }
                else
                {
                    _time = 0;  // In this case we just leave the next interval to what it was before (it's a fixed interval). But we do need to reset the time
                }
            }
            else
            {
                // We're done
                if (_blinkStream.repeat) 
                {   // Start over
                    _nextWait = _blinkStream.interval[0];
                    _curStep = 0;
                    this->pinOn();
                    _time = 0;
                }
                else
                {   // Turn off lights and wrapup
                    this->pinOff(); 
                    ClearBlinker();
                }
            }    
        }
    }
}



