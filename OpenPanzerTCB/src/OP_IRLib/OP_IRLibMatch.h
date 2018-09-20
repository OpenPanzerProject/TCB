/* OP_IRLibMatch.h  Open Panzer Infra-red library - functions for encoding and decoding model RC tank infrared signals
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
 
#ifndef OP_IRLibMatch_h
#define OP_IRLibMatch_h

// pulse parameters are in uSec (micro-seconds). 1000 uSec = 1mS = 0.001 second
#define Tamiya_BITS         3       // 2 marks and 1 space
#define Tamiya_GAP          8000
#define Tamiya_TIMESTOSEND  10      // Tamiya repeats the signal 50 times which takes 1 second (overkill). We default to only 10 repetitions which takes 1/5 second. 
                                    // This reduces the effect of the notorious "fan" shot
const PROGMEM uint16_t Tamiya16Sig[Tamiya_BITS+1] = {3000, 3000, 6000, Tamiya_GAP}; // Add 1 to include gap
const PROGMEM uint16_t Tamiya16TwoShotSig[Tamiya_BITS+1] = {4000, 5000, 3000, Tamiya_GAP}; // Add 1 to include gap

#define TAMIYA_135_STEPS        8       // To send the signal we split it into 8 steps because it is so long
#define TAMIYA_135_BYTESTOCHECK 3       // When we decode the signal, we only bother checking this many bytes (there are 8 bytes total)
#define TAMIYA_135_HDR_SPACE    3000    // The header space is the only one that is this long and lets us determine the beginning of the transmission
#define TAMIYA_135_SHORT_BIT    500     // Every other piece of the protocol is either a short or long (both marks and spaces can be short or long)
#define TAMIYA_135_LONG_BIT     1500    // 
#define TAMIYA_135_TIMESTOSEND  4       // Tamiya repeats the 130 bit signal 8 times which takes 1 second. We only repeat it 4 times which takes ~1/2 second
// These are not times! The marks and spaces will be constructed. These are the 8 data bytes (8 bits each) that need to be sent out. 
const PROGMEM uint8_t Tamiya135Cannon[TAMIYA_135_STEPS] = {199, 242, 192, 120, 135, 165, 183, 197}; 

#define HengLong_HDR_MARK   19000   
#define HengLong_SHORT_BIT  4700
#define HengLong_LONG_BIT   9500
#define HengLong_GAP        32700   // Actual HengLong gap is 40,000 but our send routine can't go above ~32,700 without rolling over
#define HengLong_BITS       7       // 4 marks and 3 spaces
#define HengLong_TIMESTOSEND 6      // HengLong repeats the signal 6 times
// I never scoped the Heng Long signal directly from an RX-18, I am taking these parameters from Clark and Mako boards. 
const PROGMEM uint16_t HengLongSig[HengLong_BITS+1] = {19000, 4700, 9500, 4700, 4700, 9500, 4700, HengLong_GAP}; // Add 1 to include gap

// Taigen - common to all versions
#define Taigen_GAP          14000   // Since Taigen only sends the signal once, it has no gap. But since we send it multiple times, we make up a gap out of thin air. It needs to be longer than GAP. 
#define Taigen_TIMESTOSEND  1       // Taigen MFUs only send the signal once. If you try to repeat it, the Taigen will read it as sequential hits. So keep it at 1. 

// Taigen V1
#define TaigenV1_MARK       620     
#define TaigenV1_SPACE      570     
#define TaigenV1_BITS       7       // V1 is 4 marks and 3 spaces (7 bits)
const PROGMEM uint16_t TaigenSigV1[TaigenV1_BITS+1] = {TaigenV1_MARK, TaigenV1_SPACE, TaigenV1_MARK, TaigenV1_SPACE, TaigenV1_MARK, TaigenV1_SPACE, TaigenV1_MARK, Taigen_GAP}; // Add 1 to include gap

// Taigen V2, V3 
#define Taigen_MARK         600     // The Taigen has been measured at everything from 600/600 to 620/570 (V1/V2). It seems to respond to most anything in that range, 
#define Taigen_SPACE        620     // but the TCB receives hits better at 620/570. Receiving is difficult because Taigen only sends the signal a single time. 
#define Taigen_BITS         17      // V2/V3 are 9 marks and 8 spaces (17 bits)
const PROGMEM uint16_t TaigenSig[Taigen_BITS+1] = {Taigen_MARK, Taigen_SPACE, Taigen_MARK, Taigen_SPACE, Taigen_MARK, Taigen_SPACE, Taigen_MARK, Taigen_SPACE, Taigen_MARK, Taigen_SPACE, Taigen_MARK, Taigen_SPACE, Taigen_MARK, Taigen_SPACE, Taigen_MARK, Taigen_SPACE, Taigen_MARK, Taigen_GAP}; // Add 1 to include gap

#define FOV_HDR_MARK        8300
#define FOV_SPACE           1550
#define FOV_ONE_MARK        3100    
#define FOV_ZERO_MARK       1550    // Zero mark is the same length as the spaces
#define FOV_GAP             17550   // How much time between repeat transmissions
#define FOV_DATA_BITS       8       // 8 data bits
#define FOV_TIMESTOSEND     6       // FOV repeats their signal 6 times, total signal length is ~1/3 of a second. 
#define FOV_TEAM_1_VALUE    80      // 0x50
#define FOV_TEAM_2_VALUE    85      // 0x55
#define FOV_TEAM_3_VALUE    90      // 0x5A
#define FOV_TEAM_4_VALUE    95      // 0x5F

#define VsTank_HDR_MARK     6600
#define VsTank_SHORT_BIT    550
#define VsTank_LONG_BIT     1650
#define VsTank_GAP          32700   // Actual VsTank gap is ~91000 but our send routine can't go above ~32,700
#define VsTank_DATA_BITS    8       // 8 data bits
#define VsTank_TIMESTOSEND  5       // VSTank repeats their signal five times which takes about 1/8th of a second. 
#define VsTank_HIT_VALUE    91      // 0x5B

#define Clark_REPAIR_CODE   16
#define Clark_MG_CODE       1040
#define Clark_REPAIR_TIMESTOSEND 3  // Clark only sends the repair code once. We increase it to at least 3, but more wouldn't hurt. 
#define Clark_REPAIR_GAP    14000   // Sony actually waits 45mS between repeat transmissions, but we'll set the Repair gap to the same as the MG gap
#define Clark_MG_GAP        14000   // Time between subsequent machine gun codes, in uS

#define RCTA_BITS           4       // For RC Tanks Australia Machine Gun and Repair codes
#define RCTA_REPAIR_TIMESTOSEND 32  // RCTA sends the Repair code 32 times. It's excessive, but once you send the code you won't be moving anyway. 
#define RCTA_MG_TIMESTOSEND 3       // We let the OP_Tank class take care of repeating the machine gun signal, we only send it out once per call here
const PROGMEM uint16_t RCTARepairSig[RCTA_BITS] = {4000,1500,2000,2500};
const PROGMEM uint16_t RCTAMGSig[RCTA_BITS] = {8000,6000,2000,4000};

#define IBU2_BITS           4       // For Italian Battle Unit IBU2 - repair code
#define IBU2_TIMESTOSEND    50      // IBU2 sends the Repair code 50 times. It's excessive, but once you send the repair code you won't be moving anyway. 
const PROGMEM uint16_t IBU2RepairSig[IBU2_BITS] = {10000,5000,15000,10000};

#define MAX_SONY_DEVICE_ID  31      // Sony Device IDs are 5 bits long, meaning the max number is 31 (32 distinct integers counting 0)
#define MAX_SONY_COMMAND    127     // Sony Commands are 7 bits long, meaning the max number is 127 (128 distinct integers counting 0)
#define Sony_HDR_MARK       2400
#define Sony_SPACE          600
#define Sony_ONE_MARK       1200
#define Sony_ZERO_MARK      600
#define Sony_GAP            20000   // I don't actually know the official Sony gap between repeat transmissions. Clark uses 14mS to repeat MG. We set it to 20mS to be safe. 
#define Sony_KHZ            40      // Sony uses 40kHz, though 38 would undoubtedly work as well
#define Sony_12_BIT         12      // 12-bit Sony protocol (probably all we will be using)
#define Sony_15_BIT         15
#define Sony_20_BIT         20
#define Sony_TIMESTOSEND    3       // This is the Sony default


#define MG_REPEAT_TIME_mS   100     // How often to repeat Machine Gun IR signals, in *milli*seconds (not uSec). This is used by the OP_Tank class
                                    // to repeatedly fire the machine gun signal so long as the machine gun is active. This needs to be a number longer
                                    // than the longest single machine gun IR code. Clark's MG code (actualy a Sony code) is a bit under 20mS long for a single time, 
                                    // the RC Tanks Australia code is about 20mS long but we send it in bursts of 3 (60mS).  Setting it to 100mS gives us plenty of time 
                                    // and we still repeat the signal 10 times a second - not accurate in real terms but plenty often enough for an IR code. 
#define GAP 12000 // Time beyond this amount (in uS) will count as a gap between transmissions. This needs to be longer than the longest 
                  // data "space" of any protocol. Heng Long has the longest space of 10mS (10,000 uS), so we set this just a bit higher. 
                  // Unfortunately this is also longer than the valid gap separating repetitions of other protocols, such as Tamiya, RCTA, and IBU. 
                  // That means these signals when repeated will actually arrive as one big long stream instead of individual repetitions. But we can deal with that in code. 
                  // Note: there are "marks" that exceed 12000 uS - Heng Long has a mark 19mS long and IBU2 has one that is 15mS. That's ok, 
                  // the GAP length is only checked against spaces, not marks. 


// Ken Shirriff did some testing that found the length of a mark pulse is typically over reported and the length of a space underreported
// by the hardware internal to infrared receivers. The length of a received mark was found to be about 100µs too long and a space 100µs too short. 
// Chris Young, who developed the IRLib classes, felt 50µs was a better value. 
// In my own testing 50 seemed to work better than 100. The receivers are plenty accurate and especially with the short signals like Sony and Taigen, 
// once you start adding/subtracting even 100 uS to the pulses you are less likely to get a reading. 
// If you want to change the default, change the definition below. 
// If you want to change the value at run time, simply set the Mark_Excess value of the receiver object to whatever you want. 
#define MARK_EXCESS_DEFAULT 50

/*
 * Originally all timing comparisons for decoding were based on a percent of the
 * target value. However when target values are relatively large, the percent tolerance
 * is too much.  In some instances an absolute tolerance is needed. In order to maintain
 * backward compatibility, the default will be to continue to use percent. If you wish to default
 * to an absolute tolerance, you should comment out the line below.
 */
#define OP_IRLib_USE_PERCENT

/*
 * These are some miscellaneous definitions that are needed by the decoding routines. 
 */
#define PERCENT_TOLERANCE 18        // percent tolerance in measurements. Adjust as needed (if using percent)
#define DEFAULT_ABS_TOLERANCE 150   // absolute tolerance in microseconds. Adjust as needed (if using absolute)

/* 
 * These revised MATCH routines allow you to use either percentage or absolute tolerances.
 * Use ABS_MATCH for absolute and PERC_MATCH for percentages. The original MATCH macro
 * is controlled by the OP_IRLib_USE_PERCENT definition a few lines above.
 */
#define PERCENT_LOW(us) (unsigned int) (((us)*(1.0 - PERCENT_TOLERANCE/100.)))
#define PERCENT_HIGH(us) (unsigned int) (((us)*(1.0 + PERCENT_TOLERANCE/100.) + 1))

#define ABS_MATCH(v,e,t) ((v) >= ((e)-(t)) && (v) <= ((e)+(t)))
#define PERC_MATCH(v,e) ((v) >= PERCENT_LOW(e) && (v) <= PERCENT_HIGH(e))


#ifdef OP_IRLib_USE_PERCENT
#define MATCH(v,e) PERC_MATCH(v,e)
#else
#define MATCH(v,e) ABS_MATCH(v,e,DEFAULT_ABS_TOLERANCE)
#endif


#ifdef OP_IRLib_TRACE
void OP_IRLib_ATTEMPT_MESSAGE(const __FlashStringHelper * s);
void OP_IRLib_TRACE_MESSAGE(const __FlashStringHelper * s);
byte OP_IRLib_REJECTION_MESSAGE(const __FlashStringHelper * s);
byte OP_IRLib_DATA_ERROR_MESSAGE(const __FlashStringHelper * s, unsigned char index, unsigned int value, unsigned int expected);
#define RAW_COUNT_ERROR OP_IRLib_REJECTION_MESSAGE(F("number of raw samples"));
#define DATA_ERROR(data, expected) OP_IRLib_DATA_ERROR_MESSAGE(F("data error"),offset,data,expected);
#define HEADER_MARK_ERROR(expected) OP_IRLib_DATA_ERROR_MESSAGE(F("header mark"),offset,rawbuf[offset],expected);
#define HEADER_SPACE_ERROR(expected) OP_IRLib_DATA_ERROR_MESSAGE(F("header space"),offset,rawbuf[offset],expected);
#define DATA_MARK_ERROR(expected) OP_IRLib_DATA_ERROR_MESSAGE(F("data mark"),offset,rawbuf[offset],expected);
#define DATA_SPACE_ERROR(expected) OP_IRLib_DATA_ERROR_MESSAGE(F("data space"),offset,rawbuf[offset],expected);
#define TRAILER_BIT_ERROR(expected) OP_IRLib_DATA_ERROR_MESSAGE(F("RC5/RC6 trailer bit length"),offset,rawbuf[offset],expected);
#else
#define OP_IRLib_ATTEMPT_MESSAGE(s)
#define OP_IRLib_TRACE_MESSAGE(s)
#define OP_IRLib_REJECTION_MESSAGE(s) false
#define OP_IRLib_DATA_ERROR_MESSAGE(s,i,v,e) false
#define RAW_COUNT_ERROR false
#define DATA_ERROR(data, expected) false
#define HEADER_MARK_ERROR(expected) false
#define HEADER_SPACE_ERROR(expected) false
#define DATA_MARK_ERROR(expected) false
#define DATA_SPACE_ERROR(expected) false
#define TRAILER_BIT_ERROR(expected) false
#endif

#endif //OP_IRLibMatch_h
