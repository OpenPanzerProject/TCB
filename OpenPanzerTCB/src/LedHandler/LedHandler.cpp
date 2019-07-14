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
    if (_isFading)   this->stopFading();
    this->pinOn();
}

boolean LedHandler::isOn(void)
{
    if (_invert) return !digitalRead(_pin);
    else         return  digitalRead(_pin);    
}

void LedHandler::off(void)
{
    if (_isBlinking) this->stopBlinking();
    if (_isFading)   this->stopFading();
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
    this->pinOn();                      // Start with the Led on. User needs to call the update() function to update the next steps 
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
    this->pinOn();                     // Start with the Led on. User needs to call the update() function to update the next steps 
    _time = 0; 
    _isBlinking = true;
}

void LedHandler::ClearBlinker(void)
{
    // This will stop any active effects
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


// Will fade a LED in or out (use FADE_IN or FADE_OUT for dir)
// Span is in milliseconds and is the length of time the fade will take,
// with a minimum of 1/4 second (span = 250 mS) and a maximum of 65 seconds (span = ~65,000)
void LedHandler::Fade(uint8_t dir, uint16_t span, boolean addBlinkEffect)
{
    // See: https://www.sparkfun.com/tutorials/329
    // Full brightness = 90  degress = 1.570 radians (pi * 0.5)
    // Half brightness = 180 degrees = 3.141 radians (pi * 1  )
    // Off             = 270 degrees = 4.712 radians (pi * 1.5)

    // Start clear
    ClearFader();
    
    // Set the blink effect flag, which will add two blinks after the fade-in or before the fade-out
    _fadeBlinkEffect = addBlinkEffect; 

    // Determine delay between updates
    _numSteps = NUM_FADE_UPDATES;           // See LedHandler.h
    if (span < _numSteps) span = _numSteps; // Don't allow fades shorter than the number of updates, it would just result in 1 mS or 0 mS updates
    _fadeInterval = span / _numSteps;       // Interval between each fade update
    _nextWait = _fadeInterval;

    // The _isFading flag will tell us more than that we are fading, but also in which direction. 
    _isFading = dir;

    // If dir = 1 then fade in starting from off, otherwise if dir = 2 fade out starting from on
    if (dir != FADE_IN && dir != FADE_OUT) dir = FADE_IN;   // Constrain
    switch (dir)
    {
        case FADE_IN:
            this->pinOff(); 
            _invert ? _fadeStart = 1.57  : _fadeStart = 4.712;
            break; 
            
        case FADE_OUT:
            _invert ? _fadeStart = 4.712 : _fadeStart = 1.57;
            this->pinOn();
            if (_fadeBlinkEffect)
            {
                // We need to start with a double-blip, and then after complete the fade-out
                _isFading = 0;  // Pretend like we're not fading, the _fadeBlinkEffect flag will save us later
                BlinkStream bs;
                bs.interval[0] = 110;      // On
                bs.interval[1] = 40;       // Off
                bs.interval[2] = 60;       // On
                bs.repeat = false;
                this->StreamBlink(bs, 3);   
                return;
            }
            break;
    }

    // Reset the ellapsedMillis time
    _time = 0;
}

void LedHandler::stopFading(void)
{
    ClearFader();
}

void LedHandler::ClearFader(void)
{
    _expireTime = 0;
    _curStep = 0;
    _numSteps = 0;
    _nextWait = 0;
    _time = 0;
    _fadeBlinkEffect = false;
    _isFading = 0;    
}

boolean LedHandler::isFading(void)
{
    if (_isFading > 0) return true;
    else               return false;
}

void LedHandler::update(void)
{
    if (_isFading > 0)  // We're in the middle of a fading operation
    {
        if (_nextWait > 0 && _time > _nextWait)
        {
            _curStep += 1; 
            if (_curStep < _numSteps)
            {
                _fadeStart = _fadeStart + (PI / (float)_numSteps);
                analogWrite(_pin, (sin(_fadeStart) * 127.5) + 127.5);  
                _time = 0;
            }
            else
            {
                // We're done
                switch (_isFading)
                {
                    // Fading-in ends with off
                    case FADE_OUT: 
                        this->pinOff(); 
                        this->ClearFader(); 
                        break;   
                    
                    // Fading-in ends with on
                    case FADE_IN:  
                    default:
                        if (_fadeBlinkEffect)   
                        {   // We want to add two blips to close-out
                            this->ClearFader();         // Clear out the fader, we have to do this after the _fadeBlinkEffect check
                            BlinkStream bs;
                            bs.interval[0] = 50;        // On
                            bs.interval[1] = 40;        // Off
                            bs.interval[2] = 110;       // On
                            bs.interval[3] = 500;       // Off
                            bs.repeat = false;
                            this->StreamBlink(bs, 4);   // 4 steps in the double-tap
                        }
                        else this->ClearFader();
                        break;                        
                }
            }
        }
    }
    else    // If we're not fading, we're blinking
    {
        if (_expireTime > 0 && _expireMe > _expireTime)
        {
            // The user only wanted this blinker to be active for a set length of time, which is now expired. Stop blinking.
            this->pinOff();
            ClearBlinker();
            ClearFader();
        }
        else 
        {
            if (_nextWait > 0 && _time > _nextWait)
            {
                _curStep += 1; 
                if (_curStep < _numSteps)
                {
                    // If we are not fading we are either turning the LED on or off
                    if (_curStep & 1) { this->pinOff(); }  // Odd numbers get turned off
                    else              { this->pinOn();  }  // Even numbers get turned on
        
                    // Calcluate time to next change
                    if (!_fixedInterval)
                    {
                        if   (_curStep > MAX_STREAM_STEPS) { this->pinOff(); ClearBlinker(); }    // This shouldn't happen, but if it does, stop the blinker
                        else { _nextWait = _blinkStream.interval[_curStep]; _time = 0;   }        // Otherwise reset the time and wait for the next interval
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
                    else if (_fadeBlinkEffect)
                    {
                        // In this case we want to start a fade-out after the blink portion is done
                        _fadeBlinkEffect = false;       // Clear this flag
                        _isFading = FADE_OUT;           // Set the fadeout flag
                        ClearBlinker();                 // Clear out the blinker
                        _numSteps = NUM_FADE_UPDATES;   // Setup the fade-out (some of it was already set up in the original Fade call
                        _nextWait = _fadeInterval;
                    }
                    else
                    {   
                        // Turn off lights and wrapup
                        this->pinOff(); 
                        ClearBlinker();
                    }
                }    
            }
        }
    }
}



