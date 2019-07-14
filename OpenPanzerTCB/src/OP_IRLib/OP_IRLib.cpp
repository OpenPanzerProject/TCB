/* OP_IRLib.cpp     Open Panzer Infra-red library - functions for encoding and decoding model RC tank infrared signals
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


#include <Arduino.h>
#include "OP_IRLib.h"
#include "OP_IRLibMatch.h"


// ==========================================================================================================================>>
// UTILITIES AND GENERAL DEFINITIONS
// ==========================================================================================================================>>

// defines for setting and clearing register bits
#ifndef cbi
    #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
    #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define CLKFUDGE 5      // fudge factor for clock interrupt overhead

#ifdef F_CPU
    #define SYSCLOCK F_CPU     // main Arduino clock
#else
    #define SYSCLOCK 16000000  // main Arduino clock
#endif

#define PRESCALE 8      // timer clock prescale
#define CLKSPERUSEC (SYSCLOCK/PRESCALE/1000000)   // timer clocks per microsecond

unsigned char Pin_from_Intr(unsigned char inum) 
{
  const unsigned char PROGMEM attach_to_pin[]= {
    #if defined(__AVR_ATmega256RFR2__)//Assume Pinoccio Scout
        4,5,SCL,SDA,RX1,TX1,7
    #elif defined(__AVR_ATmega32U4__) //Assume Arduino Leonardo
        3,2,0,1,7
    #elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Assume Arduino Mega 
        2,3, 21, 20, 19, 18
    #else   //Assume Arduino Uno or other ATmega328
        2, 3
    #endif
    };
    #if defined(ARDUINO_SAM_DUE)
        return inum;
    #endif
    if (inum<sizeof attach_to_pin) 
    {   //note this works because we know it's one byte per entry
        return attach_to_pin[inum];
    }
    else 
    {
        return 255;
    }
}

// Returns a pointer to a flash stored string that is the name of the protocol received. 
const __FlashStringHelper *ptrIRName(IRTYPES Type) {
  if(Type>LAST_IRPROTOCOL) Type=IR_UNKNOWN;
  const __FlashStringHelper *Names[LAST_IRPROTOCOL+1]={F("Unknown"),F("Tamiya"),F("Tamiya 2-Shot"),F("Tamiya 1/35"), F("Heng Long"),F("Taigen V1"),F("FOV"),F("VsTank"),F("OpenPanzer"), // Cannon IR
                                                       F("Clark Repair"),F("IBU Repair"),F("RCTA Repair"),  // Repair IR
                                                       F("Clark Machine Gun"),F("RCTA Machine Gun"),        // Machine Gun IR
                                                       F("Sony"), F("Taigen V2/V3")};                       // Generic IR, new additions (Taigen V2/V3)
  return Names[Type];
};

// Returns a pointer to a flash stored string that is the name of the team. 
const __FlashStringHelper *ptrIRTeam(IRTEAMS Team) {
  if(Team>LAST_IRTEAM) Team=IR_TEAM_NONE;
  const __FlashStringHelper *Names[LAST_IRTEAM+1]={F("None"),               // None or unknown
                                                   F("2"),F("3"),F("4")};   // FOV Teams
  return Names[Team];
};





// ==========================================================================================================================>>
// IR DECODER
// ==========================================================================================================================>>

/*
 * The IR_ReceiveParams definitions which were located here have been moved to OP_IRLib.h
 */

 /*
 * We've chosen to separate the decoding routines from the receiving routines to isolate
 * the technical hardware and interrupt portion of the code which should never need modification
 * from the protocol decoding portion that will likely be extended and modified. It also allows for
 * creation of alternative receiver classes separate from the decoder classes.
 */
IRdecodeBase::IRdecodeBase(void) {
  rawbuf=(volatile uint16_t*)IR_ReceiveParams.rawbuf;
  IgnoreHeader=false;
  SonyDeviceID = SonyCommand = 0;
  Reset();
};

/*
 * Normally the decoder uses IR_ReceiveParams.rawbuf but if you want to resume receiving while
 * still decoding you can define a separate buffer and pass the address here. 
 * Then IRrecvBase::GetResults will copy the raw values from its buffer to yours allowing you to
 * call IRrecvBase::resume immediately before you call decode.
 */
void IRdecodeBase::UseExtnBuf(void *P){
  rawbuf=(volatile uint16_t*)P;
};

/*
 * Copies rawbuf and rawlen from one decoder to another. 
 */
void IRdecodeBase::copyBuf (IRdecodeBase *source){
   memcpy((void *)rawbuf,(const void *)source->rawbuf,sizeof(IR_ReceiveParams.rawbuf));
   rawlen=source->rawlen;
};

/*
 * This routine is actually quite useful. Allows extended classes to call their parent
 * if they fail to decode themselves.
 */
bool IRdecodeBase::decode(void) {
  return false;
};

void IRdecodeBase::Reset(void) {
  decode_type= IR_UNKNOWN;
  value=0;
  bits=0;
  rawlen=0;
};

#ifndef USE_DUMP
void DumpUnavailable(void) {Serial.println(F("DumpResults unavailable"));}
#endif
// This method dumps useful information about the decoded values.
void IRdecodeBase::DumpResults(void) {
#ifdef USE_DUMP

    // I find this simple dump much more useful for tank-decoding purposes
    // But nothing beats an o-scope on the IR transmitter
    Serial.print(F("Gap: ")); Serial.println(rawbuf[0]);
    for (uint8_t i = 1; i < rawlen; i++)
    {
        Serial.print(F("Bit ")); Serial.print(i); Serial.print(F(": "));
        Serial.println(rawbuf[i]);
    }

// This is the dump routine that came with the library
/*
    int i;
    uint32_t Extent;
    int interval;
    
    if(decode_type<=LAST_IRPROTOCOL)
    {
        Serial.print(F("Decoded ")); Serial.print(ptrIRName(decode_type));
        Serial.print(F("(")); Serial.print(decode_type,DEC);
        Serial.print(F("): Value:")); Serial.print(value, HEX);
    };

    Serial.print(F(" ("));  Serial.print(bits, DEC); Serial.println(F(" bits)"));
    Serial.print(F("Raw samples (incl gap): ")); Serial.println(rawlen, DEC);
    Serial.print(F("): Gap:")); Serial.println(rawbuf[0], DEC);

    Serial.println(F("#,Mark,Space"));
    Serial.print(F("Head,")); Serial.print(rawbuf[1], DEC);
    Serial.print(F(",")); Serial.println(rawbuf[2], DEC);

    int LowSpace= 32767; int LowMark=  32767;
    int HiSpace=0; int HiMark=  0;
    Extent=rawbuf[1]+rawbuf[2];
    for (i = 3; i < rawlen; i++) 
    {
        Extent+=(interval= rawbuf[i]);
        if (i % 2) {
          LowMark=min(LowMark, interval);  HiMark=max(HiMark, interval);
          // print the bit number (0-rawlen)
          Serial.print(i/2-1,DEC);  
          // column separator
          Serial.print(F(","));
        } 
        else {
           if(interval>0)LowSpace=min(LowSpace, interval);  HiSpace=max (HiSpace, interval);
           //column separator
           Serial.print(F(","));
        }
        // This prints the interval in us
        Serial.print(interval, DEC);
        // A number is "even" when you can divide it by two and have zero remainder
        // If not odd numbered (ie, if even), new line. 
        // I want mark (odd) then space (even) then new line.
        // 0 is gap, 1 (odd) is header mark, 2 (even) is header space, and beyond that we have these 
        // odd (mark), even (space) then new line
        if ((!(i % 2)))Serial.println();
    }
    Serial.println();
    Serial.print(F("Extent="));  Serial.println(Extent,DEC);
    Serial.print(F("Mark  min:")); Serial.print(LowMark,DEC);Serial.print(F("\t max:")); Serial.println(HiMark,DEC);
    Serial.print(F("Space min:")); Serial.print(LowSpace,DEC);Serial.print(F("\t max:")); Serial.println(HiSpace,DEC);
    Serial.println();
*/
#else
  DumpUnavailable();
#endif
}

/*
 * This routine has been modified significantly from the original IRremote.
 * It assumes you've already called IRrecvBase::GetResults and it was true.
 * The purpose of GetResults is to determine if a complete set of signals
 * has been received. It then copies the raw data into your decoder's rawbuf
 * By moving the test for completion and the copying of the buffer
 * outside of this "decode" method you can use the individual decode
 * methods or make your own custom "decode" without checking for
 * protocols you don't use.
 * Note: Don't forget to call IRrecvBase::resume(); after decoding is complete.
 */
 // This routine will try to decode every protocol and return true on the first one that matches.
 // This function isn't used by the TCB. 
 // It is better to use the overloaded function below and pass a specific, single protocol to decode,
 // assuming you know which protocol you want. 
bool IRdecode::decode(void) {
  if (IRdecodeTamiya::decode())         { decode_type = IR_TAMIYA;      return true; }
  if (IRdecodeTamiya_2Shot::decode())   { decode_type = IR_TAMIYA_2SHOT; return true; }
  if (IRdecodeTamiya35::decode())       { decode_type = IR_TAMIYA_35;   return true; }
  if (IRdecodeHengLong::decode())       { decode_type = IR_HENGLONG;    return true; }
  if (IRdecodeTaigenV1::decode())       { decode_type = IR_TAIGEN_V1;   return true; }  // Taigen V1
  if (IRdecodeTaigen::decode())         { decode_type = IR_TAIGEN;      return true; }  // Taigen V2/V3
  if (IRdecodeFOV::decode())            { decode_type = IR_FOV;         return true; }  
  if (IRdecodeVsTank::decode())         { decode_type = IR_VSTANK;      return true; }
  if (IRdecodeOpenPanzer::decode())     { decode_type = IR_OPENPANZER;  return true; }
  if (IRdecodeSony::decode() && value == Clark_REPAIR_CODE) { decode_type = IR_RPR_CLARK;   return true; }
  if (IRdecodeIBU_Repair::decode())     { decode_type = IR_RPR_IBU;     return true; }
  if (IRdecodeRCTA_Repair::decode())    { decode_type = IR_RPR_RCTA;    return true; }
  if (IRdecodeSony::decode() && value == Clark_MG_CODE) { decode_type = IR_MG_CLARK;    return true; }
  if (IRdecodeRCTA_MG::decode())        { decode_type = IR_MG_RCTA;     return true; }
  if (IRdecodeSony::decode())           { decode_type = IR_SONY;        return true; }
  // If we make it to here, nothing was decoded
  decode_type = IR_UNKNOWN;
  return false;
}

// Here is a more direct version. Pass the type, it only attempts to decode that one protocol
bool IRdecode::decode(IRTYPES Type) {
// Start by setting decode_type to unknown
decode_type = IR_UNKNOWN;
    // Now decode only the type sent, if it is successful, set decode_type to that Type
    switch(Type) 
    {
        case IR_TAMIYA:         if (IRdecodeTamiya::decode())       { decode_type = Type; } break; 
        case IR_TAMIYA_2SHOT:   if (IRdecodeTamiya_2Shot::decode()) { decode_type = Type; } break; 
        case IR_TAMIYA_35:      if (IRdecodeTamiya35::decode())     { decode_type = Type; } break; 
        case IR_HENGLONG:       if (IRdecodeHengLong::decode())     { decode_type = Type; } break; 
        case IR_TAIGEN_V1:      if (IRdecodeTaigenV1::decode())     { decode_type = Type; } break;  // Taigen V1 
        case IR_TAIGEN:         if (IRdecodeTaigen::decode())       { decode_type = Type; } break;  // Taigen V2/V3
        case IR_FOV:            if (IRdecodeFOV::decode())          { decode_type = Type; } break; 
        case IR_VSTANK:         if (IRdecodeVsTank::decode())       { decode_type = Type; } break;
        case IR_OPENPANZER:     if (IRdecodeOpenPanzer::decode())   { decode_type = Type; } break; 
        case IR_RPR_CLARK:      if (IRdecodeSony::decode() && value == Clark_REPAIR_CODE) { decode_type = Type; } break; 
        case IR_RPR_IBU:        if (IRdecodeIBU_Repair::decode())   { decode_type = Type; } break; 
        case IR_RPR_RCTA:       if (IRdecodeRCTA_Repair::decode())  { decode_type = Type; } break; 
        case IR_MG_CLARK:       if (IRdecodeSony::decode() && value == Clark_MG_CODE) { decode_type = Type; } break; 
        case IR_MG_RCTA:        if (IRdecodeRCTA_MG::decode())      { decode_type = Type; } break;
        case IR_SONY:           if (IRdecodeSony::decode())         { decode_type = Type; } break;
        default:                decode_type = IR_UNKNOWN;  // In this case, the type passed was unrecognized, so nothing to decode
    }
    
// Now return true if we decoded something
if (decode_type == IR_UNKNOWN)  return false;
else return true;
}

bool IRdecodeTamiya::decode(void) {
// The Tamiya signal is very simple - two marks separated by a space, followed by a longer gap between re-transmissions:
// 3000uS On, 3000 Off, 6000 On, 8000 Off
// Tamiya repeats the signal about 50 times, but we only need to read it once. 
// Because the GAP between transmissions is less than what we count as a GAP (due to Heng Long using such a long data space),
// the Tamiya signal will arrive as one long stream instead of many repetitions. This means if we don't catch it right at the beginning, and we 
// often won't, we can't be sure where the start is. That is why we add a bit of extra code here to find the first matching start mark and then
// proceed to decode from there. 

    OP_IRLib_ATTEMPT_MESSAGE(F("Tamiya"));   

    if (rawlen <= Tamiya_BITS) return RAW_COUNT_ERROR;

    // Increment through buffer to find start
    for (unsigned char i=0; i<rawlen; i++)
    {   
        // Try to match the first mark (marks are odd elements of rawbuf, meaning i modulus 2!=0)
        if ( (i % 2 != 0)  && MATCH(rawbuf[i], pgm_read_word_near(&(Tamiya16Sig[0]))))
        {
            // We matched the first mark. Start from here. 
            // Check each item in the buffer against the Tamiya array. If at any point the bit length doesn't match, exit.      
            for (unsigned char j=0; j<Tamiya_BITS; j++)
            {   
                if (!MATCH(rawbuf[i + j], pgm_read_word_near(&(Tamiya16Sig[j])))) 
                {
                    return DATA_MARK_ERROR(pgm_read_word_near(&(Tamiya16Sig[j])));
                }
            }
            // If we make it to here, the signal was matched. 
            bits = Tamiya_BITS;
            value = 0;          // The Tamiya signal doesn't have a data value
            return true;
        }
    }
    // If we make it here, no match. 
  return DATA_MARK_ERROR(pgm_read_word_near(&(Tamiya16Sig[0])));
}
bool IRdecodeTamiya_2Shot::decode(void) {
// The Tamiya 2-shot kill signal is very simple - two marks separated by a space, followed by a longer gap between re-transmissions:
// 4000uS On, 5000 Off, 3000 On, 8000 Off
// Tamiya repeats the signal about 50 times, but we only need to read it once. 
// Because the GAP between transmissions is less than what we count as a GAP (due to Heng Long using such a long data space),
// the Tamiya signal will arrive as one long stream instead of many repetitions. This means if we don't catch it right at the beginning, and we 
// often won't, we can't be sure where the start is. That is why we add a bit of extra code here to find the first matching start mark and then
// proceed to decode from there. 

    OP_IRLib_ATTEMPT_MESSAGE(F("Tamiya 2-Shot"));   

    if (rawlen <= Tamiya_BITS) return RAW_COUNT_ERROR;

    // Increment through buffer to find start
    for (unsigned char i=0; i<rawlen; i++)
    {   
        // Try to match the first mark (marks are odd elements of rawbuf, meaning i modulus 2!=0)
        if ( (i % 2 != 0)  && MATCH(rawbuf[i], pgm_read_word_near(&(Tamiya16TwoShotSig[0]))))
        {   
            // We matched the first mark. Start from here. 
            // Check each item in the buffer against the Tamiya array. If at any point the bit length doesn't match, exit.      
            for (unsigned char j=0; j<Tamiya_BITS; j++)
            {   
                if (!MATCH(rawbuf[i + j], pgm_read_word_near(&(Tamiya16TwoShotSig[j])))) 
                {
                    return DATA_MARK_ERROR(pgm_read_word_near(&(Tamiya16TwoShotSig[j])));
                }
            }
            // If we make it to here, the signal was matched. 
            bits = Tamiya_BITS;
            value = 0;          // The Tamiya signal doesn't have a data value
            return true;
        }
    }
    // If we make it here, no match. 
    return DATA_MARK_ERROR(pgm_read_word_near(&(Tamiya16TwoShotSig[0])));
    
}
 bool IRdecodeTamiya35::decode(void) {
// Any hope the 1/35th protocol would be similar to the 1/16th was dashed when I scoped it.
// The protocol has two lengths which it uses for both marks and spaces. Short is always 500uS and long is always 1500uS. 
// The only exception is the header, which is a short mark (500uS) followed by a space of 3,000uS. 
// A "one" data bit is indicated by a long mark followed by a short space
// A "zero" data bit is indicated by a short mark followed by a long space
// Header: On 500uS, Off 3000uS
// Data: One  = On 1500uS, Off 500uS
//       Zero = On 500uS,  Off 1500us
// After the header, there are 64! bits of data. We presume these are meant to be 8 individual bytes of 8 bits each. 
// The total signal of header plus 64 bits is repeated for precisely 1 second. This doesn't allow an even number of transmissions, it actually gets repeated about 7.5 times by Tamiya. 
// There is no gap between re-transmissions other than we can look for the 3000uS space after the header mark to determine the beginning. 
// The decimal values for the 8 bytes are as follows: 199, 242, 192, 120, 135, 165, 183, 197

// Prior to adding this protocol, our IR_ReceiveParams.rawbuf[] array was capped at 50 elements. But to store a single transmission of this protocol would require 130 elements, 
// and you'd probably have to double it to make sure you always had space for a full transmission in those cases where we first picked up the signal in the middle of
// a transmission. Because our GAP setting is way over 3000uS and needs to be for other protocols, the decoder won't naturally be splitting the incoming Tamiya signal into 
// multiples, it will look like one long transmission. But increasing the size of this array has a direct impact on RAM and we are already running low. 

// What we are going to do instead is leave the array at 50. In the best case scenario where we detect the signal at the very beginning, this lets us read the first three bytes
// (header plus 24 data bits). Since so far as we know Tamiya isn't actually sending useful information and it might as well be random numbers, it doesn't matter if we decode the full
// 64 data bits. As long as even the first 24 data bits are unique to this protocol (they are), then it is enough for us to know we were hit by a Tamiya 1/35 model and not something else. 

// Cons - if we pick up the signal sometime in the middle, we simply won't be able to decode it. To be absolutely certain we avoided this possibility we would have to set RAWBUFF to 260
//        but the max it can even go is 255. However, anything over 130 would improve our chances. Anything below 130 it doesn't matter whether it's 50 or 120 - if you show up in the middle, 
//        you are not going to decode it. 
//      - This is a general con - Tamiya may use a different set of 8 bytes for different models. We tested the #48212 Sherman. It could be they use different numbers for each model 
//        but each model is able to decode each one. I doubt this is the case, but if so, we will need to modify the code a bit. However it doesn't change any of the pros/cons about
//        the code length. 

// Since the TCB is physically incapable of fitting into any 1/35 scale model, I'm not too worried about this being the most robust code implementation. Even so it should work in the 
// majority of cases. But if we adapt this code to smaller boards someday, we may want to revisit this and set RAWBUF to 255. 

    OP_IRLib_ATTEMPT_MESSAGE(F("Tamiya 1/35"));   

    // Because the signal length will far exceed RAWBUF, any reception should always have filled it completely. So here we can just do a check if that is true. 
    if (rawlen < RAWBUF) return RAW_COUNT_ERROR;

    // We will only read one byte of data a time, so we can make this a uint8_t
    uint8_t data = 0;
    // What byte of data are we on (zero-based)
    uint8_t currentByte = 0;
    // Offset will be how far into rawbuf[] we are
    int offset; 
    
    
    // Increment through buffer to find start
    for (unsigned char i=0; i<rawlen; i++)
    {   
        // Try to match the unique header space. It is the only one that is 3000uS long and marks the beginning of the transmission 
        // Spaces are even elements of rawbuf, meaning i modulus 2 = 0)
        if ( (i % 2 == 0)  && MATCH(rawbuf[i], TAMIYA_135_HDR_SPACE))
        {   
            // We matched the header space. Start from i + 1 (the next mark after the header space)
            offset = i + 1;     
            
            // Even though the signal has 8 bytes, we are only going to check the first few (however many are specified in TAMIYA_135_BYTESTOCHECK). 
            // Each byte of course has 8 data bits. And of course each bit is actually made up of two pieces of information - a mark and a space.
            // So for each bit we actually check both and increment offset by 2 even though j will only increment by 1 for each bit. 
            for (unsigned char j=0; j<(TAMIYA_135_BYTESTOCHECK * 8); j++)
            {   
                // Check the mark and determine if it is a 1 or 0
                if (MATCH(rawbuf[offset], TAMIYA_135_LONG_BIT)) 
                {
                    data = (data << 1) | 1;     // Data is 1
                    // After each mark, a space. We increment and check the space.
                    offset++;
                    // If the mark was long, the space should always be the opposite - short
                    if (!MATCH(rawbuf[offset], TAMIYA_135_SHORT_BIT)) 
                    {
                        // Space is incorrect
                        return DATA_SPACE_ERROR(TAMIYA_135_SHORT_BIT);
                    }
                } 
                else if (MATCH(rawbuf[offset], TAMIYA_135_SHORT_BIT)) 
                {
                    data <<= 1;                 // Data is 0
                    // After each mark, a space. We increment and check the space.
                    offset++;
                    // If the mark was short, the space should always be the opposite - long
                    if (!MATCH(rawbuf[offset], TAMIYA_135_LONG_BIT)) 
                    {
                        // Space is incorrect
                        return DATA_SPACE_ERROR(TAMIYA_135_LONG_BIT);
                    }
                } 
                else 
                {
                    return DATA_MARK_ERROR(TAMIYA_135_LONG_BIT); // The mark doesn't match. 
                }

                // Successful read of mark and space. Increment to next mark, repeat loop. 
                offset++;

                // But don't go beyond the limit of rawbuf
                if (offset > RAWBUF)
                {
                    return RAW_COUNT_ERROR;
                }
                
                // And check data every time we reach 8 bits (one byte)
                // Using modulo 8 with no remainder will tell us each time we get to 8 bits. 
                // We add 1 to j because j is zero-based, so j=7 is actually the 8th bit
                if ((j + 1) % 8 == 0)
                {
                    if (data != pgm_read_byte_near(&(Tamiya135Cannon[currentByte])))
                    {   // Correct protocol definition apparently, but wrong data
                        return DATA_ERROR(data, pgm_read_byte_near(&(Tamiya135Cannon[currentByte]))); 
                    }
                    else
                    {
                        // Increment to next byte
                        currentByte += 1;
                        // And start data over
                        data = 0; 
                        
                        // But check if we've read enough bytes. 
                        if (currentByte >= TAMIYA_135_BYTESTOCHECK)
                        {
                            // We've successfuly read enough bytes - we're done
                            bits = TAMIYA_135_BYTESTOCHECK * 8; // How many bits we read
                            value = 0;      // Although there is actually a value, we don't return it because it's too long and we don't need it.
                            return true;                    
                        }
                    }
                }
            }
        }
    }
}
bool IRdecodeHengLong::decode(void) {
// HengLong_BITS = 7
// Heng Long signal in uS:
// 0. 19,000 ON     Header Mark
// 1. 4,700  OFF    Short  Space
// 2. 9,500  ON     Long   Mark     
// 3. 4,700  OFF    Short  Space
// 4. 4,700  ON     Short  Mark
// 5. 9,500  OFF    Long   Space
// 6. 4,700  ON     Short  Mark
// Then a gap of about 40,000 between repetitions, Heng Long repeats 6 times. 

    OP_IRLib_ATTEMPT_MESSAGE(F("HengLong"));   

    if (rawlen < HengLong_BITS) return RAW_COUNT_ERROR;

    // Now check each item in the buffer against the HengLong array. If at any point the bit length doesn't match, exit. 
    for (unsigned char i=0; i<HengLong_BITS; i++)   // We subtract 2 from the total number of HengLong bits since we are skipping the header mark and space
    {
        if (!MATCH(rawbuf[1 + i], pgm_read_word_near(&(HengLongSig[i])))) // We add 1 to rawbuf in order to skip the first space which isn't part of the signal
        {                                                           
            return DATA_MARK_ERROR(pgm_read_word_near(&(HengLongSig[i])));
        }
    }

    // Success
    bits = HengLong_BITS;
    value = 0;          // The Heng Long signal doesn't have a data "value"
    return true;                        
}
bool IRdecodeTaigenV1::decode(void) {
// Taigen signal is very simple and very brief. It is only sent once by the Taigen unit: 
// Signal in uS:
// 0. 620  ON   Mark
// 1. 570  OFF  Space
// 2. 620  ON   Mark
// 3. 570  OFF  Space
// 4. 620  ON   Mark
// 5. 570  OFF  Space
// 6. 620  ON   Mark
// Taigen only sends this once, but if the TCB is sending it repeats it 6 times with a gap of 14,000 uS in-between
// The marks and spaces are very short and nearly the same length as each other. Sony protocols have a similar spacing
// and may be confused for a Taigen cannon hit. But for this to happen you must be set to receive Taigen hits and be hit
// by a Sony code, so probably not terribly likely (Clark uses Sony codes for machine gun and repair). 
// A Sony code can be confused for a Taigen hit, but a Taigen code will not be confused with a Sony code. 

    OP_IRLib_ATTEMPT_MESSAGE(F("Taigen V1"));   

    if (rawlen <= TaigenV1_BITS) return RAW_COUNT_ERROR;

    // Now check each item in the buffer against the the Taigen space or mark length, depending. If at any point the bit length doesn't match, exit. 
    for (unsigned char i = 0; i < TaigenV1_BITS; i++) 
    {
        if (i & 1)  // Odd numbers - spaces
        {
            if (!MATCH(rawbuf[1 + i], TaigenV1_SPACE))    return DATA_SPACE_ERROR(TaigenV1_SPACE);
        } 
        else        // Even numbers - marks
        {
            if (!MATCH(rawbuf[1 + i], TaigenV1_MARK))     return DATA_MARK_ERROR(TaigenV1_MARK);
        }
    }

    // Success
    bits = TaigenV1_BITS;
    value = 0;          // The Taigen signal doesn't have a data value
    return true;
}
bool IRdecodeTaigen::decode(void) {
// Taigen V2 and V3 signals are the same. Very similar to V1 except Mark has been reduced to 600uS and space increased to 620, also 9 marks are sent instead of 4. 
// As with Taigen V1, the signal is sent only a single time with no repeats. 
// Signal in uS:
// 0. 600  ON   Mark        | Repeat 9 times
// 1. 620  OFF  Space       |
// .... 
// 16. 620 ON   Mark        | Except instead of a space at the last, we do GAP
// 17.          GAP         | We make this up
// Taigen only sends this once, but if the TCB is sending it repeats it 6 times with a gap of 14,000 uS in-between
// The marks and spaces are very short and nearly the same length as each other. Sony protocols have a similar spacing
// and may be confused for a Taigen cannon hit. But for this to happen you must be set to receive Taigen hits and be hit
// by a Sony code, so probably not terribly likely (Clark uses Sony codes for machine gun and repair). 
// A Sony code can be confused for a Taigen hit, but a Taigen code will not be confused with a Sony code. 

    OP_IRLib_ATTEMPT_MESSAGE(F("Taigen V2/V3"));   

    if (rawlen <= Taigen_BITS) return RAW_COUNT_ERROR;

    // Now check each item in the buffer against the the Taigen space or mark length, depending. If at any point the bit length doesn't match, exit. 
    for (unsigned char i = 0; i < Taigen_BITS; i++) 
    {
        if (i & 1)  // Odd numbers - spaces
        {
            if (!MATCH(rawbuf[1 + i], Taigen_SPACE))    return DATA_SPACE_ERROR(Taigen_SPACE);
        } 
        else        // Even numbers - marks
        {
            if (!MATCH(rawbuf[1 + i], Taigen_MARK))     return DATA_MARK_ERROR(Taigen_MARK);
        }
    }

    // Success
    bits = Taigen_BITS;
    value = 0;          // The Taigen signal doesn't have a data value
    return true;
}
bool IRdecodeFOV::decode(void) {
// FOV is surely the best thought-out IR code of all the many tank manufacturers. These models were discontinued in the early 2010s. 
// After a distinct header mark there are 8 data bits which construct a single integer from 0-256. Different numbers represent different teams. 
uint32_t data = 0;
    OP_IRLib_ATTEMPT_MESSAGE(F("FOV"));  
    
    if (rawlen < (FOV_DATA_BITS*2)+2) return RAW_COUNT_ERROR;

    int offset = 1; // Skip first item in the array (rawbuf[0]), it's not part of the data. 
    
    // Initial mark
    if (!MATCH(rawbuf[offset], FOV_HDR_MARK)) return HEADER_MARK_ERROR(FOV_HDR_MARK);


    // Move the next item - the first space after the header mark
    offset++; 
    
    // There should now be 8 pairs of spaces followed by marks
    for (uint8_t i=0; i<FOV_DATA_BITS; i++)
    {
        if (!MATCH(rawbuf[offset], FOV_SPACE)) 
        {
            // Space is incorrect
            return DATA_SPACE_ERROR(FOV_SPACE);
        }
        // After each space, a mark. We increment and check the mark. 
        offset++;
        if (MATCH(rawbuf[offset], FOV_ONE_MARK)) 
        {
            data = (data << 1) | 1;
        } 
        else if (MATCH(rawbuf[offset], FOV_ZERO_MARK)) 
        {
            data <<= 1;
        } 
        else 
        {
            return DATA_MARK_ERROR(FOV_ONE_MARK); // but actually we don't know if the error was a one mark or zero mark
        }
        // Increment to next space, repeat loop 
        offset++;
    }

    // Success
    bits = FOV_DATA_BITS;
    value = data;
    return true;
}
bool IRdecodeVsTank::decode(void) {
// The VsTank IR protocol similar in some ways to FOV in that there are 8 data bits, however to my knowledge VsTank does not
// use this to send distinct numbers (so far they have no team capability). The timing is also a bit different and unlike FOV, 
// both marks *and* spaces can vary in length, which is a little confusing. 
uint32_t data = 0;
boolean nextMarkLong;
    OP_IRLib_ATTEMPT_MESSAGE(F("VsTank"));  
    
    if (rawlen < (VsTank_DATA_BITS*2)+2) return RAW_COUNT_ERROR;

    int offset = 1; // Skip first item in the array (rawbuf[0]), it's not part of the data. 

    // Initial mark
    if (!MATCH(rawbuf[offset], VsTank_HDR_MARK)) return HEADER_MARK_ERROR(VsTank_HDR_MARK);
    
    // Move to the next item - the first space after the header mark
    offset++; 

    // There should now be 8 pairs of spaces followed by marks
    for (uint8_t i=0; i<VsTank_DATA_BITS; i++)
    {
        // Spaces determine the data
        if (MATCH(rawbuf[offset], VsTank_LONG_BIT)) 
        {
            data = (data << 1) | 1; // Long space, equals 1
            nextMarkLong = false;   // Short marks follow long spaces
        } 
        else if (MATCH(rawbuf[offset], VsTank_SHORT_BIT)) 
        {
            data <<= 1;             // Short space, equals 0
            nextMarkLong = true;    // Long marks follow short spaces
        } 
        else
        {   // Space is incorrect
            return DATA_SPACE_ERROR(rawbuff[offset]);
        }
        
        // After each space, a mark. We increment and check the mark. 
        offset++;
        if      ( nextMarkLong && MATCH(rawbuf[offset], VsTank_LONG_BIT )) { /*ok*/ }
        else if (!nextMarkLong && MATCH(rawbuf[offset], VsTank_SHORT_BIT)) { /*ok*/ }
        else
        {   // Mark is incorrect
            return DATA_MARK_ERROR(rawbuff[offset]);        
        }
        // Increment to next space, repeat loop 
        offset++;
    }

    // Success - maybe
    bits = VsTank_DATA_BITS;
    value = data;
    // Check to make sure we got the right number. For now I only know of one number, but others may crop up in the future. 
    if (value == VsTank_HIT_VALUE)  return true;
    else return false;
}
bool IRdecodeOpenPanzer::decode(void) {
    uint32_t data = 0;
    OP_IRLib_ATTEMPT_MESSAGE(F("OpenPanzer"));   

    // Placeholder for future expansion
    
    value = 0;
    return false;
}
bool IRdecodeIBU_Repair::decode(void) {
// That IBU2 Repair signal is very simple - two marks and two spaces, repeated 50 times. The very first mark is slightly
// longer than the first mark of the remaining 49 repetitions. First pair goes like this: 
// 20000uS On, 5000 Off, 15000 On, 10000 Off
// The remaining 49 times go like this:
// 10000uS On, 5000 Off, 15000 On, 10000 Off
// For simplicity we only check the second version that is repeated 49 times. That means we will miss the first two marks but there's
// still plenty of signal after that to pick up. 
// Because the GAP between transmissions is less than what we count as a GAP (due to Heng Long using such a long data space),
// the IBU signal will arrive as one long stream instead of 50 repetitions. This means if we don't catch it right at the beginning, and we 
// often won't, we can't be sure where the start is. That is why we add a bit of extra code here to find the first matching start mark and then
// proceed to decode from there. 

    OP_IRLib_ATTEMPT_MESSAGE(F("IBU2 Repair"));   

    if (rawlen <= IBU2_BITS) return RAW_COUNT_ERROR;    
    
    // Increment through buffer to find start
    for (unsigned char i=0; i<rawlen; i++)
    {   
        // Try to match the first mark (marks are odd elements of rawbuf, meaning i modulus 2!=0)
        if ( (i % 2 != 0)  && MATCH(rawbuf[i], pgm_read_word_near(&(IBU2RepairSig[0]))))
        {   
            // We matched the first mark. Start from here. 
            // Check each item in the buffer against the IBU array. If at any point the bit length doesn't match, exit.         
            for (unsigned char j=0; j<IBU2_BITS; j++)
            {   
                if (!MATCH(rawbuf[i + j], pgm_read_word_near(&(IBU2RepairSig[j])))) 
                {
                    return DATA_MARK_ERROR(pgm_read_word_near(&(IBU2RepairSig[j])));
                }
            }
            // If we make it to here, the signal was matched. 
            bits = IBU2_BITS;
            value = 0;          // The IBU signal doesn't have a data value
            return true;
        }
    }
    // If we make it here, no match. 
    return DATA_MARK_ERROR(pgm_read_word_near(&(IBU2RepairSig[0])));
}
bool IRdecodeRCTA_Repair::decode(void) {
// RC Tanks Australia repair signal is very simple: 4000 uS ON, 1500 OFF, 2000 ON, 2500 OFF, repeated 32 times
// Because the GAP between transmissions is less than what we count as a GAP (due to Heng Long using such a long data space),
// the RCTA signal will arrive as one long stream instead of 32 repetitions. This means if we don't catch it right at the beginning, and we 
// often won't, we can't be sure where the start is. That is why we add a bit of extra code here to find the first matching start mark and then
// proceed to decode from there. 

    OP_IRLib_ATTEMPT_MESSAGE(F("RCTA Repair"));   

    if (rawlen <= RCTA_BITS) return RAW_COUNT_ERROR;

    // Increment through buffer to find start
    for (unsigned char i=0; i<rawlen; i++)
    {   
        // Try to match the first mark (marks are odd elements of rawbuf, meaning i modulus 2!=0)
        if ( (i % 2 != 0)  && MATCH(rawbuf[i], pgm_read_word_near(&(RCTARepairSig[0]))))
        {   
            // We matched the first mark. Start from here. 
            // Check each item in the buffer against the RCTA array. If at any point the bit length doesn't match, exit.        
            for (unsigned char j=0; j<RCTA_BITS; j++)
            {   
                if (!MATCH(rawbuf[i + j], pgm_read_word_near(&(RCTARepairSig[j])))) 
                {
                    return DATA_MARK_ERROR(pgm_read_word_near(&(RCTARepairSig[j])));
                }
            }
            // If we make it to here, the signal was matched. 
            bits = RCTA_BITS;
            value = 0;          // The RCTA signal doesn't have a data value
            return true;
        }
    }
    // If we make it here, no match. 
    return DATA_MARK_ERROR(pgm_read_word_near(&(RCTARepairSig[0])));
}
bool IRdecodeRCTA_MG::decode(void) {
// RC Tanks Australia machine gun signal is very simple: 8000 uS ON, 6000 OFF, 2000 ON, 4000 OFF, repeated 20 times by RCTA devices. 
// When sent by the TCB, it will be repeated in 3-shot bursts every MG_REPEAT_TIME_mS. 
// Because the GAP between transmissions is less than what we count as a GAP (due to Heng Long using such a long data space),
// the RCTA signal will arrive as one long stream instead of individual repetitions. This means if we don't catch it right at the beginning, and we 
// often won't, we can't be sure where the start is. That is why we add a bit of extra code here to find the first matching start mark and then
// proceed to decode from there. 

    OP_IRLib_ATTEMPT_MESSAGE(F("RCTA Machine Gun"));   

    if (rawlen <= RCTA_BITS) return RAW_COUNT_ERROR;
    
    // Increment through buffer to find start
    for (unsigned char i=0; i<rawlen; i++)
    {   
        // Try to match the first mark (marks are odd elements of rawbuf, meaning i modulus 2!=0)
        if ( (i % 2 != 0)  && MATCH(rawbuf[i], pgm_read_word_near(&(RCTAMGSig[0]))))
        {
            // We matched the first mark. Start from here. 
            // Check each item in the buffer against the RCTA array. If at any point the bit length doesn't match, exit.
            for (unsigned char j=0; j<RCTA_BITS; j++)
            {   
                if (!MATCH(rawbuf[i + j], pgm_read_word_near(&(RCTAMGSig[j])))) 
                {
                    return DATA_MARK_ERROR(pgm_read_word_near(&(RCTAMGSig[j])));
                }
            }
            // If we make it to here, the signal was matched. 
            bits = RCTA_BITS;
            value = 0;          // The Tamiya signal doesn't have a data value
            return true;
        }
    }
    // If we make it here, no match. 
    return DATA_MARK_ERROR(pgm_read_word_near(&(RCTAMGSig[0])));
    
}
// We have the Sony protocol included because Clark uses it for repair and machine gun codes
// Sony protocol can be 8, 12, 15, or 20 bits in length, but for now we only use the 12 bit codes.
bool IRdecodeSony::decode(void) {
uint32_t data = 0;
    OP_IRLib_ATTEMPT_MESSAGE(F("Sony"));  

    if (rawlen < (Sony_12_BIT*2)+2) return RAW_COUNT_ERROR;

    int offset = 1; // Skip first item in the array (rawbuf[0]), it's not part of the data. 

    // Initial mark
    if (!MATCH(rawbuf[offset], Sony_HDR_MARK)) return HEADER_MARK_ERROR(Sony_HDR_MARK);

    // Move the next item - the first space after the header mark
    offset++; 

    // There should now be 12 pairs of spaces followed by marks
    for (uint8_t i=0; i<Sony_12_BIT; i++)
    {
        if (!MATCH(rawbuf[offset], Sony_SPACE)) 
        {
            // Space is incorrect
            return DATA_SPACE_ERROR(Sony_SPACE);
        }
        // After each space, a mark. We increment and check the mark. 
        offset++;
        if (MATCH(rawbuf[offset], Sony_ONE_MARK)) 
        {
            data = (data << 1) | 1;
        } 
        else if (MATCH(rawbuf[offset], Sony_ZERO_MARK)) 
        {
            data <<= 1;
        } 
        else 
        {
            return DATA_MARK_ERROR(Sony_ZERO_MARK); // but actually we don't know if the error was a one mark or zero mark
        }
        // Increment to next space, repeat loop 
        offset++;
    }  

    // Success
    bits = Sony_12_BIT;
    value = data;
    return true;
}
void IRdecodeBase::convertValueToSonyNumbers(uint32_t &val)
{
// See: http://www.righto.com/2010/03/understanding-sony-ir-remote-codes-lirc.html

// The 12-bit Sony codes can be thought of as 12 bits however you want, but Sony breaks them down into a 7-bit Command followed by a 5-bit Device ID, 
// transmitted least-significant-bit first, which is backwards from how we typically read binary data. 
// There are times we might want to parse these 12 bits into Command and DeviceID, which this function will do. 
// (See a similar function in the IRsend class that allows us to send Sony codes by passing a DeviceID and Command)
    
    // The process is basically to break up the 12 bits into the two pieces, convert each piece to a full byte, then swapping all the bits in that byte using a lookup
    // table in progmem. As an example, take the following 12 bit code: 
    // 0100000 10000
    // This is the code Clark uses for machine gun. It could be represented by decimal 1040 (which is what we actually use when decoding Clark MG since that is a lot faster). 
    // But if we were to break this down according to the Sony specification, this would be device ID 1 (right-most 5 bits flipped), Command 2 (left most 7 bits flipped)
    // To get the Sony Device ID:
    // 1. 12-bit code ANDed with 001F: 0100000 10000 & 0000000 11111 = xxxxxxx 10000 : Basically, this converts all the Command bits to 0, and keeps all five Device ID bits the same
    // 2. Next we shift the result of the above left by three bits   = xxxx 10000000 : The reason is, we want our 5 bit DeviceID to take up a full 8 bits. 
    // 3. We save the result in a 8 bit unsigned integer             = 10000000      : So we get rid of the leading zeros
    // 4. Finally we swap all the bits                               = 00000001      : Our Sony Device ID is 1
    // To get the Sony Command: 
    // 1. 12-bit code ANDed with 0FE0: 0100000 10000 & 1111111 00000 = 0100000 xxxxx : This converts all the Device ID bits to 0, and keeps all seven Command bits the same
    // 2. Next shift the result above to the right by four bits:     = xxxx 01000000 : Now our 7-bit command takes up a full 8 bits
    // 3. Save the result in an 8 bit unsigned integer               = 01000000 
    // 4. Finally swap all the bits                                  = 00000010      : Our Sony Command is 2

    uint8_t v; 

    v = (uint8_t)((val & 0x001F) << 3);     // Return a byte of only the right-most 5 bits, but move the 5 bits to the left by 3 so we get a full 8 bit byte
    SonyDeviceID = ReverseByte(v);          // Swap the bits, this is now our Device ID

    v = (uint8_t)((val & 0x0FE0) >> 4);     // Toss the device ID, and leave us with the command bits padded to 8 (by only moving right 4 instead of 5)
    SonyCommand = ReverseByte(v);           // Swap the bits, this is now our Command 
    
    //Serial.print(F("Val: ")); Serial.print(val); Serial.print(F(" DeviceID: ")); Serial.print(SonyDeviceID); Serial.print(F(" Command: ")); Serial.println(SonyCommand);
}



// ==========================================================================================================================>>
// IR RECEIVER
// ==========================================================================================================================>>
volatile ir_receive_params_t IR_ReceiveParams;
volatile uint32_t isrLastTimeStamp;

IRrecvBase::IRrecvBase(unsigned char recvpin)
{
  IR_ReceiveParams.recvpin = recvpin;
  Init();
}

void IRrecvBase::Init(void) 
{
  IR_ReceiveParams.blinkflag = 0;
  Mark_Excess=MARK_EXCESS_DEFAULT;
}

unsigned char IRrecvBase::getPinNum(void)
{
  return IR_ReceiveParams.recvpin;
}

void IRrecvBase::No_Output (void) 
{
    #if defined(IR_SEND_PWM_PIN)
        pinMode(IR_SEND_PWM_PIN, OUTPUT);  
        digitalWrite(IR_SEND_PWM_PIN, LOW); // When not sending PWM, we want it low    
    #endif
}

// enable/disable blinking of an LED on IR processing
void IRrecvBase::setBlinkingOnReceive(bool blinkflag)
{
  IR_ReceiveParams.blinkflag = blinkflag;
  if (blinkflag) pinMode(BLINKLED, OUTPUT);
}

//Do the actual blinking off and on
//This is not part of IRrecvBase because it may need to be inside an ISR
void do_Blink(void) 
{
    if (IR_ReceiveParams.blinkflag) 
    {
        if(IR_ReceiveParams.rawlen % 2) { BLINKLED_ON(); } // turn LED on
        else { BLINKLED_OFF(); } // turn LED off
    }
}

/* Any receiver class must implement a GetResults method that will return true when a complete code
 * has been received. At a successful end of your GetResults code you should then call IRrecvBase::GetResults
 * and it will copy the data from the receiver structures into your decoder. Some receivers
 * provide results in rawbuf measured in ticks on some number of microseconds while others
 * return results in actual microseconds. If you use ticks then you should pass a multiplier
 * value in Time_per_Ticks.
 */
bool IRrecvBase::GetResults(IRdecodeBase *decoder, const uint16_t Time_per_Tick) {
  decoder->Reset();//clear out any old values.
  decoder->rawlen = IR_ReceiveParams.rawlen;
/* Typically IR receivers over-report the length of a mark and under-report the length of a space.
 * This routine adjusts for that by subtracting Mark_Excess from recorded marks and
 * deleting it from a recorded spaces. The amount of adjustment used to be defined in OP_IRLibMatch.h.
 * It is now user adjustable with the old default of 100;
 * By copying the the values from IR_ReceiveParams to decoder we can call IRrecvBase::resume 
 * immediately while decoding is still in progress.
 */
  for(unsigned char i=0; i<IR_ReceiveParams.rawlen; i++) 
  {
    decoder->rawbuf[i]=IR_ReceiveParams.rawbuf[i]*Time_per_Tick + ( (i % 2)? -Mark_Excess:Mark_Excess);
  }
  return true;
}

void IRrecvBase::enableIRIn(void) { 
  pinMode(IR_ReceiveParams.recvpin, INPUT_PULLUP);
  resume();
}

void IRrecvBase::disableIRIn(void) {
    // Disable IR receiver interrupt
    EIMSK &= ~(1 << INT4);   // AND-NOT
}

void IRrecvBase::resume() {
  IR_ReceiveParams.rawlen = 0;
}

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
IRrecvPCI::IRrecvPCI(unsigned char inum) 
{
    Init();             // Base function, initializes blinkflag = 0, sets Mark_Excess=100 as default (can be changed by user myreceiverobject.Mark_Excess = whatever;
    intrnum=inum;       // Sets the interrupt number (not the pin number). From IR_RECEIVE_INT_NUM in OP_Tank.h. Should be 0 for Arduino External Interrupt 0 (Atmega INT 4)
    IR_ReceiveParams.recvpin=Pin_from_Intr(inum); //If you want to know the pin number, check myreceiverobject.getPinNum;
    
    // The interrupt doesn't get enabled until a call to enableIRIn of the base class. This sets the pin to input, 
    // then calls the resume() function of PCI class, which itself calls the resume() of the base class (clears IR_ReceiveParams), then finally
    // attaches the interrupt. 

    // But, we do want to *setup* the interrupt here. Then it is a simple matter of enabling/disabling it.

    // SETUP EXTERNAL INTERRUPT
    // ------------------------------------------------------------------------------------------------------------------------>>
    // EICRB (External Interrupt Control Register B)
    // Only present on chips with more than 4 external interrupts, this one controls external interrupts 4-7
    // There are two bits in the control register for each of the 4 interrupts. We are interested in external interrupt 4
    // so we are going to be setting Interrupt Sense Control bits 4-0 and 4-1 (ISC40, ISC41). These bits determine
    // the type of interrupt that is triggered. 
    // 
    // These are the possible settings: 
    // ISCn1    ISCn0   Type
    // 0        0       Interrupt on low level
    // 0        1       Any change generates interrupt
    // 1        0       Interrupt on falling edge
    // 1        1       Interrupt on rising edge

    // For IR Receiving, we want an interrupt on any change

    // What this line does is basically clear the two ISC4 bits (not 1, ANDed with the two bits)
        EICRB = (EICRB & ~((1 << ISC40) | (1 << ISC41)));
    // Now we set ISC40 which will set it to interrupt on change
        EICRB |= (1 << ISC40);

    // to enable the interrupt, we would do this: 
    // EIMSK |= (1 << INT4);
    // But this will be done by the resume function

    // Here is how we would disable it:
    // EIMSK &= ~(1 << INT4);   // AND-NOT, instead of OR 
}


bool IRrecvPCI::GetResults(IRdecodeBase *decoder) 
{
    // If state is running but we are in the middle of a space (pin high = off/space), then check time for gap
    if(IR_ReceiveParams.rcvstate==STATE_RUNNING && digitalRead(IR_ReceiveParams.recvpin))
    {
        uint32_t ChangeTime = IR_ReceiveParams.timer;
        // The final "space" of any bit stream lasts for eternity, or else, until the next reception. 
        // Since there is no pin change until the next reception, the pin-change interrupt never gets called,
        // the buffer never gets full, and rcvstate never equals STOP (ie, ready). For this reason enabling
        // blink led on reception doesn't work well with IRrecvPCI because it stays on forever after the first transmission.
        // So if we call the decoder let's set the state to stop if the time from last reception is greater than
        // some reasonable number. GAP is a define set in OP_IRLibMatch.h (Chris Young default was 10,000us = 10ms = 0.010 seconds)
        if( (micros()-ChangeTime) > GAP) 
        {
            IR_ReceiveParams.rcvstate=STATE_STOP;
            //Setting gap to 2 is a flag to let you know why we stopped for debugging purposes
            //IR_ReceiveParams.rawbuf[0]=2;
        }
    }
    
    if (IR_ReceiveParams.rcvstate != STATE_STOP) { return false; }
    else
    {
        // Disable interrupt 
        EIMSK &= ~(1 << INT4);              // AND-NOT, instead of OR
        IRrecvBase::GetResults(decoder);    // Call the base function to copy IR_ReceiveParams to the decoder
        return true;
    }
};

ISR(INT4_vect)
{
    boolean StartMark;  
    if (digitalRead(IR_ReceiveParams.recvpin)) { StartMark = false; }   // When the pin goes high, a Mark has ended (switch from on to off). This is now a space. 
    else { StartMark = true; }  // When the pin goes low, a Mark has begun (signal received)
    
    uint32_t volatile TimeStamp = micros();
    uint32_t DeltaTime = TimeStamp - IR_ReceiveParams.timer; // How much time has elapsed since our last check
    
    switch(IR_ReceiveParams.rcvstate) 
    {
        case STATE_STOP: return;    // If we've stopped, stay stopped until the resume() function is called by the user. 
        
        case STATE_RUNNING:         // If we're running
            do_Blink();
            // If a mark is just beginning, a space just ended. Check if the space lasted longer than GAP, and if so, stop receiving. 
            if (StartMark && DeltaTime > GAP) 
            {                                           // If GAP amount of space has occured between this mark and the prior, we quit, ignore the remainder,
                IR_ReceiveParams.rcvstate = STATE_STOP; // and consider our signal complete. Any more transmissions will be missed until we resume()
                return;                                 // But we don't do this check if the pin just went to a space, meaning it was a Mark before - we allow 
                                                        // any length of mark. 
            }
            break;
        
        case STATE_IDLE:    // IDLE - means we are waiting for a mark to begin
            if (StartMark) 
            {   // We're off to the races. Turn the receiver to running and we will start recording the bit lengths. 
                IR_ReceiveParams.rcvstate = STATE_RUNNING;
            }
            else
            {   // If the pin is at SPACE (actually pin high/1) then do nothing. That means 
                // it was on, and is now off. Somehow we missed it turning on. Wait for the next on. 
                return; 
            }
            break;
    };
    
    
    if (IR_ReceiveParams.rcvstate == STATE_RUNNING)
    {
        // IR_ReceiveParams.rawbuf[0] will equal the amount of time since last reception. Not really very interesting, and also almost never accurate
        // because rawbuff is 16 bit long and the time in microseconds from the last reception is almost surely going to overflow that unless it is a
        // repeat signal. 
        // In the original Shirriff/Young code you may see in some places that debugging options set rawbuf[0] to other values, this is because the actual
        // value isn't very useful so they overload it with something else. 
        // But the rest of the array [1 to rawbuf] will be your actual bits (mark and space lengths in us)
        IR_ReceiveParams.rawbuf[IR_ReceiveParams.rawlen] = DeltaTime;
        IR_ReceiveParams.timer = TimeStamp; // What time is it, save.

        // Increment rawlen for next time, but make sure we don't exceed RAWBUF
        if (++IR_ReceiveParams.rawlen >= RAWBUF) 
        {
            IR_ReceiveParams.rcvstate = STATE_STOP;
            return;
            //Setting gap to 1 is a flag to let you know why we stopped For debugging purposes
            //IR_ReceiveParams.rawbuf[0]=1; 
        }   
    }
}

void IRrecvPCI::resume(void) 
{
    // This gets called instead of the base class resume(), but we have a call to the base resume() here to hit it anyway.
    IR_ReceiveParams.rcvstate = STATE_IDLE; // Initiate the state 
    IRrecvBase::resume();                   // This sets IR_ReceiveParams.rawlen = 0 (no input)
    IR_ReceiveParams.timer = micros();      // What time is it? Save to timer.

    // ENABLE EXTERNAL INTERRUPT
    // ------------------------------------------------------------------------------------------------------------------------>>
    // First clear any interrupt flag, which is done by writing a logical one to the flag register
    EIFR |= (1 << INTF4);
    // we set the appropriate bit in the External Interrupt Mask Register (EIMSK). We are turning on external interrupt 4
    // so we SET bit 4 by ORing a 1 shifted over to the correct bit. 
    EIMSK |= (1 << INT4);
}
 




// ==========================================================================================================================>>
// IR SENDER - HARDWARE SPECIFIC
// ==========================================================================================================================>>
// IR Sending is the most changed from Chris Young's library. He and Ken Shirrif before him used Timer 2 interrupts to create the
// actual PWM, which is still the case here, but the transmitted bit pattern ("marks" and "spaces") were sent out using straight delays. 
// These delays blocked the main loop, and could be quite long - for example, if we implemented the Tamiya protocol as Tamiya does (repeated 50 times), 
// we would block the main loop from doing anything for an entire second. One second is an eternity for the microcontroller. 

// The change we have made is to use the Compare register B of Timer 1 to set up an interrupt that will set the pin high (PWM) 
// or low (off) for the marks and spaces. First we construct a stream of data "bits" starting with the header mark and proceeding
// to the end of the signal, including all marks and spaces. These are saved in the IR_SendParams.sendStream[] array. 
// We then connect the output pin to OC2B, and set an intterupt to occur after the specified number of micro-seconds has elapsed 
// as specified in sendStream[0] (the header mark). When the interrupt triggers we set the pin low, and set another interrupt to occur
// after sendStream[1] amount of time has elapsed. Each time an interrupt occurs, we toggle the pin and set the next interrupt to 
// occur after the length of time specified in sendStream. These interrupts do very little work and take almost no time. In the meanwhile
// the sketch and other interrupts are free to run without delay. 

// See OP_Settings.h under Timer 2 for defines related to IR sending

volatile ir_send_params_t IR_SendParams;
                                
IRsendBase::IRsendBase () 
{
    // IRsendBase uses Timer 1 Compare B interrupt. Timer 1 is setup in the main sketch, using a macro defined in OP_Settings.h
 
    // Setup IR send pin
    pinMode(IR_SEND_PWM_PIN, OUTPUT);  
    digitalWrite(IR_SEND_PWM_PIN, LOW); // When not sending PWM, we want it low    
    
    // We are not yet sending
    IR_SendParams.sending = false;
}

void IRsendBase::enableIROut(unsigned char khz) 
{
//NOTE: These comments are specific to the TCB board. The generic IR library allowed the use of different pins and timers
//on different boards. Here we are using Timer 2 on the Atmega 2560 to generate the PWM frequency. 
    // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
    // The IR output will be on Atmega Pin 18/Arduino Pin 9 (OC2B) - this is port H6.
    // This routine is designed for 36-40KHz; if you use it for other values, it's up to you
    // to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
    // TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
    // controlling the duty cycle.
    // There is no prescaling, so the output frequency is 16MHz / (2 * OCR2A)
    // To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
    // A few hours staring at the ATmega documentation and this will all make sense.
    // See Ken Shirriff's "Secrets of Arduino PWM" at http://www.righto.com/2009/07/secrets-of-arduino-pwm.html
 
    // Set the output pin
    pinMode(IR_SEND_PWM_PIN, OUTPUT);  
    digitalWrite(IR_SEND_PWM_PIN, LOW); // When not sending PWM, we want it low    
    IR_SEND_CONFIG_KHZ(khz);            // Configure frequency
}

// Timer1 Output Compare B interrupt service routine
ISR(TIMER1_COMPB_vect)
{   // This triggers when TCNT1 = OCR1B
    IRsendBase::OCR1B_ISR();
}

void IRsendBase::OCR1B_ISR()
{   
    unsigned char TCCR2A_State;
    
    if (IR_SendParams.streamIndex == IR_SendParams.bitsToSend) 
    {
        IR_SendParams.streamIndex = 0;          // Back to 1st bit
        IR_SendParams.currentStep += 1;         // Next step
        if (IR_SendParams.currentStep > IR_SendParams.numSteps)
        {
            IR_SendParams.currentStep = 1;      // Back to step 1 (we always skip step 0)
            IR_SendParams.timesRepeated += 1;   // Increase the repetition count
        }

        // At each new step (streamIndex == bitsToSend), multi-step protocols need to have the 
        // sendStream filled in once more with the next step's values. 
        // If the protocol isn't multi-step, the function will do nothing. 
        FillSendStreamMultiStep();
    }
    
    if (IR_SendParams.timesRepeated == IR_SendParams.timesToRepeat)
    {
        // We're done
        IR_SEND_PWM_STOP;   // Turn off PWM
        stopSending();      // Turn off interrupt 
    }
    else
    {
        // Toggle the PWM - if it's on, we turn it off; if it's off, we turn it on
        TCCR2A_State = TCCR2A;
        (TCCR2A_State & _BV(COM2B1)) ? IR_SEND_PWM_STOP : IR_SEND_PWM_START;
        OCR1B = TCNT1 + IR_SendParams.sendStream[IR_SendParams.streamIndex++];  // Set the length of time and increment streamIndex 
    }   
}

void IRsendBase::startSending(void)
{
    // If we are already sending, ignore. The send will never occur, so you will have to try again later. 
    if (IR_SendParams.sending) return;
    
    // But if we are not already sending, proceed
    IR_SendParams.timesRepeated = 0;
    IR_SendParams.streamIndex = 0;
    IR_SendParams.sending = true;   // So we know not to start another send operation until this one is done
    enableIROut(IR_SendParams.kHz);
    
    // Make sure interrupts are off, setup the PWM and compare time, then clear any interrupts for good measure, 
    // and only then turn on the output compare interrupt. Otherwise you may trigger an interrupt in the setup
    // and the first bit will get skipped. 
    
    // Make sure interrupt is off
    TIMSK1 &= ~(1 << OCIE1B);       // OCIE1B bit of TIMSK1 = Output Compare Interrupt Enable 1 B. 
    
    // Turn on PWM
    IR_SEND_PWM_START;              
    
    // Set the compare time
    OCR1B = TCNT1 + IR_SendParams.sendStream[IR_SendParams.streamIndex++];  // Set the length of time of this bit, then increment to next bit

    // Clear any pending interrupts
    TIFR1 |= (1 << OCF1B);          // Output Compare Flag 1 B (clear by writing logic one)

    // Now turn the interrupt on that will occur when the timer reaches the Compare B value (OCR1B)
    TIMSK1 |= (1 << OCIE1B);        // OCIE1B bit of TIMSK1 = Output Compare Interrupt Enable 1 B. 
    
}

void IRsendBase::stopSending(void)
{
    // Clear this flag to disable compare interrupts
    TIMSK1 &= ~(1 << OCIE1B);       // OCIE1B bit of TIMSK1 = Output Compare Interrupt Enable 1 B. 
    IR_SendParams.sending = false;  // Flag to let us know the sending is done
}

boolean IRsendBase::isSendingDone(void)
{
    return !IR_SendParams.sending;
}

void IRsendBase::FillSendStreamMultiStep(void)
{
// This function assumes you already have specified IR_SendParams.currentStep and IR_SendParams.sendProtocol, so make sure you did.     
// This function is used for multi-step protocols. What are those? Presently there is only a single one, the Tamiya 1/35th IR protocol. 
// That one is 64 bits long plus a header bit, and of course after each bit we also have a space, so in total there are 130 pieces of information
// that have to be sent for one transmission. This is way more than we want to store in an array of 32-bit longs (the total array would take 
// 520 bytes of RAM!). So instead we break the transmission down into "steps." In the OCR1B_ISR() function that takes care of sending each next piece of
// information, when all the bits of sendStream have been sent, it checks if the protocol has additional steps. If so, it calls this function. This function
// in turn is tasked with filling in the next step's worth of information into sendStream[], which will then get sent out. OCR1B_ISR keeps sending one bit 
// at a time, one step at a time, and repeating the whole process for the timesToRepeat setting. In this way, we never need to store very much in 
// the sendStream array at any one time. 
    
    // 'j' will be the index to sendStream[]. It allows us to keep track of how many bits we are sending for this step. 
    uint8_t j = 0;  
    
    switch (IR_SendParams.sendProtocol)
    {
        case IR_TAMIYA_35: 
            if (IR_SendParams.currentStep == 1)
            {
                // We add the header bits if this is the first step - short mark followed by header space
                IR_SendParams.sendStream[0] = IR_uS_TO_TICKS((uint32_t)TAMIYA_135_SHORT_BIT); 
                IR_SendParams.sendStream[1] = IR_uS_TO_TICKS((uint32_t)TAMIYA_135_HDR_SPACE); 
                j = 2; // the next bit we add will be index 2 (0, 1 - next will be 2)
            }
            
            // Now get the value for this step from our stepValues array
            uint8_t value = IR_SendParams.stepValues[IR_SendParams.currentStep - 1];    // We let the user enter steps 1-8 for example, but actual array values will be 0-7, so subtract 1 from currentStep
            
            // Notice in each run through the loop we actually add two pieces to the stream - the mark, then the space. 
            // For this protocol, we know each step value will always be 8 bits. 
            for (uint8_t i=0; i<8; i++) 
            {
                // We look at the top bit. If 1, we add a one mark, otherwise a zero mark
                if (value & LEFTBIT)  
                {   // 1 - this is a long mark
                    IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)TAMIYA_135_LONG_BIT); 
                    // Long marks are always followed by short spaces
                    IR_SendParams.sendStream[j+1] = IR_uS_TO_TICKS((uint32_t)TAMIYA_135_SHORT_BIT);     
                } 
                else
                {   // 0 - this is a short mark
                    IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)TAMIYA_135_SHORT_BIT); 
                    // Short marks are always followed by long spaces
                    IR_SendParams.sendStream[j+1] = IR_uS_TO_TICKS((uint32_t)TAMIYA_135_LONG_BIT);  
                }
                // Add 2 to j because we added a mark and a space
                j +=2;
                // Now shift to the next bit
                value <<= 1;
            }
            
            // Specify how many bits there are to send. 
            IR_SendParams.bitsToSend = j;
            break;
        
        // Add other cases here in the future
        
    }
}


// ==========================================================================================================================>>
// IR SENDER - PROTOCOLS
// ==========================================================================================================================>>

void IRsendTamiya::send(void)
{
// The Tamiya signal is very simple - two marks separated by a space, followed by a longer gap between re-transmissions:
// 3000uS On, 3000 Off, 6000 On, 8000 Off
// Tamiya repeats the signal about 50 times, which takes one complete second. 
// This is overkill, and is what makes possible the notorious "fan shot" (where one tank hits
// multiple enemy tanks with one shot by panning the turret while firing). 
// We only repeat it 10 times (or whatever is set in OP_IRLibMatch.h)

    // Load the IR_SendParams struct with our signal data
    IR_SendParams.bitsToSend = Tamiya_BITS+1;           // Number of bits plus gap
    IR_SendParams.timesToRepeat = Tamiya_TIMESTOSEND;   // How many times to repeat
    IR_SendParams.kHz = 38;                             // 38 kHz
    IR_SendParams.currentStep = 1;                      // On step 1
    IR_SendParams.numSteps = 1;                         // This protocol only has 1 step
    IR_SendParams.sendProtocol = IR_TAMIYA;             // What protocol are we sending 
    for (int i=0; i<IR_SendParams.bitsToSend; i++)      // Array of bit lengths
    {
        IR_SendParams.sendStream[i] = IR_uS_TO_TICKS((uint32_t)pgm_read_word_near(&(Tamiya16Sig[i])));  // Convert bit lengths in uS -> to number of Timer 1 clock ticks
    }    
    startSending();                                     // Send it out
}
void IRsendTamiya_2Shot::send(void)
{
// The Tamiya 2-Shot signal will theoretically destroy any Tamiya tank in 2 shots regardless of the weight setting that tank has. 
// This signal is very similar to the regular Tamiya signal - two marks separated by a space, followed by a longer gap between re-transmissions:
// 4000uS On, 5000 Off, 3000 On, 8000 Off
// Tamiya repeats the signal about 50 times, which takes one complete second. 
// This is overkill, and is what makes possible the notorious "fan shot" (where one tank hits
// multiple enemy tanks with one shot by panning the turret while firing). 
// We only repeat it 10 times (or whatever is set in OP_IRLibMatch.h)

    // Load the IR_SendParams struct with our signal data
    IR_SendParams.bitsToSend = Tamiya_BITS+1;           // Number of bits plus gap
    IR_SendParams.timesToRepeat = Tamiya_TIMESTOSEND;   // How many times to repeat
    IR_SendParams.kHz = 38;                             // 38 kHz
    IR_SendParams.currentStep = 1;                      // On step 1
    IR_SendParams.numSteps = 1;                         // This protocol only has 1 step
    IR_SendParams.sendProtocol = IR_TAMIYA_2SHOT;       // What protocol are we sending     
    for (int i=0; i<IR_SendParams.bitsToSend; i++)      // Array of bit lengths
    {
        IR_SendParams.sendStream[i] = IR_uS_TO_TICKS((uint32_t)pgm_read_word_near(&(Tamiya16TwoShotSig[i])));  // Convert bit lengths in uS -> to number of Timer 1 clock ticks
    }    
    startSending();                                     // Send it out
}
void IRsendTamiya35::send(void)
{
// Any hope the 1/35th protocol would be similar to the 1/16th was dashed when I scoped it. This one is ridiculous and caused me some significant changes to the 
// entire sending class. 
// The protocol has two lengths which it uses for both marks and spaces. Short is always 500uS and long is always 1500uS. 
// The only exception is the header, which is a short mark (500uS) followed by a space of 3,000uS. 
// A "one" data bit is indicated by a long mark followed by a short space
// A "zero" data bit is indicated by a short mark followed by a long space
// Header: On 500uS, Off 3000uS
// Data: One  = On 1500uS, Off 500uS
//       Zero = On 500uS,  Off 1500us
// After the header, there are 64! bits of data. We presume these are meant to be 8 individual bytes of 8 bits each. 
// The total signal of header plus 64 bits is repeated for precisely 1 second. This doesn't allow an even number of transmissions, it actually gets repeated about 7.5 times
// There is no gap between re-transmissions other than we can look for the 3000uS space after the header mark to determine the beginning. 
// The decimal values for the 8 bytes are as follows: 199, 242, 192, 120, 135, 165, 183, 197
// We don't know what those mean or if those numbers are different for different models (we tested the #48212 Sherman). Clearly Tamiya could if they wanted
// send a great deal of information across with 64 bits of data. 
// Anyway - we don't want to store all 130 pieces of information in the sendStream array (64 bits + header bit * 2 because each is a mark and space) because that would
// take a ton of RAM. Instead I created the concept of "multi-step" protocols. So far this is the only one, all other protocols have 1 step. But in this case the protocol
// is given 8 steps for the 8 bytes. In the first byte we also include the header. Our sendStream array was set to a max of 26 elements which is more than enough for a single byte. 
// We created a new function called FillSendStreamMultiStep() that actually fills the sendStream array for whatever protocol is stored in IR_SendParams.sendProtocol for whatever
// step is stored in IR_SendParams.currentStep. If the protocol isn't a multi-step protocol, it will do nothing. FillSendStreamMultiStep() is called here to fill the first step, 
// and then by the OCR1B_ISR() routine whenever it gets to the next step as the data is being sent out. And even with multiple steps OCR1B_ISR() can still repeat the whole thing
// multiple time as specified by IR_SendParams.timesToRepeat. So you can use this process for future protocols that might be very long.
    // Load the IR_SendParams struct with our signal data
    IR_SendParams.timesToRepeat = TAMIYA_135_TIMESTOSEND;   // How many times to repeat. Tamiya gets out 7.5 repetitions in one second. We only send it 4 times and call that plenty. 
    IR_SendParams.kHz = 37;                                 // The 1/35 IR signal was measured at 37 kHz, though the standard 38kHz would also no doubt work. 
    IR_SendParams.currentStep = 1;                          // We are at step 1 of the Tamiya 1/35 IR signal
    IR_SendParams.numSteps = 8;                             // There will be 8 steps per transmission of the Tamiya 1/35 IR signal
    IR_SendParams.sendProtocol = IR_TAMIYA_35;              // What protocol are we sending
    // IR_SendParams.sendStream                             // Taken care of by the FillSendStreamMultiStep function below
    // IR_SendParams.bitsToSend                             // Taken care of by the FillSendStreamMultiStep function below
    
    // The Tamiya 1/35 IR code is so long we split it into 8 steps, each step has an 8-bit value that gets converted to 16 marks and spaces.
    // Here we save the 8 values for each step
    for (uint8_t i=0; i<TAMIYA_135_STEPS; i++)
    {
        IR_SendParams.stepValues[i] = pgm_read_byte_near(&(Tamiya135Cannon[i]));
    }

    // Now we construct the sendStream for the first step. We actually use a special function for this. 
    // This function will rely on IR_SendParams.currentStep and IR_SendParams.sendProtocol so make sure you've specified those already
    FillSendStreamMultiStep();      

    startSending();                                         // Send it out
}
void IRsendHengLong::send(void)
{
// The HengLong signal consists of 4 marks and 3 spaces. Both marks and spaces vary in length. 
    
    // Load the IR_SendParams struct with our signal data
    IR_SendParams.bitsToSend = HengLong_BITS+1;         // Number of bits plus gap
    IR_SendParams.timesToRepeat = HengLong_TIMESTOSEND; // How many times to repeat
    IR_SendParams.kHz = 38;                             // 38 kHz
    IR_SendParams.currentStep = 1;                      // On step 1
    IR_SendParams.numSteps = 1;                         // This protocol only has 1 step
    IR_SendParams.sendProtocol = IR_HENGLONG;           // What protocol are we sending 
    for (int i=0; i<IR_SendParams.bitsToSend; i++)      // Array of bit lengths
    {   // We also cast the progmem read to uint32_t (the data type for sendStream) because without it we have had some issues. 
        IR_SendParams.sendStream[i] = IR_uS_TO_TICKS((uint32_t)pgm_read_word_near(&(HengLongSig[i])));  // Convert bit lengths in uS -> to number of Timer 1 clock ticks
    }    
    startSending();                                     // Send it out
}
void IRsendTaigenV1::send(void)
{
// Unlike most other protocols, Taigen only sends their signal a single time. We however send it 6 times, which is what HengLong does. 
// The Taigen V1 signal is similar to HengLong's in that it consists of 4 marks and 3 spaces. However marks and spaces don't vary in length, 
// nor are they the same length as HengLong's. 

    // Load the IR_SendParams struct with our signal data
    IR_SendParams.bitsToSend = TaigenV1_BITS+1;         // Number of bits plus gap
    IR_SendParams.timesToRepeat = Taigen_TIMESTOSEND;   // How many times to repeat
    IR_SendParams.kHz = 39;                             // 39 kHz
    IR_SendParams.currentStep = 1;                      // On step 1
    IR_SendParams.numSteps = 1;                         // This protocol only has 1 step
    IR_SendParams.sendProtocol = IR_TAIGEN_V1;          // What protocol are we sending     
    for (int i=0; i<IR_SendParams.bitsToSend; i++)      // Array of bit lengths
    {
        IR_SendParams.sendStream[i] = IR_uS_TO_TICKS((uint32_t)pgm_read_word_near(&(TaigenSigV1[i])));  // Convert bit lengths in uS -> to number of Timer 1 clock ticks
    }    
    startSending();                                     // Send it out
}
void IRsendTaigen::send(void)
{
// This is used for Taigen V2 and v3 signals, which are similar to the V1 protocol but involves 9 marks instead of 4, and the timing is slightly different (shorter mark, longer space). 
// Unlike most other protocols, Taigen only sends their signal a single time. We however send it 6 times, which is what HengLong does. 

    // Load the IR_SendParams struct with our signal data
    IR_SendParams.bitsToSend = Taigen_BITS+1;           // Number of bits plus gap
    IR_SendParams.timesToRepeat = Taigen_TIMESTOSEND;   // How many times to repeat
    IR_SendParams.kHz = 39;                             // 39 kHz
    IR_SendParams.currentStep = 1;                      // On step 1
    IR_SendParams.numSteps = 1;                         // This protocol only has 1 step
    IR_SendParams.sendProtocol = IR_TAIGEN;             // What protocol are we sending     
    for (int i=0; i<IR_SendParams.bitsToSend; i++)      // Array of bit lengths
    {
        IR_SendParams.sendStream[i] = IR_uS_TO_TICKS((uint32_t)pgm_read_word_near(&(TaigenSig[i])));  // Convert bit lengths in uS -> to number of Timer 1 clock ticks
    }    
    startSending();                                     // Send it out
}
void IRsendFOV::send(uint32_t data)
{
// The FOV signal is very nicely constructed. Spaces are constant length, marks vary between short and long to represent 0 and 1. 
// After the header mark and space, there are 8 marks separated by spaces. These 8 marks make up one byte of numerical data. 
// By changing the data, FOV can specify different teams. 

    // How many data bits - in this case it will be 8
    uint8_t nbits = FOV_DATA_BITS;  

    // Load the IR_SendParams struct with our signal data
    IR_SendParams.bitsToSend = (nbits*2)+2;             // Databits * 2 to account for spaces, plus 1 to account for header mark, plus 1 for gap
    IR_SendParams.timesToRepeat = FOV_TIMESTOSEND;      // How many times to repeat
    IR_SendParams.kHz = 38;                             // 38 kHz
    IR_SendParams.currentStep = 1;                      // On step 1
    IR_SendParams.numSteps = 1;                         // This protocol only has 1 step
    IR_SendParams.sendProtocol = IR_FOV;                // What protocol are we sending         

    // Since the data we send out can vary, we can't hard-code it. Rather we have to convert an 8-bit number to marks and spaces
    IR_SendParams.sendStream[0] = IR_uS_TO_TICKS((uint32_t)FOV_HDR_MARK);   // First bit will always be header mark
    IR_SendParams.sendStream[1] = IR_uS_TO_TICKS((uint32_t)FOV_SPACE);      // Second bit will be a space

    // Now we shift through each data bit and send out the mark and space
    // Data is passed as a 32 bit unsigned integer. There are only 8 bits of 
    // data in the FOV signal. So we first shift out all the 24 leading zeros (32-8 = 24)
    // and just start looking at our actual data bits. 
    data = data << (32 - nbits);

    // Create a variable 'j' that will be the index to sendStream[]. We start it at 2 because we already added two pieces above (header mark and space)
    uint8_t j = 2;  
    // Notice in each run through the loop we actually add two pieces to the stream - the mark, then the space. 
    // nbits is the number of *data* bits, not the total number of bits, which will be greater.
    for (uint8_t i = 0; i < nbits; i++) 
    {
        // We look at the top bit. If 1, we add a one mark, otherwise a zero mark
        if (data & TOPBIT)  { IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)FOV_ONE_MARK);  } 
        else                { IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)FOV_ZERO_MARK); }
        j +=1;
        // After each mark we add a space. 
        IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)FOV_SPACE);
        // Now shift to the next bit and increment j
        data <<= 1;
        j += 1; 
    }
    // Go back one slot (to the last actual bit)
    j -= 1; 
    // Replace that last space with our GAP
    IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)FOV_GAP);
    
    startSending();                                     // Send it out
}
void IRsendVsTank::send(uint32_t data)
{
// At first glance, the VsTank signal appears to be constructed much like FOV. After a header mark there are 8 marks that appear to comprise 
// one byte of data. Yet on closer exampination, one sees that in this case the spaces actually determine the numerical value. Even more confusing, 
// the marks are not all the same length - but they do follow a consistent pattern: long marks always follow short spaces, and short marks 
// always follow long spaces. 
// So far as I know VsTank only has one code, so we could just hard-code the entire string of bits like we do with Tamiya or HengLong, 
// rather than sending/decoding an actual number. But since the protocol is clearly meant to accomodate an 8-bit number I've programmed this to 
// function that way, in case it comes to light that VsTank uses other numbers I don't know about (I only tested a single model). 

    // How many data bits - in this case it will be 8
    uint8_t nbits = VsTank_DATA_BITS;    

    // Load the IR_SendParams struct with our signal data
    IR_SendParams.bitsToSend = (nbits*2)+2;             // Databits * 2 to account for spaces, plus 1 to account for header mark, plus 1 for gap
    IR_SendParams.timesToRepeat = VsTank_TIMESTOSEND;   // How many times to repeat
    IR_SendParams.kHz = 34;                             // 34 kHz for VsTank, although 38kHz was also tested to work. 
    IR_SendParams.currentStep = 1;                      // On step 1
    IR_SendParams.numSteps = 1;                         // This protocol only has 1 step
    IR_SendParams.sendProtocol = IR_VSTANK;             // What protocol are we sending         

    // First bit will always be the header mark
    IR_SendParams.sendStream[0] = IR_uS_TO_TICKS((uint32_t)VsTank_HDR_MARK);        
    
    // Now we shift through each data bit and send out the marks and spaces. 
    // Data is passed as a 32 bit unsigned integer. There are only 8 bits of 
    // "data" though so we first shift out all the 24 leading zeros (32-8 = 24)
    // and just start looking at our actual data bits. 
    data = data << (32 - nbits);

    // Create a variable 'j' that will be the index to sendStream[]. We start it at 1 because we already added the header mark above
    uint8_t j = 1;  
    // Notice in each run through the loop we actually add two pieces to the stream - a space followed by a mark
    // nbits is the number of *data* bits, not the total number of bits, which will be greater.
    for (uint8_t i = 0; i < nbits; i++) 
    {
        // Space is set by the data (long or short depending on 1 or 0). The following mark is the opposite of the space. 
        if (data & TOPBIT) 
        {
            IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)VsTank_LONG_BIT);    // Long space (1)
            j += 1;
            IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)VsTank_SHORT_BIT);   // Short mark
        } 
        else 
        {
            IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)VsTank_SHORT_BIT);   // Short space (0)
            j += 1;
            IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)VsTank_LONG_BIT);    // Long mark
        }
        // Now shift to the next bit and increment j
        data <<= 1; 
        j += 1;
    }

    // Finally, add the Gap
    IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)VsTank_GAP);
    
    // Now send it out
    startSending();     
}
void IRsendOpenPanzer::send(void)
{
    // Placeholder for future expansion
}
void IRsendClark_Repair::send()
{
    // Clark uses the standard Sony 12-bit protocol for the repair signal. 
    // The repair code in binary is: 0000 0001 0000
    // That is equal to Hex 10 or decimal 16. 
    // Clark only sends the signal once, but given your vehicle is going to be immobilized for 15 seconds as soon as it shoots the repair code,
    // that doesn't seem like good practice (if you're going to be stuck, might as well send the signal repeately so you at least have a good 
    // chance of repairing the target). So we repeat the signal by the setting in OP_IRLibMatch.h
    IRsendSony sonySender;
    sonySender.send(Clark_REPAIR_CODE, Clark_REPAIR_TIMESTOSEND);       
}
void IRsendIBU_Repair::send(void)
{
// The IBU2 repair signal is a simple repeating sequence:
// 10,000uS On, 5,000 Off, 15,000 On, 10,000 Off
// This is repeated 49 times. 
// In fact there are actually 50 transmissions, and the first pair of marks is slightly different from the remaining 49: 
// 20,000uS On, 5,000 Off, 15,000 On, 10,000 Off
// But for simplicity we ignore the fact that the very first bit is slightly longer, and IBU2 doesn't care, the signal is still read. 
// In total this code takes ~1.2 seconds to send. That is really excessive, and we could shorten it by repeating it fewer times. 
// But since this is a repair signal, the tank won't be moving during the repair anyway so it won't matter. 

    // Load the IR_SendParams struct with our signal data
    IR_SendParams.bitsToSend = IBU2_BITS;               // Number of bits. Don't need to add an extra for gap here because we've built that in. 
    IR_SendParams.timesToRepeat = IBU2_TIMESTOSEND;     // How many times to repeat
    IR_SendParams.kHz = 38;                             // 38 kHz
    IR_SendParams.currentStep = 1;                      // On step 1
    IR_SendParams.numSteps = 1;                         // This protocol only has 1 step
    IR_SendParams.sendProtocol = IR_RPR_IBU;            // What protocol are we sending         
    for (int i=0; i<IR_SendParams.bitsToSend; i++)      // Array of bit lengths
    {
        IR_SendParams.sendStream[i] = IR_uS_TO_TICKS((uint32_t)pgm_read_word_near(&(IBU2RepairSig[i])));  // Convert bit lengths in uS -> to number of Timer 1 clock ticks
    }    
    startSending();                                     // Send it out
}
void IRsendRCTA_Repair::send()
{
// RC Tanks Australia repair signal is very simple: 4000 uS ON, 1500 OFF, 2000 ON, 2500 OFF, repeated 32 times
// 32 times seems excessive, but once you fire the repair shot the tank will be immobilized for 15 seconds, so it's not 
// like you have anything else to do. Since you are stuck anyway, might as well send the signal many times so you have a
// good chance of actually repairing the other vehicle. 
    
    // Load the IR_SendParams struct with our signal data
    IR_SendParams.bitsToSend = RCTA_BITS;                   // Don't add gap to this one, it is already included in RCTA_BITS
    IR_SendParams.timesToRepeat = RCTA_REPAIR_TIMESTOSEND;  // How many times to repeat
    IR_SendParams.kHz = 38;                                 // RCTA signal is standard 38 kHz
    IR_SendParams.currentStep = 1;                          // On step 1
    IR_SendParams.numSteps = 1;                             // This protocol only has 1 step
    IR_SendParams.sendProtocol = IR_RPR_RCTA;               // What protocol are we sending         
    for (int i=0; i<IR_SendParams.bitsToSend; i++)          // Array of bit lengths
    {
        IR_SendParams.sendStream[i] = IR_uS_TO_TICKS((uint32_t)pgm_read_word_near(&(RCTARepairSig[i])));  // Convert bit lengths in uS -> to number of Timer 1 clock ticks
    }    
    startSending();                                         // Send it out
}
void IRsendClark_MG::send()
{
// Clark uses the standard Sony 12-bit protocol for the machine gun signal. 
// The repair code in binary is: 0100000 10000
// In Sony-speak, that's Device ID 1 and Command 2
// You could send this code like so: 
// sendDeviceIDCommand(IR_SONY,1,2);    // Clark MG code

// But we can also just hardcode it, which we do here because it's a lot faster. 
// We just send the binary equivalent which is Hex 410 or decimal 1040. 
    
    // We only send the signal once, and let the OP_Tank class worry about calling it repeatedly while the machine gun is active
    IRsendSony sonySender;
    sonySender.send(Clark_MG_CODE, 1);      // 1 means send once
}
void IRsendRCTA_MG::send()
{
// RC Tanks Australia machine gun signal is very simple: 8000 uS ON, 6000 OFF, 2000 ON, 4000 OFF, repeated 20 times
// It is standard 38kHz

    // Load the IR_SendParams struct with our signal data
    IR_SendParams.bitsToSend = RCTA_BITS;                   // Don't add gap to this one, it is already included in RCTA_BITS
    IR_SendParams.timesToRepeat = RCTA_MG_TIMESTOSEND;      // We only send the signal once, and let the OP_Tank class worry about calling it repeatedly while the machine gun is active
    IR_SendParams.kHz = 38;                                 // RCTA signal is standard 38 kHz
    IR_SendParams.currentStep = 1;                          // On step 1
    IR_SendParams.numSteps = 1;                             // This protocol only has 1 step
    IR_SendParams.sendProtocol = IR_MG_RCTA;                // What protocol are we sending         
    for (int i=0; i<IR_SendParams.bitsToSend; i++)          // Array of bit lengths
    {
        IR_SendParams.sendStream[i] = IR_uS_TO_TICKS((uint32_t)pgm_read_word_near(&(RCTAMGSig[i])));  // Convert bit lengths in uS -> to number of Timer 1 clock ticks
    }    
    startSending();                                         // Send it out
} 
void IRsendSony::send(uint32_t data, uint8_t times_send)
{
// Sony uses a variable length mark and a fixed length space. The Sony protocol requires sending the command 
// at least three times, but since we are not interfacing with typical Sony devices, we allow any number. 
// Clark TK-xx devices use Sony codes for repair and machine gun signals. 
    
    // How many data bits - for now we only use the 12-bit protocol, but Sony also uses others up to 20, and you could actually set it to whatever you wanted. 
    uint8_t nbits = Sony_12_BIT;

    // Load the IR_SendParams struct with our signal data
    IR_SendParams.bitsToSend = (nbits*2)+2;     // Databits * 2 to account for spaces, plus 1 to account for header mark, plus 1 for gap
    IR_SendParams.timesToRepeat = times_send;   // How many times to repeat
    IR_SendParams.kHz = Sony_KHZ;               // kHz
    IR_SendParams.currentStep = 1;              // On step 1
    IR_SendParams.numSteps = 1;                 // This protocol only has 1 step
    IR_SendParams.sendProtocol = IR_SONY;       // What protocol are we sending         

    // Header mark and space
    IR_SendParams.sendStream[0] = IR_uS_TO_TICKS((uint32_t)Sony_HDR_MARK);  // First bit will always be header mark
    IR_SendParams.sendStream[1] = IR_uS_TO_TICKS((uint32_t)Sony_SPACE);     // Second bit will be a space
        
    // Now we shift through each data bit and send out the mark and space
    // Data is passed as a 32 bit unsigned integer, but the number of bits we actually send is 
    // less. So we first shift out all the leading zeros and just start looking at our actual data bits. 
    data = data << (32 - nbits);

    // Create a variable 'j' that will be the index to sendStream[]. We start it at 2 because we already added two pieces above (header mark and space)
    uint8_t j = 2;  
    // Notice in each run through the loop we actually add two pieces to the stream - the mark, then the space. 
    for (uint8_t i = 0; i < nbits; i++) 
    {
        // We look at the top bit. If 1, we add a one mark, otherwise a zero mark
        if (data & TOPBIT)  { IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)Sony_ONE_MARK);  } 
        else                { IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)Sony_ZERO_MARK); }
        j +=1;
        // After each mark we add a space. But instead of a last space, we add the gap
        if (j <= (nbits*2))     { IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)Sony_SPACE);   }
        else                    { IR_SendParams.sendStream[j] = IR_uS_TO_TICKS((uint32_t)Sony_GAP); }
        // Now shift to the next bit and increment j
        data <<= 1;
        j += 1; 
    }
    startSending();         // Send it out
};

void IRsendSony::sendDeviceIDCommand(uint8_t SonyDeviceID, uint8_t SonyCommand, uint8_t times_send)
{
// See: http://www.righto.com/2010/03/understanding-sony-ir-remote-codes-lirc.html

// The 12-bit Sony codes can be thought of as 12 bits however you want, but Sony breaks them down into a 7-bit Command followed by a 5-bit Device ID, 
// transmitted least-significant-bit first, which is backwards from how we typically read binary data. 
// There are times we might want to send a transmission without figuring out the 12 bits in advance, but rather just passing a function the 
// the DeviceID and Command numbers. That is what this function allows us to do. 
// (See a similar function in the IRdecode class that allows us to calculate the DeviceID and Command from a decoded Sony transmission)
    
    // The process is basically to break up the 12 bits into the two pieces, convert each piece to a full byte, then swapping all the bits in that byte using a lookup
    // table in progmem. As an example, take the following 12 bit code: 
    // 0100000 10000
    // This is the code Clark uses for machine gun. It could be represented by decimal 1040 (which is what we actually use when decoding Clark MG since that is a lot faster). 
    // But if we were to break this down according to the Sony specification, this would be device ID 1 (right-most 5 bits flipped), Command 2 (left most 7 bits flipped)
    // To get the Sony Device ID:
    // 1. 12-bit code ANDed with 001F: 0100000 10000 & 0000000 11111 = xxxxxxx 10000 : Basically, this converts all the Command bits to 0, and keeps all five Device ID bits the same
    // 2. Next we shift the result of the above left by three bits   = xxxx 10000000 : The reason is, we want our 5 bit DeviceID to take up a full 8 bits. 
    // 3. We save the result in a 8 bit unsigned integer             = 10000000      : So we get rid of the leading zeros
    // 4. Finally we swap all the bits                               = 00000001      : Our Sony Device ID is 1
    // To get the Sony Command: 
    // 1. 12-bit code ANDed with 0FE0: 0100000 10000 & 1111111 00000 = 0100000 xxxxx : This converts all the Device ID bits to 0, and keeps all seven Command bits the same
    // 2. Next shift the result above to the right by four bits:     = xxxx 01000000 : Now our 7-bit command takes up a full 8 bits
    // 3. Save the result in an 8 bit unsigned integer               = 01000000 
    // 4. Finally swap all the bits                                  = 00000010      : Our Sony Command is 2

uint32_t SonyData = 0;
uint8_t v; 

    // Make sure DeviceID is within range
    if (SonyDeviceID > MAX_SONY_DEVICE_ID) return;
    
    // Make sure Command is within range
    if (SonyCommand > MAX_SONY_COMMAND) return;
    
    v = ReverseByte(SonyCommand);   // Flip the Sony command bits around
    SonyData = v << 4;              // Now shift the command to the left to leave space for the Device ID. 
                                    // You might think we'd shift this left by 5 bits (that's how long the DeviceID is), 
                                    // but in fact we want to subtract 1 bit from our 8-bit "v" to get a 7-bit Command. 
                                    // We could shift right by 1 then left by 5, or just shift left by 4. 
    
    v = ReverseByte(SonyDeviceID);  // Flip the Sony Device ID bits around
    v >>= 3;                        // The Device ID is only 5 bits long, and "v" is 8, so shift right by 3
    
    SonyData |= v;                  // Now "OR" the Command and Device ID portions to get one piece of data. 

    send(SonyData, times_send);
}

void IRsendRaw::send(uint32_t buf[], unsigned char len, unsigned char khz)
{
// Pass an array and this will send it out a single time. 

    // Load the IR_SendParams struct with our signal data
    IR_SendParams.bitsToSend = len;             // Number of bits
    IR_SendParams.timesToRepeat = 1;            // Raw gets sent one time
    IR_SendParams.kHz = khz;                    // Set the frequency
    IR_SendParams.currentStep = 1;              // On step 1
    IR_SendParams.numSteps = 1;                 // This protocol only has 1 step
    IR_SendParams.sendProtocol = IR_UNKNOWN;    // What protocol are we sending         
    for (int i=0; i<IR_SendParams.bitsToSend; i++)
    {
        IR_SendParams.sendStream[i] = IR_uS_TO_TICKS(buf[i]);  // Convert bit lengths in uS -> to number of Timer 1 clock ticks
    }    
    startSending(); 
}
 

// The IRsend class allows us to send almost any protocol (other than raw) from a single class. 

// Many tank protocols don't actually need any data parameters, so as you will see below, even
// though this version of send takes a "data" argument, it is most often not used.
 void IRsend::send(IRTYPES Type, uint32_t data) {
  switch(Type) {
    case IR_TAMIYA:         IRsendTamiya::send();                   break;  
    case IR_TAMIYA_2SHOT:   IRsendTamiya_2Shot::send();             break;
    case IR_TAMIYA_35:      IRsendTamiya35::send();                 break;  
    case IR_HENGLONG:       IRsendHengLong::send();                 break;
    case IR_TAIGEN_V1:      IRsendTaigenV1::send();                 break;  // Taigen V1
    case IR_TAIGEN:         IRsendTaigen::send();                   break;  // Taigen V2/V3
    case IR_FOV:            IRsendFOV::send(data);                  break;  // data
    case IR_VSTANK:         IRsendVsTank::send(data);               break;  // data, although so far we know of only one valid data
    case IR_OPENPANZER:     IRsendOpenPanzer::send();               break;  // data, but this protocol is not written yet
    case IR_RPR_CLARK:      IRsendClark_Repair::send();             break;
    case IR_RPR_IBU:        IRsendIBU_Repair::send();               break;
    case IR_RPR_RCTA:       IRsendRCTA_Repair::send();              break;
    case IR_MG_CLARK:       IRsendClark_MG::send();                 break;
    case IR_MG_RCTA:        IRsendRCTA_MG::send();                  break;
    case IR_SONY:           IRsendSony::send(data);                 break;  // data
  }
}

// Here is a simpler version. Pass the type, it sends whatever the default data is. 
// This works for most tank protocols which have no variable data associated with the transmission. 
void IRsend::send(IRTYPES Type) {
  switch(Type) {
    case IR_TAMIYA:         IRsendTamiya::send();           break;  
    case IR_TAMIYA_2SHOT:   IRsendTamiya_2Shot::send();     break;
    case IR_TAMIYA_35:      IRsendTamiya35::send();         break;  
    case IR_HENGLONG:       IRsendHengLong::send();         break;
    case IR_TAIGEN_V1:      IRsendTaigenV1::send();         break;  // Taigen V1
    case IR_TAIGEN:         IRsendTaigen::send();           break;  // Taigen V2/V3
    case IR_FOV:            IRsendFOV::send();              break;  // Will default to Team 1
    case IR_VSTANK:         IRsendVsTank::send();           break;
//  case IR_OPENPANZER:     IRsendOpenPanzer::send();       break;  
    case IR_RPR_CLARK:      IRsendClark_Repair::send();     break;
    case IR_RPR_IBU:        IRsendIBU_Repair::send();       break;  
    case IR_RPR_RCTA:       IRsendRCTA_Repair::send();      break;
    case IR_MG_CLARK:       IRsendClark_MG::send();         break;
    case IR_MG_RCTA:        IRsendRCTA_MG::send();          break;    
  }
}


// This is an alternate version for sending 12-bit Sony commands (and 12-bit Sony-compatibles) 
// by passing the Device ID and Command rather than the entire data value. We could use this
// for example to pass a tank ID that the RCTA Mako2/ASP boards could read. 
// If an Open Panzer protocol is developed, it will likely use this format as well

// Yes, we could just create an IRsendSony object, but it might be convenient to call the IRsend 
// class instead, since we may not know at compile time which class we are ultimately going to use,
// and IRsend allows us to use any of them. 
void IRsend::sendDeviceIDCommand(IRTYPES Type, uint8_t DeviceID, uint8_t Command) 
{
    switch(Type) 
    {
        case IR_SONY:   IRsendSony::sendDeviceIDCommand(DeviceID, Command); break;
        // Add other 12-bit Sony-compatible protocols as they are created...
    }
}

 




// ==========================================================================================================================>>
// Various debugging routines
// ==========================================================================================================================>>
#ifdef OP_IRLiB_TRACE
void OP_IRLIB_ATTEMPT_MESSAGE(const __FlashStringHelper * s) {Serial.print(F("Attempting ")); Serial.print(s); Serial.println(F(" decode:"));};
void OP_IRLIB_TRACE_MESSAGE(const __FlashStringHelper * s) {Serial.print(F("Executing ")); Serial.println(s);};
byte OP_IRLIB_REJECTION_MESSAGE(const __FlashStringHelper * s) { Serial.print(F(" Protocol failed because ")); Serial.print(s); Serial.println(F(" wrong.")); return false;};
byte OP_IRLIB_DATA_ERROR_MESSAGE(const __FlashStringHelper * s, unsigned char index, uint16_t value, uint16_t expected) {  
 OP_IRLIB_REJECTION_MESSAGE(s); Serial.print(F("Error occurred with rawbuf[")); Serial.print(index,DEC); Serial.print(F("]=")); Serial.print(value,DEC);
 Serial.print(F(" expected:")); Serial.println(expected,DEC); return false;
};
#endif

