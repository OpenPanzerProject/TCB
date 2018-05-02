 #ifndef OP_EEPROM_VARINFO_H
#define OP_EEPROM_VARINFO_H


//--------------------------------------------------------------------------------------------------------------------------------------->>
//=======================================================================================================================================>>
// VERY IMPORTANT ! 
//=======================================================================================================================================>>
// You must make sure this number equals the number of variables defined in the __eeprom_data struct (including the unused FirstVar)
// 
    #define NUM_STORED_VARS         319

// THIS NUMBER CAN BE CALCULATED BY THE EXCEL REFERENCE SHEET - AS CAN THE ENTIRE PROGMEM STATEMENT BELOW
// Don't bother trying to do it by hand!
//=======================================================================================================================================>>
//--------------------------------------------------------------------------------------------------------------------------------------->>

#include "../OP_Settings/OP_Settings.h"

// Data types that we are using. Currently we have no need for floats
typedef uint8_t _vartype; 
#define varNULL      0
#define varBOOL      1
#define varCHAR      2
#define varINT8      3
#define varUINT8     4
#define varINT16     5
#define varUINT16    6
#define varINT32     7
#define varUINT32    8
#define LAST_VARTYPE varUINT32


// Little function to help us print out helpful var type names, rather than numbers. 
// To use, call something like this:  Serial.print(printVarType(my_vartypes));
// Actual function defined in OP_EEPROM.cpp
const __FlashStringHelper *printVarType(_vartype Type);     


// Meta-data we want to know for each storage/eeprom variable
struct _storage_var_info {
    uint16_t varID;         // Arbitrary ID set by the software developer
    uint16_t varOffset;     // Offset of this variable within the _eeprom_data structure
    _vartype varType;       // Type of variable. This will let us determine bytes and sign. 
};


// A PROGMEM array of meta-data about every variable in the _eeprom_data struct
// The _storage_var_info struct has three members: ID, Offset, VarType
// This progmem statement can be generated automatically by the reference Excel sheet
// Note we put this in FAR - that means, beyond the first 64k of program memory(see OP_Settings.h)
const _storage_var_info STORAGEVARS[NUM_STORED_VARS] PROGMEM_FAR = {     
    {0, 0, varUINT8},        // FirstVar
    {1011, 1, varUINT8},        // ThrottleSettings.channelNum
    {1012, 2, varUINT16},        // ThrottleSettings.pulseMin
    {1013, 4, varUINT16},        // ThrottleSettings.pulseMax
    {1014, 6, varUINT16},        // ThrottleSettings.pulseCenter
    {1015, 8, varUINT8},        // ThrottleSettings.deadband
    {1016, 9, varBOOL},        // ThrottleSettings.reversed
    {1017, 10, varUINT8},        // TurnSettings.channelNum
    {1018, 11, varUINT16},        // TurnSettings.pulseMin
    {1019, 13, varUINT16},        // TurnSettings.pulseMax
    {1020, 15, varUINT16},        // TurnSettings.pulseCenter
    {1021, 17, varUINT8},        // TurnSettings.deadband
    {1022, 18, varBOOL},        // TurnSettings.reversed
    {1023, 19, varUINT8},        // ElevationSettings.channelNum
    {1024, 20, varUINT16},        // ElevationSettings.pulseMin
    {1025, 22, varUINT16},        // ElevationSettings.pulseMax
    {1026, 24, varUINT16},        // ElevationSettings.pulseCenter
    {1027, 26, varUINT8},        // ElevationSettings.deadband
    {1028, 27, varBOOL},        // ElevationSettings.reversed
    {1029, 28, varUINT8},        // AzimuthSettings.channelNum
    {1030, 29, varUINT16},        // AzimuthSettings.pulseMin
    {1031, 31, varUINT16},        // AzimuthSettings.pulseMax
    {1032, 33, varUINT16},        // AzimuthSettings.pulseCenter
    {1033, 35, varUINT8},        // AzimuthSettings.deadband
    {1034, 36, varBOOL},        // AzimuthSettings.reversed
    {1211, 37, varUINT8},        // Aux1_ChannelSettings.channelNum
    {1212, 38, varUINT16},        // Aux1_ChannelSettings.pulseMin
    {1213, 40, varUINT16},        // Aux1_ChannelSettings.pulseMax
    {1214, 42, varUINT16},        // Aux1_ChannelSettings.pulseCenter
    {1215, 44, varBOOL},        // Aux1_ChannelSettings.Digital
    {1216, 45, varUINT8},        // Aux1_ChannelSettings.NumPositions
    {1217, 46, varBOOL},        // Aux1_ChannelSettings.reversed
    {1218, 47, varUINT8},        // Aux2_ChannelSettings.channelNum
    {1219, 48, varUINT16},        // Aux2_ChannelSettings.pulseMin
    {1220, 50, varUINT16},        // Aux2_ChannelSettings.pulseMax
    {1221, 52, varUINT16},        // Aux2_ChannelSettings.pulseCenter
    {1222, 54, varBOOL},        // Aux2_ChannelSettings.Digital
    {1223, 55, varUINT8},        // Aux2_ChannelSettings.NumPositions
    {1224, 56, varBOOL},        // Aux2_ChannelSettings.reversed
    {1225, 57, varUINT8},        // Aux3_ChannelSettings.channelNum
    {1226, 58, varUINT16},        // Aux3_ChannelSettings.pulseMin
    {1227, 60, varUINT16},        // Aux3_ChannelSettings.pulseMax
    {1228, 62, varUINT16},        // Aux3_ChannelSettings.pulseCenter
    {1229, 64, varBOOL},        // Aux3_ChannelSettings.Digital
    {1230, 65, varUINT8},        // Aux3_ChannelSettings.NumPositions
    {1231, 66, varBOOL},        // Aux3_ChannelSettings.reversed
    {1232, 67, varUINT8},        // Aux4_ChannelSettings.channelNum
    {1233, 68, varUINT16},        // Aux4_ChannelSettings.pulseMin
    {1234, 70, varUINT16},        // Aux4_ChannelSettings.pulseMax
    {1235, 72, varUINT16},        // Aux4_ChannelSettings.pulseCenter
    {1236, 74, varBOOL},        // Aux4_ChannelSettings.Digital
    {1237, 75, varUINT8},        // Aux4_ChannelSettings.NumPositions
    {1238, 76, varBOOL},        // Aux4_ChannelSettings.reversed
    {1239, 77, varUINT8},        // Aux5_ChannelSettings.channelNum
    {1240, 78, varUINT16},        // Aux5_ChannelSettings.pulseMin
    {1241, 80, varUINT16},        // Aux5_ChannelSettings.pulseMax
    {1242, 82, varUINT16},        // Aux5_ChannelSettings.pulseCenter
    {1243, 84, varBOOL},        // Aux5_ChannelSettings.Digital
    {1244, 85, varUINT8},        // Aux5_ChannelSettings.NumPositions
    {1245, 86, varBOOL},        // Aux5_ChannelSettings.reversed
    {1246, 87, varUINT8},        // Aux6_ChannelSettings.channelNum
    {1247, 88, varUINT16},        // Aux6_ChannelSettings.pulseMin
    {1248, 90, varUINT16},        // Aux6_ChannelSettings.pulseMax
    {1249, 92, varUINT16},        // Aux6_ChannelSettings.pulseCenter
    {1250, 94, varBOOL},        // Aux6_ChannelSettings.Digital
    {1251, 95, varUINT8},        // Aux6_ChannelSettings.NumPositions
    {1252, 96, varBOOL},        // Aux6_ChannelSettings.reversed
    {1253, 97, varUINT8},        // Aux7_ChannelSettings.channelNum
    {1254, 98, varUINT16},        // Aux7_ChannelSettings.pulseMin
    {1255, 100, varUINT16},        // Aux7_ChannelSettings.pulseMax
    {1256, 102, varUINT16},        // Aux7_ChannelSettings.pulseCenter
    {1257, 104, varBOOL},        // Aux7_ChannelSettings.Digital
    {1258, 105, varUINT8},        // Aux7_ChannelSettings.NumPositions
    {1259, 106, varBOOL},        // Aux7_ChannelSettings.reversed
    {1260, 107, varUINT8},        // Aux8_ChannelSettings.channelNum
    {1261, 108, varUINT16},        // Aux8_ChannelSettings.pulseMin
    {1262, 110, varUINT16},        // Aux8_ChannelSettings.pulseMax
    {1263, 112, varUINT16},        // Aux8_ChannelSettings.pulseCenter
    {1264, 114, varBOOL},        // Aux8_ChannelSettings.Digital
    {1265, 115, varUINT8},        // Aux8_ChannelSettings.NumPositions
    {1266, 116, varBOOL},        // Aux8_ChannelSettings.reversed
    {1267, 117, varUINT8},        // Aux9_ChannelSettings.channelNum
    {1268, 118, varUINT16},        // Aux9_ChannelSettings.pulseMin
    {1269, 120, varUINT16},        // Aux9_ChannelSettings.pulseMax
    {1270, 122, varUINT16},        // Aux9_ChannelSettings.pulseCenter
    {1271, 124, varBOOL},        // Aux9_ChannelSettings.Digital
    {1272, 125, varUINT8},        // Aux9_ChannelSettings.NumPositions
    {1273, 126, varBOOL},        // Aux9_ChannelSettings.reversed
    {1274, 127, varUINT8},        // Aux10_ChannelSettings.channelNum
    {1275, 128, varUINT16},        // Aux10_ChannelSettings.pulseMin
    {1276, 130, varUINT16},        // Aux10_ChannelSettings.pulseMax
    {1277, 132, varUINT16},        // Aux10_ChannelSettings.pulseCenter
    {1278, 134, varBOOL},        // Aux10_ChannelSettings.Digital
    {1279, 135, varUINT8},        // Aux10_ChannelSettings.NumPositions
    {1280, 136, varBOOL},        // Aux10_ChannelSettings.reversed
    {1281, 137, varUINT8},        // Aux11_ChannelSettings.channelNum
    {1282, 138, varUINT16},        // Aux11_ChannelSettings.pulseMin
    {1283, 140, varUINT16},        // Aux11_ChannelSettings.pulseMax
    {1284, 142, varUINT16},        // Aux11_ChannelSettings.pulseCenter
    {1285, 144, varBOOL},        // Aux11_ChannelSettings.Digital
    {1286, 145, varUINT8},        // Aux11_ChannelSettings.NumPositions
    {1287, 146, varBOOL},        // Aux11_ChannelSettings.reversed
    {1288, 147, varUINT8},        // Aux12_ChannelSettings.channelNum
    {1289, 148, varUINT16},        // Aux12_ChannelSettings.pulseMin
    {1290, 150, varUINT16},        // Aux12_ChannelSettings.pulseMax
    {1291, 152, varUINT16},        // Aux12_ChannelSettings.pulseCenter
    {1292, 154, varBOOL},        // Aux12_ChannelSettings.Digital
    {1293, 155, varUINT8},        // Aux12_ChannelSettings.NumPositions
    {1294, 156, varBOOL},        // Aux12_ChannelSettings.reversed
    {1311, 157, varUINT8},        // PortA_Settings.dataDirection
    {1312, 158, varUINT8},        // PortA_Settings.dataType
    {1313, 159, varUINT8},        // PortB_Settings.dataDirection
    {1314, 160, varUINT8},        // PortB_Settings.dataType
    {1411, 161, varUINT16},        // SF_Trigger[0].TriggerID
    {1412, 163, varUINT8},        // SF_Trigger[0].specialFunction
    {1413, 164, varUINT16},        // SF_Trigger[1].TriggerID
    {1414, 166, varUINT8},        // SF_Trigger[1].specialFunction
    {1415, 167, varUINT16},        // SF_Trigger[2].TriggerID
    {1416, 169, varUINT8},        // SF_Trigger[2].specialFunction
    {1417, 170, varUINT16},        // SF_Trigger[3].TriggerID
    {1418, 172, varUINT8},        // SF_Trigger[3].specialFunction
    {1419, 173, varUINT16},        // SF_Trigger[4].TriggerID
    {1420, 175, varUINT8},        // SF_Trigger[4].specialFunction
    {1421, 176, varUINT16},        // SF_Trigger[5].TriggerID
    {1422, 178, varUINT8},        // SF_Trigger[5].specialFunction
    {1423, 179, varUINT16},        // SF_Trigger[6].TriggerID
    {1424, 181, varUINT8},        // SF_Trigger[6].specialFunction
    {1425, 182, varUINT16},        // SF_Trigger[7].TriggerID
    {1426, 184, varUINT8},        // SF_Trigger[7].specialFunction
    {1427, 185, varUINT16},        // SF_Trigger[8].TriggerID
    {1428, 187, varUINT8},        // SF_Trigger[8].specialFunction
    {1429, 188, varUINT16},        // SF_Trigger[9].TriggerID
    {1430, 190, varUINT8},        // SF_Trigger[9].specialFunction
    {1431, 191, varUINT16},        // SF_Trigger[10].TriggerID
    {1432, 193, varUINT8},        // SF_Trigger[10].specialFunction
    {1433, 194, varUINT16},        // SF_Trigger[11].TriggerID
    {1434, 196, varUINT8},        // SF_Trigger[11].specialFunction
    {1435, 197, varUINT16},        // SF_Trigger[12].TriggerID
    {1436, 199, varUINT8},        // SF_Trigger[12].specialFunction
    {1437, 200, varUINT16},        // SF_Trigger[13].TriggerID
    {1438, 202, varUINT8},        // SF_Trigger[13].specialFunction
    {1439, 203, varUINT16},        // SF_Trigger[14].TriggerID
    {1440, 205, varUINT8},        // SF_Trigger[14].specialFunction
    {1441, 206, varUINT16},        // SF_Trigger[15].TriggerID
    {1442, 208, varUINT8},        // SF_Trigger[15].specialFunction
    {1443, 209, varUINT16},        // SF_Trigger[16].TriggerID
    {1444, 211, varUINT8},        // SF_Trigger[16].specialFunction
    {1445, 212, varUINT16},        // SF_Trigger[17].TriggerID
    {1446, 214, varUINT8},        // SF_Trigger[17].specialFunction
    {1447, 215, varUINT16},        // SF_Trigger[18].TriggerID
    {1448, 217, varUINT8},        // SF_Trigger[18].specialFunction
    {1449, 218, varUINT16},        // SF_Trigger[19].TriggerID
    {1450, 220, varUINT8},        // SF_Trigger[19].specialFunction
    {1451, 221, varUINT16},        // SF_Trigger[20].TriggerID
    {1452, 223, varUINT8},        // SF_Trigger[20].specialFunction
    {1453, 224, varUINT16},        // SF_Trigger[21].TriggerID
    {1454, 226, varUINT8},        // SF_Trigger[21].specialFunction
    {1455, 227, varUINT16},        // SF_Trigger[22].TriggerID
    {1456, 229, varUINT8},        // SF_Trigger[22].specialFunction
    {1457, 230, varUINT16},        // SF_Trigger[23].TriggerID
    {1458, 232, varUINT8},        // SF_Trigger[23].specialFunction
    {1459, 233, varUINT16},        // SF_Trigger[24].TriggerID
    {1460, 235, varUINT8},        // SF_Trigger[24].specialFunction
    {1461, 236, varUINT16},        // SF_Trigger[25].TriggerID
    {1462, 238, varUINT8},        // SF_Trigger[25].specialFunction
    {1463, 239, varUINT16},        // SF_Trigger[26].TriggerID
    {1464, 241, varUINT8},        // SF_Trigger[26].specialFunction
    {1465, 242, varUINT16},        // SF_Trigger[27].TriggerID
    {1466, 244, varUINT8},        // SF_Trigger[27].specialFunction
    {1467, 245, varUINT16},        // SF_Trigger[28].TriggerID
    {1468, 247, varUINT8},        // SF_Trigger[28].specialFunction
    {1469, 248, varUINT16},        // SF_Trigger[29].TriggerID
    {1470, 250, varUINT8},        // SF_Trigger[29].specialFunction
    {1471, 251, varUINT16},        // SF_Trigger[30].TriggerID
    {1472, 253, varUINT8},        // SF_Trigger[30].specialFunction
    {1473, 254, varUINT16},        // SF_Trigger[31].TriggerID
    {1474, 256, varUINT8},        // SF_Trigger[31].specialFunction
    {1475, 257, varUINT16},        // SF_Trigger[32].TriggerID
    {1476, 259, varUINT8},        // SF_Trigger[32].specialFunction
    {1477, 260, varUINT16},        // SF_Trigger[33].TriggerID
    {1478, 262, varUINT8},        // SF_Trigger[33].specialFunction
    {1479, 263, varUINT16},        // SF_Trigger[34].TriggerID
    {1480, 265, varUINT8},        // SF_Trigger[34].specialFunction
    {1481, 266, varUINT16},        // SF_Trigger[35].TriggerID
    {1482, 268, varUINT8},        // SF_Trigger[35].specialFunction
    {1483, 269, varUINT16},        // SF_Trigger[36].TriggerID
    {1484, 271, varUINT8},        // SF_Trigger[36].specialFunction
    {1485, 272, varUINT16},        // SF_Trigger[37].TriggerID
    {1486, 274, varUINT8},        // SF_Trigger[37].specialFunction
    {1487, 275, varUINT16},        // SF_Trigger[38].TriggerID
    {1488, 277, varUINT8},        // SF_Trigger[38].specialFunction
    {1489, 278, varUINT16},        // SF_Trigger[39].TriggerID
    {1490, 280, varUINT8},        // SF_Trigger[39].specialFunction
    {1611, 281, varUINT8},        // DriveMotors
    {1612, 282, varUINT8},        // TurretRotationMotor
    {1613, 283, varUINT8},        // TurretElevationMotor
    {1811, 284, varINT16},        // TurretElevation_EPMin
    {1812, 286, varINT16},        // TurretElevation_EPMax
    {1813, 288, varBOOL},        // TurretElevation_Reversed
    {1814, 289, varUINT8},        // TurretElevation_MaxSpeedPct
    {1815, 290, varUINT8},        // TurretRotation_MaxSpeedPct
    {1816, 291, varINT16},        // TurretRotation_EPMin
    {1817, 293, varINT16},        // TurretRotation_EPMax
    {1818, 295, varBOOL},        // TurretRotation_Reversed
    {1911, 296, varINT16},        // SteeringServo_EPMin
    {1912, 298, varINT16},        // SteeringServo_EPMax
    {1913, 300, varBOOL},        // SteeringServo_Reversed
    {2011, 301, varBOOL},        // Airsoft
    {2012, 302, varBOOL},        // MechanicalBarrelWithCannon
    {2013, 303, varINT16},        // RecoilDelay
    {2014, 305, varBOOL},        // RecoilReversed
    {2015, 306, varBOOL},        // ServoRecoilWithCannon
    {2016, 307, varINT16},        // RecoilServo_Recoil_mS
    {2017, 309, varINT16},        // RecoilServo_Return_mS
    {2018, 311, varINT16},        // RecoilServo_EPMin
    {2019, 313, varINT16},        // RecoilServo_EPMax
    {2211, 315, varBOOL},        // SmokerControlAuto
    {2212, 316, varINT16},        // SmokerIdleSpeed
    {2213, 318, varINT16},        // SmokerFastIdleSpeed
    {2214, 320, varINT16},        // SmokerMaxSpeed
    {2215, 322, varINT16},        // SmokerDestroyedSpeed
    {2411, 324, varBOOL},        // AccelRampEnabled_1
    {2412, 325, varUINT8},        // AccelSkipNum_1
    {2413, 326, varUINT8},        // AccelPreset_1
    {2414, 327, varBOOL},        // DecelRampEnabled_1
    {2415, 328, varUINT8},        // DecelSkipNum_1
    {2416, 329, varUINT8},        // DecelPreset_1
    {2417, 330, varBOOL},        // AccelRampEnabled_2
    {2418, 331, varUINT8},        // AccelSkipNum_2
    {2419, 332, varUINT8},        // AccelPreset_2
    {2420, 333, varBOOL},        // DecelRampEnabled_2
    {2421, 334, varUINT8},        // DecelSkipNum_2
    {2422, 335, varUINT8},        // DecelPreset_2
    {2423, 336, varUINT8},        // BrakeSensitivityPct
    {2424, 337, varUINT16},        // TimeToShift_mS
    {2425, 339, varUINT16},        // EnginePauseTime_mS
    {2426, 341, varUINT16},        // TransmissionDelay_mS
    {2427, 343, varBOOL},        // NeutralTurnAllowed
    {2428, 344, varUINT8},        // NeutralTurnPct
    {2429, 345, varUINT8},        // TurnMode
    {2430, 346, varUINT8},        // DriveType
    {2431, 347, varUINT8},        // MaxForwardSpeedPct
    {2432, 348, varUINT8},        // MaxReverseSpeedPct
    {2433, 349, varUINT8},        // HalftrackTreadTurnPct
    {2434, 350, varBOOL},        // EngineAutoStart
    {2435, 351, varINT32},        // EngineAutoStopTime_mS
    {2436, 355, varUINT8},        // MotorNudgePct
    {2437, 356, varUINT16},        // NudgeTime_mS
    {2438, 358, varBOOL},        // DragInnerTrack
    {2439, 359, varBOOL},        // EnableTrackRecoil
    {2440, 360, varUINT8},        // TrackRecoilKickbackSpeed
    {2441, 361, varUINT8},        // TrackRecoilDecelerateFactor
    {2511, 362, varBOOL},        // EnableBarrelStabilize
    {2512, 363, varUINT8},        // BarrelSensitivity
    {2513, 364, varBOOL},        // EnableHillPhysics
    {2514, 365, varUINT8},        // HillSensitivity
    {2711, 366, varINT16},        // IgnoreTurretDelay_mS
    {2811, 368, varUINT8},        // SoundDevice
    {2812, 369, varUINT16},        // Squeak1_MinInterval_mS
    {2813, 371, varUINT16},        // Squeak1_MaxInterval_mS
    {2814, 373, varUINT16},        // Squeak2_MinInterval_mS
    {2815, 375, varUINT16},        // Squeak2_MaxInterval_mS
    {2816, 377, varUINT16},        // Squeak3_MinInterval_mS
    {2817, 379, varUINT16},        // Squeak3_MaxInterval_mS
    {2818, 381, varBOOL},        // Squeak1_Enabled
    {2819, 382, varBOOL},        // Squeak2_Enabled
    {2820, 383, varBOOL},        // Squeak3_Enabled
    {2821, 384, varUINT8},        // MinSqueakSpeed
    {2822, 385, varBOOL},        // HeadlightSound_Enabled
    {2823, 386, varBOOL},        // TurretSound_Enabled
    {2824, 387, varBOOL},        // BarrelSound_Enabled
    {2825, 388, varUINT16},        // Squeak4_MinInterval_mS
    {2826, 390, varUINT16},        // Squeak4_MaxInterval_mS
    {2827, 392, varUINT16},        // Squeak5_MinInterval_mS
    {2828, 394, varUINT16},        // Squeak5_MaxInterval_mS
    {2829, 396, varUINT16},        // Squeak6_MinInterval_mS
    {2830, 398, varUINT16},        // Squeak6_MaxInterval_mS
    {2831, 400, varBOOL},        // Squeak4_Enabled
    {2832, 401, varBOOL},        // Squeak5_Enabled
    {2833, 402, varBOOL},        // Squeak6_Enabled
    {2834, 403, varUINT8},        // VolumeEngine
    {2835, 404, varUINT8},        // VolumeTrackOverlay
    {2836, 405, varUINT8},        // VolumeEffects
    {2837, 406, varBOOL},        // HeadlightSound2_Enabled
    {3011, 407, varUINT8},        // IR_FireProtocol
    {3012, 408, varUINT8},        // IR_HitProtocol_2
    {3013, 409, varUINT8},        // IR_RepairProtocol
    {3014, 410, varUINT8},        // IR_MGProtocol
    {3015, 411, varBOOL},        // Use_MG_Protocol
    {3016, 412, varBOOL},        // Accept_MG_Damage
    {3017, 413, varUINT8},        // DamageProfile
    {3018, 414, varUINT16},        // CustomClassSettings.reloadTime
    {3019, 416, varUINT16},        // CustomClassSettings.recoveryTime
    {3020, 418, varUINT8},        // CustomClassSettings.maxHits
    {3021, 419, varUINT8},        // CustomClassSettings.maxMGHits
    {3022, 420, varBOOL},        // SendTankID
    {3023, 421, varUINT16},        // TankID
    {3024, 423, varUINT8},        // IR_Team
    {3211, 424, varUINT32},        // USBSerialBaud
    {3212, 428, varUINT32},        // AuxSerialBaud
    {3213, 432, varUINT32},        // MotorSerialBaud
    {3214, 436, varUINT32},        // Serial3TxBaud
    {3215, 440, varBOOL},        // LVC_Enabled
    {3216, 441, varUINT16},        // LVC_Cutoff_mV
    {3411, 443, varBOOL},        // RunningLightsAlwaysOn
    {3412, 444, varUINT8},        // RunningLightsDimLevelPct
    {3413, 445, varBOOL},        // BrakesAutoOnAtStop
    {3414, 446, varUINT16},        // AuxLightFlashTime_mS
    {3415, 448, varUINT16},        // AuxLightBlinkOnTime_mS
    {3416, 450, varUINT16},        // AuxLightBlinkOffTime_mS
    {3417, 452, varUINT8},        // AuxLightPresetDim
    {3418, 453, varUINT8},        // MGLightBlink_mS
    {3419, 454, varBOOL},        // FlashLightsWhenSignalLost
    {3420, 455, varBOOL},        // HiFlashWithCannon
    {3421, 456, varBOOL},        // AuxFlashWithCannon
    {3422, 457, varUINT8},        // SecondMGLightBlink_mS
    {3423, 458, varBOOL},        // CannonReloadBlink
    {9011, 459, varBOOL},        // PrintDebug
    {9999, 460, varUINT32}        // InitStamp
};


#endif  // Define OP_EEPROM_VARINFO_H