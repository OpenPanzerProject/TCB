/* OP_LedHandler.h      Open Panzer Led Handler - class for handling LEDs including blink effects, requires use of elapsedMillis
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

#ifndef LedHandler_h
#define LedHandler_h

#include <Arduino.h>
#include "../OP_Settings/OP_Settings.h"
#include "../elapsedMillis/elapsedMillis.h"

#define FADE_IN                            1
#define FADE_OUT                           2
#define NUM_FADE_UPDATES                 100

#define DEFAULT_BLINK_INTERVAL           200
#define MAX_STREAM_STEPS                  10            // A stream consists of a pattern of on/off blinks separated by user-specified lengths of time. A single blink (on/off) takes 2 steps. 
typedef struct                                          // This struct holds an array of blink patterns, and a flag to indicate if it should repeat or not
{
    uint16_t        interval[MAX_STREAM_STEPS];
    boolean         repeat;
} BlinkStream;

class LedHandler
{   public:
        LedHandler() {}; 
        
        void begin (byte p, boolean i=false); 
        void on(void);
        boolean isOn(void);
        void off(void);
        void toggle(void);
        void update(void);                                                      // Update blinking effect
        boolean isBlinking(void);
        boolean isFading(void);
        void ExpireIn(uint16_t); 
        void Blink(uint16_t interval=DEFAULT_BLINK_INTERVAL);                   // Blinks once at set interval
        void Blink(uint8_t times, uint16_t  interval=DEFAULT_BLINK_INTERVAL);   // Overload - Blinks N times at set interval
        void startBlinking(uint16_t on_interval=DEFAULT_BLINK_INTERVAL, uint16_t off_interval=DEFAULT_BLINK_INTERVAL);   // Starts a continuous blink at the set intervals, to stop call stopBlinking() or Off()
        void stopBlinking(void);
        void DoubleTap(boolean repeat=false);
        void TripleTap(boolean repeat=false);
        void QuadTap(boolean repeat=false);
        void StreamBlink(BlinkStream bs, uint8_t numSteps);
        void Fade(uint8_t fade_in, uint16_t span, boolean AddBlinkEffect);
        void stopFading(void);
        
    private:
        void ClearBlinker(void);
        void ClearFader(void);
        void pinOn(void);
        void pinOff(void);
        elapsedMillis   _time;
        elapsedMillis   _expireMe;
        uint16_t        _expireTime; 
        byte            _pin;
        boolean         _invert;
        boolean         _isBlinking;
        uint8_t         _isFading;
        uint16_t        _curStep;
        uint16_t        _numSteps;
        uint16_t        _nextWait;
        boolean         _fixedInterval;
        BlinkStream     _blinkStream;
        float           _fadeStart;
        boolean         _fadeBlinkEffect;
        uint16_t        _fadeInterval;
     
};


#endif 
