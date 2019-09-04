/* OP_Motors.h      Open Panzer Motors - a library of motor functions
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


/* A base and derived "motor" classes for controlling motorized outputs from the TCB board, ie, 
 * left and right drive motors, the turret rotation and elevation motors, recoil servo, etc. 
 * Motors can be driven by a variety of means: 
 * - Directly from the TCB board using the onboard L298 dual motor driver (for small motors of ~1A max each)
 *   This could easily power turret motors on 1/16th models or even drive motors on 1/24 scale
 * - Serial enabled speed controls such as the Dimension Engineering Sabertooth series, Pololu Qik series,  
 *   or future Open Panzer serial ESCs
 * - Traditional RC speed controls which take a standard servo PWM input, although these types of ESCs are rarely
 *   well suited to tank tracks. If using an ESC, REMOVE THE RED (POSITIVE) WIRE FROM THE ESC PLUG! 
 * - Servos modified for continuous rotation, such as those used for custom turret rotation drives (or even main drives
 *   in very small tanks such as 1/24th scale)
 * - Unmodified servos, often used for custom turret elevation control
 *
 *
 */
 
#ifndef OP_Motors_h
#define OP_Motors_h

#include "../OP_Settings/OP_Settings.h"
#include "../OP_Servo/OP_Servo.h"
#include "../OP_Sabertooth/OP_Sabertooth.h"
#include "../OP_PololuQik/OP_PololuQik.h"
#include "../OP_Scout/OP_Scout.h"

// ESC Position type. Motor controllers we will be using (including the onboard L298) are all dual, so we have SIDEA and SIDEB.
// We can also use servo outputs to control speed controllers, there are 8 possible positions for those. 
typedef enum ESC_POS_t
{   SERVO_8 = 0,
    SERVO_7 = 1,
    SERVO_6 = 2,
    SERVO_5 = 3,
    SERVO_4 = 4,
    SERVO_3 = 5,
    SERVO_2 = 6,
    SERVO_1 = 7,
    SIDEA, 
    SIDEB
};

typedef char Drive_t;
#define SERVO8      0
#define SERVO7      1
#define SERVO6      2
#define SERVO5      3
#define SERVO4      4
#define SERVO3      5
#define SERVO2      6
#define SERVO1      7   
#define POLOLU      8
#define SABERTOOTH  9
#define OP_SCOUT    10
#define ONBOARD     11  
#define SERVO_ESC   12
#define SERVO_PAN   13
#define SERVO_RECOIL 14
#define DRIVE_DETACHED    15
//#define ADDITIONAL (number)
#define LAST_DRIVE_TYPE DRIVE_DETACHED
const __FlashStringHelper *ptrDriveType(Drive_t dType); //Returns a character string that is name of drive type (see OP_Motors.cpp)


class Motor {
  protected:
    ESC_POS_t ESC_Position;
    int e_minspeed, e_maxspeed, e_middlespeed;      // We have external speed range, and internal. External is the range of numbers that will be passed
                                                    // to the motor object from our main sketch. These are likely to be -255 to 255
    int i_minspeed, i_maxspeed, i_middlespeed;      // The internal is specific to each ESC, the external range will be mapped to the internal range
                                                    // For example, the internal range for a servo motor is 1000-2000. For Pololu Serial controllers it is -127 to 127
    int di_minspeed, di_maxspeed, di_middlespeed;   // We also have "backup" or "default" values of internal speed, so we can modify internal speed
                                                    // temporarily and then easily revert back to the default. 
    int curspeed;                                   // Current speed
    boolean reversed;                               // Motor reversed

  public:
    // Constructor, set member ESC_Position, external speed range, and reversed status
    Motor (ESC_POS_t pos, int min, int max, int middle, boolean rev=false) : ESC_Position(pos), e_minspeed(min), e_maxspeed(max), e_middlespeed(middle), reversed(rev) {}
    
    // This is the internal range of values that is specific to each motor driver.
    void set_InternalRange (int min, int max, int middle)
        { this->i_minspeed = min; this->i_maxspeed = max; this->i_middlespeed = middle; }

    // This is a copy of the internal range. In case we want to temporarily modify the range of possible output values,
    // we can use cut_Speed and then revert back to defaults by using restore_Speed
    void set_DefaultInternalRange (int min, int max, int middle)
        { this->di_minspeed = min; this->di_maxspeed = max; this->di_middlespeed = middle; }

    // Restores both positive and negative speed ranges to internal defaults
    void restore_Speed(void)        
        { this->i_minspeed = this->di_minspeed; this->i_maxspeed = this->di_maxspeed; this->i_middlespeed = this->di_middlespeed; }

    // Functions to set/get the reversed status
    void set_Reversed (boolean rev) 
        { this->reversed = rev; }
    boolean isReversed(void)
        { return this->reversed; }

    // Other common functions
    void cut_SpeedPct(uint8_t);         // Cut the total speed range by some percent
    void set_MaxSpeedPct(uint8_t);      // Alternate way of writing cut_SpeedPct
    
    // This maps the external speed range to the internal one
    int map_Range(int s)
    {   if (s == this->e_middlespeed) {return this->i_middlespeed;}
        else 
        {   if (s > this->e_middlespeed)
            {
                if (this->reversed) return map(s, this->e_middlespeed, this->e_maxspeed, this->i_middlespeed, this->i_minspeed); 
                else                return map(s, this->e_middlespeed, this->e_maxspeed, this->i_middlespeed, this->i_maxspeed);
            }
            else
            {
                if (this->reversed) return map(s, this->e_middlespeed, this->e_minspeed, this->i_middlespeed, this->i_maxspeed); 
                else                return map(s, this->e_middlespeed, this->e_minspeed, this->i_middlespeed, this->i_minspeed); 
            }
        }
    }   
    
    int getSpeed(void) { return this->curspeed; }
    virtual void setSpeed(int) =0;                  // Purely virtual
    virtual void begin(void) =0;
    virtual void stop(void) =0;
    virtual void update(void) =0;
};



class OPScout_SerialESC: public Motor, public OP_Scout {
  public:
    OPScout_SerialESC(ESC_POS_t pos, int min, int max, int middle, byte addr, HardwareSerial *hwSerial, uint32_t *baud, boolean drag, uint8_t cl) : Motor(pos,min,max,middle), OP_Scout(addr,hwSerial), motorbaud(baud), dragInnerTrack(drag), currentLimit(cl) {}
    void setSpeed(int s);
    void begin(void);
    void stop(void);
    void update(void);
  private:
    uint32_t LastUpdate_mS;
    uint32_t *motorbaud;
    boolean dragInnerTrack;
    uint8_t currentLimit;
};

class Sabertooth_SerialESC: public Motor, public OP_Sabertooth {
  public:
    Sabertooth_SerialESC(ESC_POS_t pos, int min, int max, int middle, byte addr, HardwareSerial *hwSerial) : Motor(pos,min,max,middle), OP_Sabertooth(addr,hwSerial) {}
    void setSpeed(int s);
    void begin(void);
    void stop(void);
    void update(void);
  private:
    static boolean sentAutobaud;
    uint32_t LastUpdate_mS;    
};

class Pololu_SerialESC: public Motor, public OP_PololuQik {
  public:
    Pololu_SerialESC(ESC_POS_t pos, int min, int max, int middle, byte deviceID, HardwareSerial *hwSerial) : Motor(pos,min,max,middle), OP_PololuQik(deviceID,hwSerial) {}
    void setSpeed(int s);
    void begin(void);
    void stop(void);
    void update(void);
  private:
    static boolean sendAutobaud;
    uint32_t LastUpdate_mS;        
};

class Onboard_ESC: public Motor {
  public:
    Onboard_ESC(ESC_POS_t pos, int min, int max, int middle) : Motor(pos,min,max,middle) {}
    void setSpeed(int s);
    void begin(void);   
    void stop(void);
    void update(void) { return; }   // Do nothing, we only need the update for serial controllers
};

class Servo_ESC: public Motor, public OP_Servos {
  public:
    Servo_ESC(ESC_POS_t pos, int min, int max, int middle) : Motor(pos,min,max,middle) {}
    void setPos(int s); 
    void setSpeed(int s) { setPos(s); } // We keep this for compatibility with the other motor classes, but in this case, it actually sets the servo *position* directly, it has nothing to do with speed.
    void begin(void);
    void stop(void);
    void update(void) { return; }   // Do nothing, we only need the update for serial controllers
};

class Servo_PAN: public Motor, public OP_Servos {
  public:
    Servo_PAN(ESC_POS_t pos, int min, int max, int middle) : Motor(pos,min,max,middle) {}
    void setLimits(uint16_t, uint16_t); // Set end-point limits on the servo object, not the motor
    void setSpeed(int s);   // This will actually set the speed at which the servo *pans*
    void setPos(int s);     // This sets an actual position directly, rather than the speed. 
    int fixedPos;           // What is the position the user wants (not the speed, not the actual present position, but the position they want it set to)
    int16_t PulseWidth(void);   // We won't know what the actual position of the pan servo is, so this will tell us
    void begin(void);
    void stop(void);
    void update(void) { return; }   // Do nothing, we only need the update for serial controllers
  private: 
    boolean canSetFixedPos; 
};


typedef unsigned char RecoilPreset;
#define RS_PRESET_NONE                  0
#define RS_PRESET_TAIGEN_TIGER1         1
//#define ADDITIONAL (number)
#define LAST_RS_PRESET RS_PRESET_TAIGEN_TIGER1
const __FlashStringHelper *ptrRecoilPreset(RecoilPreset rs); //Returns a character string that is name of recoil preset (see OP_Motors.cpp)

class Servo_RECOIL: public Motor, public OP_Servos {
  public:
    Servo_RECOIL(ESC_POS_t pos, int min, int max, int middle, uint16_t mS_Recoil, uint16_t mS_Return, uint8_t Reversed) : Motor(pos,min,max,middle), _RecoilmS(mS_Recoil), _ReturnmS(mS_Return), _Reversed(Reversed) {}
    void setSpeed(int); // This doesn't do anything for this particular derived class
    void setLimits(uint16_t, uint16_t); // Set end-point limits on the servo object, not the motor
    void begin(void);
    void stop(void);    // This doesn't do anything for this particular derived class
    void Recoil(void)
        { this->StartRecoil(this->ESC_Position); }
    void update(void) { return; }   // Do nothing
  private:
    const uint16_t _RecoilmS;
    const uint16_t _ReturnmS;
    const uint8_t  _Reversed;   
};

// This sub-class is empty and does nothing. 
class Null_Motor: public Motor {
  public:
    Null_Motor() : Motor(ESC_POS_t(0),0,0,0) {}
    void setSpeed(int s) { return; }
    void begin(void)     { return; }  
    void stop(void)      { return; }
    void update(void)    { return; }
};


#endif //OP_Motors_h





