/* OP_PCComm.h      Open Panzer PC Communication - a library for talking to an application running on a PC
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

/* This library provides functions for listening, responding to and executing commands received from a computer application. 
 * Primarily this will involve updating all variable stored in EEPROM from settings the user has defined in the desktop app. 
 *
 * Serial communication is initiated whenever the TCB receives the characters defined in INIT_STRING (as of now, "OPZ"). 
 * Once those are read on the serial line, the TCB will go into listening mode. From then all communication is formally defined below:
 *
 * We use a constructed serial protocol of "sentences" composed (most of the time) of four pieces of information, seperated by a delimiter character (we use the pipe char "|")
 * The first two values of the sentence must always be non-zero. These are the "Command" and the "ID"
 * The next two values in the sentence are the "value" to be operated upon, and finally, a "Checksum" of all or part of the sentence.
 * In some cases there will be no value relevant to the command being sent, and as such, there would also be no ID to identify that value since there is no value. 
 * In those cases the command will be repeated in the ID slot. The Value should still be sent but it will be 0. 
 * 
 * In other cases there can be more than one value returned. In this case the sentence will be Command | ID | Value1 | Value2 | ValueN | Checksum
 * For now the only time we send more than one value is when streaming radio data to the PC. We have no cases thus far where the PC needs to send more than one value at a time.
 *
 * IDs above MIN_EEPROM_ID (1000) will always be the identifier for an eeprom variable. IDs below MIN_EEPROM_ID are typically commands that have been repeated
 * in the ID slot but not always. 
 * 
 * Some examples in pseudo code:
 *
 * PC tells us to upate a variable in EEPROM
 * -------------------------------------------------------------------------------------------------
 * 124|2212|120|xxx newline
 * 124      - command to update EEPROM (see definitions of commands below)
 * |        - delimiter
 * 2212     - ID of the value to be updated (in our Excel we see this is SmokerFastIdleSpeed)
 * |        - delimiter
 * 120      - the new value the EEPROM variable should be set to
 * |        - delimiter
 * xxx      - checksum (not calculated for this example)
 * newline  - newline character, tells us the sentence is over
 *
 * PC tells us to disconnect
 * -------------------------------------------------------------------------------------------------
 * 31|31|0|0 newline
 * 31       - command to tell the Tank Control Board we are done, disconnect
 * |        - delimiter
 * 31       - command repeated in the ID slot
 * |        - delimiter
 * 0        - nothing needed in the value slot, but we still include the slot
 * |        - delimiter
 * 0        - nothing needed in the checksum slot, but we still include the slot
 * newline  - end of sentence
 *
  * Tank Control Board tells PC to repeat the last sentence (something didn't go right with reception)
 * -------------------------------------------------------------------------------------------------
 * 135|135|0|0 newline
 * 135      - command to tell the computer we need the last sentence resent 
 * |        - delimiter
 * 135      - command repeated in the ID slot
 * |        - delimiter
 * 0        - nothing needed in the value slot, but we still include the slot
 * |        - delimiter
 * 0        - nothing needed in the checksum slot, but we still include the slot
 * newline  - end of sentence
 *
 * Tank Control Board gives the PC a value (PC must request it first with PCCMD_READ_EEPROM, this would be the response)
 * -------------------------------------------------------------------------------------------------
 * 136|2212|120|xxx newline
 * 136      - command to tell the computer we are giving him a value
 * |        - delimiter
 * 2212     - ID of the value we are sending (should match the ID the computer just requested)
 * |        - delimiter
 * 120      - the value presently in eeprom
 * |        - delimiter
 * 0        - checksum (not calculated in this example)
 * newline  - end of sentence
 *
 * Tank Control Board gives the PC the FIRMWARE_VERSION from OP_Settings.h (PC must request it first with PCCMD_READ_VERSION)
 * -------------------------------------------------------------------------------------------------
 * 136|128|FIRMWARE_VERSION|xxx newline
 * 136      - command to tell the computer we are giving him a value
 * |        - delimiter
 * 128      - This is one of the unusual cases where the ID is not an EEPROM ID, but nor is it a repeat of the command. 
 *            In fact in this case it is a repeat of the command the computer sent (PCCMD_READ_VERSION = 128)
 *            Because this ID is less than MIN_EEPROM_ID it will know it is not an eeprom variable. Instead the PCCMD_READ_VERSION is used as an *ID*
 *            will tell the PC this value is in response to that request. 
 * |        - delimiter
 * FIRMWARE_VERSION     - from OP_Settings.h, this is actually a string
 * |        - delimiter
 * 0        - checksum (not calculated in this example)
 * newline  - end of sentence
 *
 *
 * Almost all communication is one sentence each device, back and forth. So PC will send a sentence and wait for TCB to send a sentence,
 * then TCB will wait for the PC to send a sentence, etc... In other words, each device should only send one sentence a time, and 
 * refrain from sending any more sentences until it hears back from the other device. 
 *
 * The biggest exception to this is when the PC requests radio data streaming, in which case TCB will keep sending the PC radio data
 * until told to stop. 
 *
 * Another exception is if the watchdog timer expires, in which case the TCB will tell the PC goodbye even if it's not the TCB's turn to talk. 
 * 
 *
 */ 
 

#ifndef OP_PCComm_h
#define OP_PCComm_h

#include <Arduino.h>
#include <avr/pgmspace.h>
#include "../OP_Devices/OP_Devices.h"
#include "../EEPROMex/EEPROMex.h"
#include "../OP_EEPROM/OP_EEPROM.h"
#include "../OP_Radio/OP_Radio.h"
#include "../OP_Settings/OP_Settings.h"


// Communication defines
#define DELIMITER               '|'
#define NEWLINE                 '\n'
#define NUM_DELIMITERS           3      // How many delimiters are in a full sentence

// Commands received from PC
#define INIT_STRING             "OPZ"   // The initialization string that tells us to start communicating with the PC
                                        // Can't be more characters than VALUE_BUFF - 1 
#define PCCMD_SABERTOOTH_BAUD   118     // PC wants us to set the baud rate on certain Sabertooth devices connected to Serial 2
#define PCCMD_CONFPOLOLU_DRIVE  119     // PC wants us to configure a Pololu device connected to Serial 2 for use with drive motors
#define PCCMD_CONFPOLOLU_TURRET 120     // PC wants us to configure a Pololu device connected to Serial 2 for use with turret motors
#define PCCMD_NUM_CHANNELS      121     // The PC wants to know the number of utilized channels
#define PCCMD_STARTSTREAM_RADIO 122     // The PC wants us to stream channel pulse-widths
#define PCCMD_STOPSTREAM_RADIO  123     // The PC wants us to stop streaming channel data
#define PCCMD_UPDATE_EEPROM     124     // PC has given us a value to write to EEPROM
#define PCCMD_READ_EEPROM       126     // PC wants us to read EEPROM and return value
#define PCCMD_READ_VERSION      128     // PC wants to know what firmware version we're running
#define PCCMD_STAY_AWAKE        129     // PC is tellings us to stay on the line
#define PCCMD_MINOPC_VERSION    130     // PC requests the minimum version of OP Config the current version of TCB firmware requires
#define PCCMD_READ_HARDWARE     131     // PC requests what hardware this is
#define PCCMD_DISCONNECT        31      // PC tells us to disconnect

// "Commands" returned by device
#define DVCMD_RADIO_NOTREADY    132     // The radio is not ready - can't return radio stream
#define DVCMD_NEXT_SENTENCE     133     // Done, what's next
#define DVCMD_REPEAT_SENTENCE   135     // Please repeat the last request
#define DVCMD_RETURN_VALUE      136     // Device is returning the requested value
#define DVCMD_NOSUCH_VALUE      137     // Device could not find the requested value
#define DVCMD_GOODBYE           138     // Device is disconnecting

// IDs returned by device that are NOT EEPROM
// Note that in most non-EEPROM operations, the device actually returns a copy of the sent command in the ID slot.
// But radio streaming is an exception. 
#define DVID_RADIOSTREAM_LO     401
#define DVID_RADIOSTREAM_HI     402

// IDs returned by device that ARE EEPROM
#define MIN_EEPROM_ID           1000    // Any ID over this number will be the ID of an eeprom variable. Any ID below it will be something else

// Safety
#define SERIAL_COMM_TIMEOUT     8500    // How many milliseconds of inactivity do we wait before we just disconnect from the computer (1000 ms = 1 second)
                                        // We set this to slightly more than 2 times the STAY_AWAKE_BEEP_TIME in OPConfig (4000). That means the PC 
                                        // can send up to 2 stay awake commands without a response from us (or without being able to read the response from us)
                                        // before it initiates a disconnection. It also means we get multiple chances at reading the stay awake command before we
                                        // initiate a disconnect. 
#define MAX_COMM_ERRORCOUNT     10      // How many communication errors do we permit before simply disconnecting


// We default to communicating with the computer through Serial 0 (aka, "Serial") which is through the USB connector.
#define DEFAULT_SERIAL_PORT     Serial  
// However we do have an alternate port on the board we could use, hardware Serial1
// The switchToAltSerial() function will switch us to whatever is defined here. 
#define ALT_SERIAL_PORT         Serial1

#define VALUE_BUFF              15      // Max amount of bytes it could take to print any single value in the sentence. Firmware version is max 9 chars with the delimiter, longest we have so far.
                                        // But we also use VALUE_BUFF for an entire prefix (Command/Delimeter/ID/Delimeter) which could reach 10 bytes. 
                                        // And technically a 4 byte long could equal a number 11 bytes long with delimiter, but we don't have any numbers that big so far. 
#define SENTENCE_BUFF           64      // Max amount of bytes a single sentence could take. If sending 8 channels of data back, it would be about 55 bytes. Arduino's
                                        // transmit buffer is only 64 bytes, so we try not to exceed that. It is possible to change the Arduino buffer and possible to do a lot of things
                                        // but we are trying to keep this compatible. 

struct SentencePrefix {
    uint8_t     Command = 0;
    uint16_t    ID = 0;
};

struct DataSentence {
    uint8_t     Command = 0;
    uint16_t    ID = 0;
    uint32_t    Value = 0;
    int16_t     Checksum = 0;
};


// Class OP_PCComm
class OP_PCComm
{   public:
        OP_PCComm(void);                            // Constructor
        static void begin(OP_EEPROM *, OP_Radio *, DEVICE); // Begin
        
        // Functions 
        static boolean CheckPC(void);               // Did the PC talk to us? 
        static void ListenToPC(void);               // Listens to PC and takes whatever actions it commands
        
        static void switchToAltSerial(void);        // For changing the communication port to the ALT_SERIAL_PORT defined above (Serial1)
        static void revertToDefaultSerial(void);    // For reverting to the DEFAULT_SERIAL_PORT defined above (Serial0, aka, Serial)
        
        static void skipCRC();      // Mostly used for testing, we can skip CRC checking (the CRC portion of the sentence must still be provided though, it just won't be checked)
        static void requireCRC();   
                
    private:
        // Functions
        static boolean ReadData(void);                              // Process incoming bytes on the serial port
        static boolean ParseSentence(char *data, int datasize);     // Try to convert a full line of data into a sentence
        static void ProcessCommand(void);                           // Do whatever the computer asked us to
        
        static void AskForNextSentence(void);
        static void AskForNextSentence_wError(void);
        static void RepeatSentence(void);
        static void TellPC_Goodbye(void);
        static void GivePC_Value_byID(uint16_t ID);                 // Sends an eeprom value by eeprom variable ID
        static void GivePC_Int(uint16_t returnID, int32_t val);     // Sends an arbitrary value up to int32
        static void GivePC_FirmwareVersion(void);
		static void GivePC_HardwareVersion(void);		
        static void GivePC_MinOPCVersion(void); 
        static void sendNullValueSentence(uint8_t command, boolean setValueFlag = false);
        static void prefixToByteArray(SentencePrefix s, char *prefixOut, uint8_t prefixBUFF, uint8_t &returnStrLen);

        static int32_t constructNumber(char *c, int numBytes);
        static int16_t calcrc(char *ptr, int16_t count);
        
        static void startWatchdog(void);
        static void resetWatchdog(void);
        static void triggerWatchdog(void);
        static void updateTimer(void);
        
        static void StartLEDs(void);    // These let us control the onboard Red and Green LEDs while we are in communication mode. 
        static void UpdateLEDs(void);   // While in communication mode, the Red LED will be on steady, and the Green LED will blink slowly. 
        static void StopLEDs(void);     
        
                
        // Vars
        static OP_EEPROM        *_op_eeprom;
        static HardwareSerial   *_serial;
        static OP_Radio         *_radio;
		static uint8_t			_hardwareDevice;
        static uint32_t         WatchdogStartTime;
        static boolean          Timeout;
        static boolean          Disconnect;
        static boolean          eepromUpdated;
        static boolean          CRCRequired;
        static int              numErrors;
        static DataSentence     SentenceIN;
        
};


#endif //OP_PCComm_h

