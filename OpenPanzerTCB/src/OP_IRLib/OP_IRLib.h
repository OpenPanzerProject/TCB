/* OP_IRLib.h       Open Panzer Infra-red library - functions for encoding and decoding model RC tank infrared signals
 * Source:          openpanzer.org              
 * Authors:         Ken Shirrif, Chris Young, Luke Middleton
 *
 * This library is based on the excellent work of Chris Young, who himself based his IRlib on 
 * Ken Shirriff's original IRremote library. 
 *
 * Changes from Chris's version include the removal of many protocols not required by RC tankers, the
 * creation of those that are, and the exclusive use of the IRrecvPCI (pin change interrupt) class 
 * rather than the alternative receive classes. 
 * 
 * Both Ken Shirriff's original library and Chris Young's rewrite are covered under the 
 * GNU LESSER GENERAL PUBLIC LICENSE, as is this modification. Their prior copyrights are listed below. 
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
 * IRLib
 * Version 1.5   June 2014
 * Copyright 2014 by Chris Young http://cyborg5.com (project page: http://tech.cyborg5.com/irlib/)
 * github: https://github.com/cyborg5/IRLib/
 *
 * IRremote
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://www.righto.com/2009/08/multi-protocol-infrared-remote-library.html http://www.righto.com/
 * github: https://github.com/shirriff/Arduino-IRremote
 *
 */
 

#ifndef OP_IRLib_h
#define OP_IRLib_h
#include <Arduino.h>
#include <avr/interrupt.h>
#include "OP_IRLibMatch.h"
#include "../OP_Settings/OP_Settings.h"


// If IRLIB_TRACE is defined, some debugging information about the decode will be printed
// IRLIB_TEST must be defined for the IRtest unit tests to work.  It will make some
// methods virtual, which will be slightly slower, which is why it is optional.
// #define IRLIB_TRACE
// #define IRLIB_TEST

/* If not using either DumpResults methods of IRdecode nor IRfrequency you can
 * comment out the following define to eliminate considerable program space.
 */
#define USE_DUMP

// Only used for testing; can remove virtual for shorter code
#ifdef IRLIB_TEST
#define VIRTUAL virtual
#else
#define VIRTUAL
#endif


// ==========================================================================================================================>>
// UTILITIES AND GENERAL DEFINITIONS
// ==========================================================================================================================>>
// Defines for blinking an LED during IR reception, use this for testing
// We will use L1 (headlights)
#define BLINKLED        53
#define BLINKLED_ON()  (PORTB |= B00000001)
#define BLINKLED_OFF() (PORTB &= B11111110)

// Of if you want to use light 2 (L2)
//#define BLINKLED      5
//#define BLINKLED_ON() (PORTE |= B00001000)
//#define BLINKLED_OFF()    (PORTE &= B11110111)

// IR PROTOCOLS DEFINED
typedef char IRTYPES; 
#define IR_UNKNOWN          0       // Unknown and disabled are the same value!
#define IR_DISABLED         0       // Unknown and disabled are the same value!
#define IR_TAMIYA           1       // Tamiya should always be number 1
#define IR_TAMIYA_2SHOT     2       // Tamiya 2-shot kill code
#define IR_TAMIYA_35        3
#define IR_HENGLONG         4
#define IR_TAIGEN_V1        5       // Original Taigen V1 boards
#define IR_FOV              6       // No longer being sold. Taigen is going to re-release them but we still don't know if they will use the same IR or not. 
#define IR_VSTANK           7
#define IR_OPENPANZER       8       // Not yet implemented. For future custom IR codes.
#define IR_RPR_CLARK        9       // Repair signal: Clark TK-20, 22, and 60 repair protocol
#define IR_RPR_IBU          10      // Repair signal: Italian Battle Unit
#define IR_RPR_RCTA         11      // Repair signal: RC Tanks Australia. Theoretically this is the same as IBU, but untested. 
#define IR_MG_CLARK         12      // Machine gun: Clark protocol (Sony)
#define IR_MG_RCTA          13      // Machine gun: RCTA protocol
#define IR_SONY             14      // For general purpose Sony codes
#define IR_TAIGEN           15      // Taigen V2, V3
//#define ADDITIONAL (number) 
#define LAST_IRPROTOCOL IR_TAIGEN
const __FlashStringHelper *ptrIRName(IRTYPES Type); //Returns a character string that is name of protocol.

// TEAM DEFINITIONS
typedef char IRTEAMS;
#define IR_TEAM_NONE           0
#define IR_TEAM_FOV_1          0    // FOV Team 1 we treat as TEAM_NONE. We can use IR_TEAM_FOV_1 in code but it will equal IR_TEAM_NONE
#define IR_TEAM_FOV_2          1    
#define IR_TEAM_FOV_3          2
#define IR_TEAM_FOV_4          3
//#define ADDITIONAL           next number
#define LAST_IRTEAM IR_TEAM_FOV_4
const __FlashStringHelper *ptrIRTeam(IRTEAMS Team); //Returns a character string that is name of the team.

// From: http://graphics.stanford.edu/~seander/bithacks.html#BitReverseTable
// This lookup table allows us to reverse all the bits in a byte. We can use
// this for sending/receiving Sony Device ID/Commands.
const uint8_t BitReverseTable256[256] PROGMEM_FAR =
{
#   define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
#   define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#   define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
    R6(0), R6(2), R6(1), R6(3)
};
#define ReverseByte(b) pgm_read_byte_far(pgm_get_far_address(BitReverseTable256) + b)

#define TOPBIT 0x80000000   // Mask for left-most bit of a 32 bit binary number
#define LEFTBIT 0x80        // Mask for left-most bit of an 8 bit binary number
#define MARK  0
#define SPACE 1


// ==========================================================================================================================>>
// IR DECODER
// ==========================================================================================================================>>

// Base class for decoding raw results
class IRdecodeBase
{   public:
        IRdecodeBase(void);
        IRTYPES decode_type;           // 
        uint32_t value;                // Decoded value
        unsigned char bits;            // Number of bits in decoded value
        volatile uint16_t *rawbuf;     // Raw intervals in microseconds
        unsigned char rawlen;          // Number of records in rawbuf.
        bool IgnoreHeader;             // Relaxed header detection allows AGC to settle
        virtual void Reset(void);      // Initializes the decoder
        virtual bool decode(void);     // This base routine always returns false, override with your routine
        virtual void DumpResults (void);  
        void UseExtnBuf(void *P);      //Normally uses same rawbuf as IRrecv. Use this to define your own buffer.
        void copyBuf (IRdecodeBase *source);//copies rawbuf and rawlen from one decoder to another
        
        // Sony Codes
        void convertValueToSonyNumbers(uint32_t &val);
        uint8_t SonyDeviceID;   // Sony DeviceID. Will equal zero for non-Sony classes.
        uint8_t SonyCommand;    // Sony Command. Will equal zero for non-Sony classes.

    protected:
        unsigned char offset;           // Index into rawbuf used various places
};

class IRdecodeTamiya: public virtual IRdecodeBase 
{   public:
        virtual bool decode(void);
};
class IRdecodeTamiya_2Shot: public virtual IRdecodeBase
{   public:
        virtual bool decode(void);
};
class IRdecodeTamiya35: public virtual IRdecodeBase 
{   public:
        virtual bool decode(void);
};
class IRdecodeHengLong: public virtual IRdecodeBase 
{   public:
        virtual bool decode(void);
};
class IRdecodeTaigenV1: public virtual IRdecodeBase 
{   public:
        virtual bool decode(void);
};
class IRdecodeTaigen: public virtual IRdecodeBase 
{   public:
        virtual bool decode(void);
};
class IRdecodeFOV: public virtual IRdecodeBase 
{   public:
        virtual bool decode(void);
};
class IRdecodeVsTank: public virtual IRdecodeBase 
{   public:
        virtual bool decode(void);
};
class IRdecodeOpenPanzer: public virtual IRdecodeBase 
{   public:
        virtual bool decode(void);
};
// class IRdecodeClark_Repair
// We don't have a Clark repair class because we jsut use the Sony class instead
class IRdecodeIBU_Repair: public virtual IRdecodeBase 
{   public:
        virtual bool decode(void);
};
class IRdecodeRCTA_Repair: public virtual IRdecodeBase 
{   public:
        virtual bool decode(void);
};
// class IRdecodeClark_MG
// We don't have a Clark MG class because we jsut use the Sony class instead
class IRdecodeRCTA_MG: public virtual IRdecodeBase 
{   public:
        virtual bool decode(void);
};
class IRdecodeSony: public virtual IRdecodeBase 
{
    public:
        virtual bool decode(void);
};
// Main class for decoding all supported protocols
class IRdecode: 
    public virtual IRdecodeTamiya,              // Battle protocols
    public virtual IRdecodeTamiya_2Shot,
    public virtual IRdecodeTamiya35,
    public virtual IRdecodeHengLong,
    public virtual IRdecodeTaigenV1,            // Taigen V1
    public virtual IRdecodeTaigen,              // Taigen V2, V3
    public virtual IRdecodeFOV,
    public virtual IRdecodeVsTank,
    public virtual IRdecodeOpenPanzer,
    public virtual IRdecodeIBU_Repair,          // Repair protocols
    public virtual IRdecodeRCTA_Repair,
    public virtual IRdecodeRCTA_MG,             // Machine gun protocols
    public virtual IRdecodeSony                 // Sony is used for Clark Repair and MG
{   public:
        virtual bool decode(void);    // Calls each decode routine individually
        bool decode(IRTYPES Type);    // Only tries to decode the given protocol
};


// ==========================================================================================================================>>
// IR RECEIVER 
// ==========================================================================================================================>>
// This enum and struct had previously been in a separate file called IRLibRData.h 
// Since we are always going to need it, just put it here instead. 
// The ir_receive_params_t structure contains a variety of variables needed by the receiver routines.
// Typically this data would be part of the IRrecv class however the interrupt service routine
// must have access to it and you cannot pass a parameter to such a routine. The data must be global.
// Receiver states:
enum rcvstate_t {STATE_UNKNOWN, STATE_IDLE, STATE_MARK, STATE_SPACE, STATE_STOP, STATE_RUNNING};
// Information for the interrupt handler:
#define RAWBUF 51 // Length of raw duration buffer (cannot exceed 255)
                  // 51 lets us have 25 data bits (mark and space pair) plus reserving the first element (0) for other data. 
                  // 25 data bits lets us have 1 header bit plus 24 data bits/3 bytes. This is more than enough for every protocol
                  // except for the Tamiya 1/35 models, but even then we can still usually decode it just fine. 
typedef struct {
  unsigned char recvpin;        // pin for IR data from detector
  rcvstate_t rcvstate;          // state machine
  bool blinkflag;               // TRUE to enable blinking of some LED on IR processing (see below)
  uint32_t timer;               // state timer, counts 50uS ticks.(and other uses)
  uint16_t rawbuf[RAWBUF];      // raw data
  unsigned char rawlen;         // counter of entries in rawbuf
} ir_receive_params_t;
extern volatile ir_receive_params_t IR_ReceiveParams;

class IRrecvBase
{   public:
        IRrecvBase(void) {};
        IRrecvBase(unsigned char recvpin);
        void No_Output(void);
        void setBlinkingOnReceive(bool blinkflag);
        bool GetResults(IRdecodeBase *decoder, const uint16_t Time_per_Ticks = 1);
        void enableIRIn(void);
        void disableIRIn(void);
        virtual void resume(void);
        unsigned char getPinNum(void);
        unsigned char Mark_Excess;
    
    protected:
        void Init(void);
};

//Do the actual blinking off and on
//This is not part of IRrecvBase because it may need to be inside an ISR and therefore needs to be global in scope
void do_Blink(void);

/* This receiver uses the pin change hardware interrupt to detect when your input pin
 * changes state. It gives more detailed results than the 50µs interrupts of IRrecv
 * and theoretically is more accurate than IRrecvLoop. However because it only detects
 * pin changes, it doesn't always know when it's finished. GetResults attempts to detect
 * a long gap of space but sometimes the next signal gets there before GetResults notices.
 * This means the second set of signals can get messed up unless there is a long gap.
 * This receiver is based in part on Arduino firmware for use with AnalysIR IR signal analysis
 * software for Windows PCs. Many thanks to the people at http://analysir.com for their 
 * assistance in developing this section of code.
 */
class IRrecvPCI: public IRrecvBase
{   public:
        //Note this is interrupt number not pin number
        IRrecvPCI(unsigned char inum);
        bool GetResults(IRdecodeBase *decoder);
        void resume(void);
    private:
        unsigned char intrnum;
};

/* This routine maps interrupt numbers used by attachInterrupt() into pin numbers.
 * NOTE: these interrupt numbers which are passed to “attachInterrupt()” are not 
 * necessarily identical to the interrupt numbers in the datasheet of the processor 
 * chip you are using. These interrupt numbers are a system unique to the 
 * “attachInterrupt()” Arduino function.  It is used by both IRrecvPCI and IRfrequency.
 */
unsigned char Pin_from_Intr(unsigned char inum);




// ==========================================================================================================================>>
// IR SENDER
// ==========================================================================================================================>>
// This struct is used for interrupt-based sending. It holds an array of pulse widths and other information related to 
// sending them out. 
#define MAX_SEND_BITS 26
#define MAX_STEPS 8     // For now we allow up to 8 steps, which is how many Tamiya 1/35 has. If we need more later for something else, increase this number. 
typedef struct {
    uint32_t sendStream[MAX_SEND_BITS];
    uint8_t  streamIndex;
    uint8_t  bitsToSend;
    uint8_t  timesToRepeat;
    uint8_t  timesRepeated; 
    uint8_t  kHz;
    boolean  sending; 
    IRTYPES  sendProtocol;
    // These have been added to handle the odd Tamiya 1/35th IR codes. It is 64 bits long not counting the header. We will split it into 8 bytes (8 steps)
    // and put the 8 bytes into the values array. At each step we will construct a new sendStream of 8 bits (10 bit for the first step to include the header).
    // For protocols that don't need this madness (which is all of them for now), set currentStep = numSteps = 1; 
    uint8_t  currentStep;   
    uint8_t  numSteps;
    uint8_t  stepValues[MAX_STEPS+1];   // Add 1 to number of MAX_STEPS because we let the user start counting from 1 instead of 0. 
} ir_send_params_t;
extern volatile ir_send_params_t IR_SendParams;

//Base class for sending signals
class IRsendBase
{   public:
        IRsendBase();
        static void OCR1B_ISR(void);    
        static boolean isSendingDone();
        
    protected:
        static void enableIROut(unsigned char khz);
        static void startSending(void);
        static void stopSending(void);
        static void FillSendStreamMultiStep(void);  // For now this is only used for the Tamiya 1/35 IR code. Pass it a step, it fills in sendStream
};

class IRsendTamiya: public virtual IRsendBase
{   public:
        void send(void);    
};
class IRsendTamiya_2Shot: public virtual IRsendBase
{   public:
        void send(void);    
};
class IRsendTamiya35: public virtual IRsendBase
{   public:
        void send(void);    
};
class IRsendHengLong: public virtual IRsendBase
{   public:
        void send(void); 
};
class IRsendTaigenV1: public virtual IRsendBase
{   public:
        void send(void);
};
class IRsendTaigen: public virtual IRsendBase
{   public:
        void send(void);
};
class IRsendFOV: public virtual IRsendBase
{   public:
        void send(uint32_t data); 
        void send(void)      { this->send(FOV_TEAM_1_VALUE); }  // If no data is passed, we default to Team 1, which is free-for-all.
        void sendTeam2(void) { this->send(FOV_TEAM_2_VALUE); }  // Shortcuts for sending the team-specific signals
        void sendTeam3(void) { this->send(FOV_TEAM_3_VALUE); }
        void sendTeam4(void) { this->send(FOV_TEAM_4_VALUE); }
};
class IRsendVsTank: public virtual IRsendBase
{   public:
        void send(uint32_t data);
        // Overloaded with the VsTank signal
        void send(void) { this->send(VsTank_HIT_VALUE); }
};
class IRsendOpenPanzer: public virtual IRsendBase
{   public:
        void send(void);    // PLACEHOLDER FOR FUTURE
};
class IRsendClark_Repair: public virtual IRsendBase
{   public:
        // Clark uses sony codes which do have data, but the data is fixed, 
        // so we just hard-code the function rather than passing data to it. 
        void send(void);    
};
class IRsendIBU_Repair: public virtual IRsendBase
{   public:
        void send(void);    
};
class IRsendRCTA_Repair: public virtual IRsendBase
{   public:
        void send(void);    
};
class IRsendClark_MG: public virtual IRsendBase
{
    public:
        // Clark uses sony codes which do have data, but the data is fixed, 
        // so we just hard-code the function rather than passing data to it. 
        void send(void); 
};
class IRsendRCTA_MG: public virtual IRsendBase
{
    public:
        void send(void); 
};

class IRsendSony: public virtual IRsendBase
{   // Sony is used by Clark for repair and machine gun codes
    // It is also used by RC Tanks Australia (Mako, ASP) to send the tank ID (but not in a convenient manner so we have not reproduced it)
    public:
        void send(uint32_t data, uint8_t times_send=Sony_TIMESTOSEND);
        void sendDeviceIDCommand(uint8_t SonyDeviceID, uint8_t SonyCommand, uint8_t times_send=Sony_TIMESTOSEND);
};
class IRsendRaw: public virtual IRsendBase
{
    public:
        void send(uint32_t buf[], unsigned char len, unsigned char khz);
};

class IRsend: 
    public virtual IRsendTamiya,
    public virtual IRsendTamiya_2Shot,
    public virtual IRsendTamiya35,
    public virtual IRsendHengLong,
    public virtual IRsendTaigenV1,      // Taigen V1
    public virtual IRsendTaigen,        // Taigen V2, V3
    public virtual IRsendFOV,
    public virtual IRsendVsTank,
    public virtual IRsendOpenPanzer,
    public virtual IRsendClark_Repair,
    public virtual IRsendIBU_Repair,
    public virtual IRsendRCTA_Repair,
    public virtual IRsendClark_MG,
    public virtual IRsendRCTA_MG,
    public virtual IRsendSony,
    public virtual IRsendRaw
{   public:
        void send(IRTYPES Type, uint32_t data);
        void send(IRTYPES Type); // Will send default signals
        void sendDeviceIDCommand(IRTYPES Type, uint8_t DeviceID, uint8_t Command);  // Only use with 12-bit Sony-compatible protocols
};




#endif //OP_IRLib_h
