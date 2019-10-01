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

#ifndef OP_Smoker_h
#define OP_Smoker_h   

#include <Arduino.h>
#include "../OP_Settings/OP_Settings.h"

typedef char Smoker_t;
#define SMOKERTYPE_ONBOARD_STANDARD     0
#define SMOKERTYPE_ONBOARD_SEPARATE     1
#define SMOKERTYPE_SERIAL               2
//#define ADDITIONAL (number)
#define LAST_SMOKERTYPE SMOKERTYPE_SERIAL
const __FlashStringHelper *ptrSmokerType(Smoker_t sType); //Returns a character string that is name of smoker type (see OP_Smoker.cpp)


// DEFINES FOR SERIAL SMOKER 
// -------------------------------------------------------------------------------------------------------------------------------------------->> 
// Address
#define SMOKER_ADDRESS                       0x85    // 133     Scout uses 131 and 132. Sabertooth range is 128-135
// Commands                                          
#define SMOKER_CMD_HEATER_LEVEL              0x00    // 0       Heater level                                               
#define SMOKER_CMD_FAN_SPEED                 0x04    // 4       Fan speed
#define SMOKER_CMD_SERIAL_WATCHDOG           0x0E    // 14
#define SMOKER_CMD_BAUD_RATE                 0x0F    // 15
#define SMOKER_CMD_STARTUP                   0x18    // 24      Startup command
#define SMOKER_CMD_SHUTDOWN                  0x19    // 25      Shutdown command
// Codes
#define SMOKER_BAUD_CODE_2400                   1     // Codes for changing baud rates
#define SMOKER_BAUD_CODE_9600                   2     // These are the same codes used by certain Dimension Engineering Sabertooth controllers
#define SMOKER_BAUD_CODE_19200                  3     //
#define SMOKER_BAUD_CODE_38400                  4     //    
#define SMOKER_BAUD_CODE_115200                 5     //
#define SMOKER_BAUD_CODE_57600                  6     // The preceding codes are numbered identically to the codes used for Sabertooth controllers, which do not include 57600. That is why 57600 is number 6 and not number 5. 
// Watchdog
#define OP_Smoker_WatchdogTimeout_mS         1000     // in milliseconds. If a serial command is not received after this length of time, the Serial Smoker will shutdown automatically. 


class OP_Smoker {
  public:
    OP_Smoker(int min, int max, int middle, int idle, int fastIdle, int maxSpeed, int idleHeat, int fastIdleHeat, int maxHeat, HardwareSerial *port, uint32_t *baud, Smoker_t st, uint8_t preheat) : e_minspeed(min), e_maxspeed(max), e_middlespeed(middle), Idle(idle), FastIdle(fastIdle), MaxSpeed(maxSpeed), HeatAmtIdle(idleHeat), HeatAmtFastIdle(fastIdleHeat), HeatAmtMax(maxHeat), smokerPort(port), smokerBaud(baud), SmokerType(st), preHeat_Seconds(preheat) {}
    void setSpeed(int s);
    int getSpeed(void) { return this->curspeed; }
    void begin(void);   
    void stop(void);
    void setIdle(void);
    void setFastIdle(void);
    void preHeat(void);
    void update(void);              // Actually we will use the update routine on the smoker for special effects
    void Startup(boolean);          // This will trigger the startup  smoker effect (boolean for whether the transmission is engaged or not)
    void Shutdown(boolean);         // This will trigger the shutdown smoker effect (boolean for whether the transmission is engaged or not - the effect slowly turns off smoker)

    // SERIAL FUNCTIONS
    // ---------------------------------------------------------------->>
    // Sends a packet serial command to the serial smoker device
    // command  The number of the command.
    // value    The command's value.
    void command(byte command, byte value = 0) const;
    
    // Sets the level of the heating element or the speed of the fan
    // output   The heater or fan (code 0 or 4)
    // speed    The level/speed, between -255 to 255
    void setLevelSerial(byte output, int level) const;

    // RANGE FUNCTIONS
    // ---------------------------------------------------------------->>
    // This is the internal range of speed values for the fan
    void set_InternalSpeedRange (int min, int max, int middle)
        { this->i_minspeed = min; this->i_maxspeed = max; this->i_middlespeed = middle; }

    // This is a copy of the internal range for fan speeds. In case we want to temporarily modify the range of possible output values,
    // we can use cut_Speed and then revert back to defaults by using restore_Speed
    void set_DefaultInternalSpeedRange (int min, int max, int middle)
        { this->di_minspeed = min; this->di_maxspeed = max; this->di_middlespeed = middle; }

    // This is the internal range of values for the heater
    void set_InternalHeatRange (int min, int max, int middle)
        { this->i_minheat = min; this->i_maxheat = max; this->i_middleheat = middle; }

    // This is a copy of the internal range for heat levels. In case we want to temporarily modify the range of possible output values,
    // we can use cut_Heat and then revert back to defaults by using restore_Heat
    void set_DefaultInternalHeatRange (int min, int max, int middle)
        { this->di_minheat = min; this->di_maxheat = max; this->di_middleheat = middle; }

    // Restores fan speed range and heat level to internal defaults
    void restore_Speed(void)        
        { this->i_minspeed = this->di_minspeed; this->i_maxspeed = this->di_maxspeed; this->i_middlespeed = this->di_middlespeed; 
          this->i_minheat  = this->di_minheat;  this->i_maxheat  = this->di_maxheat;  this->i_middleheat  = this->di_middleheat;  }

    // Other common functions
    void cut_SpeedPct(uint8_t);         // Cut the total speed range by some percent
    void set_MaxSpeedPct(uint8_t);      // Alternate way of writing cut_SpeedPct
    void cut_HeatPct(uint8_t);          // Cut the total heat level range by some percent
    void set_MaxHeatPct(uint8_t);       // Alternate way of writing cut_HeatPct
    
    // This maps the external speed range to the internal one
    int map_RangeFan(int s)
    {   if (s == this->e_middlespeed) {return this->i_middlespeed;}
        else 
        {   if (s > this->e_middlespeed)
            {   //Serial.print("A ");
                if (this->reversed) return map(s, this->e_middlespeed, this->e_maxspeed, this->i_middlespeed, this->i_minspeed); 
                else                return map(s, this->e_middlespeed, this->e_maxspeed, this->i_middlespeed, this->i_maxspeed); //Serial.print(map(s, this->e_middlespeed, this->e_maxspeed, this->i_middlespeed, this->i_maxspeed)); Serial.print(" "); 
            }
            else
            {
                if (this->reversed) return map(s, this->e_middlespeed, this->e_minspeed, this->i_middlespeed, this->i_maxspeed); 
                else                return map(s, this->e_middlespeed, this->e_minspeed, this->i_middlespeed, this->i_minspeed); 
            }
        }
    }   

    // This maps the external heat level range to the internal one
    int map_RangeHeat(int s)
    {   if (s == this->e_middlespeed) {return this->i_middleheat;}
        else 
        {   if (s > this->e_middlespeed)
            {
                if (this->reversed) return map(s, this->e_middlespeed, this->e_maxspeed, this->i_middleheat, this->i_minheat); 
                else                return map(s, this->e_middlespeed, this->e_maxspeed, this->i_middleheat, this->i_maxheat);
            }
            else
            {
                if (this->reversed) return map(s, this->e_middlespeed, this->e_minspeed, this->i_middleheat, this->i_maxheat); 
                else                return map(s, this->e_middlespeed, this->e_minspeed, this->i_middleheat, this->i_minheat); 
            }
        }
    }   


private:
    int e_minspeed, e_maxspeed, e_middlespeed;      // We have external speed range, and internal. External is the range of numbers that will be passed
                                                    // to the motor object from our main sketch. These are likely to be -255 to 255
    int i_minspeed, i_maxspeed, i_middlespeed;      // The internal is specific to each ESC, the external range will be mapped to the internal range
                                                    // For example, the internal range for a servo motor is 1000-2000. For Pololu Serial controllers it is -127 to 127
    int di_minspeed, di_maxspeed, di_middlespeed;   // We also have "backup" or "default" values of internal speed, so we can modify internal speed
                                                    // temporarily and then easily revert back to the default. 
    
    int i_minheat, i_maxheat, i_middleheat;         // Now do the same for the heater. We can use the same external range however
    int di_minheat, di_maxheat, di_middleheat;
    int curheat;                              
    
    boolean reversed;                               // Motor reversed - not presently used
    int curspeed;                                   // Current speed
    const int Idle;
    const int FastIdle;
    const int MaxSpeed;
    const int HeatAmtIdle;
    const int HeatAmtFastIdle;
    const int HeatAmtMax;
    HardwareSerial *smokerPort;
    uint32_t *smokerBaud;
    const Smoker_t SmokerType;
    uint8_t preHeat_Seconds;
    uint32_t LastUpdate_mS;    
  
    // These can be used for special smoker effects. The main sketch will poll the update() function routinely, 
    // which can then create changes in smoker speeds over time. We use this presently to slowly reduce speed
    // from idle when the engine is turned off. 
    #define smoker_update_rate_mS   15
    enum smoker_effect_t
    {   NONE = 0,
        SHUTDOWN
    };
    smoker_effect_t smoker_effect;
    void setSpeed_wEffect(int s);   // The update() routine can call this version of setSpeed without clearing the special effect
    void clearSmokerEffect(void) { if (smoker_effect != NONE) smoker_effect = NONE; }
    
};



#endif
