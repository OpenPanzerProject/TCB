 #ifndef OP_EEPROM_VARINFO_H
#define OP_EEPROM_VARINFO_H


//--------------------------------------------------------------------------------------------------------------------------------------->>
//=======================================================================================================================================>>
// VERY IMPORTANT ! 
//=======================================================================================================================================>>
// You must make sure this number equals the number of variables defined in the __eeprom_data struct (including the unused FirstVar)
// 
    #define NUM_STORED_VARS         331

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
    {1315, 161, varUINT16},        // IOBlinkOnTime_mS
    {1316, 163, varUINT16},        // IOBlinkOffTime_mS
    {1411, 165, varUINT16},        // SF_Trigger[0].TriggerID
    {1412, 167, varUINT8},        // SF_Trigger[0].specialFunction
    {1413, 168, varUINT16},        // SF_Trigger[1].TriggerID
    {1414, 170, varUINT8},        // SF_Trigger[1].specialFunction
    {1415, 171, varUINT16},        // SF_Trigger[2].TriggerID
    {1416, 173, varUINT8},        // SF_Trigger[2].specialFunction
    {1417, 174, varUINT16},        // SF_Trigger[3].TriggerID
    {1418, 176, varUINT8},        // SF_Trigger[3].specialFunction
    {1419, 177, varUINT16},        // SF_Trigger[4].TriggerID
    {1420, 179, varUINT8},        // SF_Trigger[4].specialFunction
    {1421, 180, varUINT16},        // SF_Trigger[5].TriggerID
    {1422, 182, varUINT8},        // SF_Trigger[5].specialFunction
    {1423, 183, varUINT16},        // SF_Trigger[6].TriggerID
    {1424, 185, varUINT8},        // SF_Trigger[6].specialFunction
    {1425, 186, varUINT16},        // SF_Trigger[7].TriggerID
    {1426, 188, varUINT8},        // SF_Trigger[7].specialFunction
    {1427, 189, varUINT16},        // SF_Trigger[8].TriggerID
    {1428, 191, varUINT8},        // SF_Trigger[8].specialFunction
    {1429, 192, varUINT16},        // SF_Trigger[9].TriggerID
    {1430, 194, varUINT8},        // SF_Trigger[9].specialFunction
    {1431, 195, varUINT16},        // SF_Trigger[10].TriggerID
    {1432, 197, varUINT8},        // SF_Trigger[10].specialFunction
    {1433, 198, varUINT16},        // SF_Trigger[11].TriggerID
    {1434, 200, varUINT8},        // SF_Trigger[11].specialFunction
    {1435, 201, varUINT16},        // SF_Trigger[12].TriggerID
    {1436, 203, varUINT8},        // SF_Trigger[12].specialFunction
    {1437, 204, varUINT16},        // SF_Trigger[13].TriggerID
    {1438, 206, varUINT8},        // SF_Trigger[13].specialFunction
    {1439, 207, varUINT16},        // SF_Trigger[14].TriggerID
    {1440, 209, varUINT8},        // SF_Trigger[14].specialFunction
    {1441, 210, varUINT16},        // SF_Trigger[15].TriggerID
    {1442, 212, varUINT8},        // SF_Trigger[15].specialFunction
    {1443, 213, varUINT16},        // SF_Trigger[16].TriggerID
    {1444, 215, varUINT8},        // SF_Trigger[16].specialFunction
    {1445, 216, varUINT16},        // SF_Trigger[17].TriggerID
    {1446, 218, varUINT8},        // SF_Trigger[17].specialFunction
    {1447, 219, varUINT16},        // SF_Trigger[18].TriggerID
    {1448, 221, varUINT8},        // SF_Trigger[18].specialFunction
    {1449, 222, varUINT16},        // SF_Trigger[19].TriggerID
    {1450, 224, varUINT8},        // SF_Trigger[19].specialFunction
    {1451, 225, varUINT16},        // SF_Trigger[20].TriggerID
    {1452, 227, varUINT8},        // SF_Trigger[20].specialFunction
    {1453, 228, varUINT16},        // SF_Trigger[21].TriggerID
    {1454, 230, varUINT8},        // SF_Trigger[21].specialFunction
    {1455, 231, varUINT16},        // SF_Trigger[22].TriggerID
    {1456, 233, varUINT8},        // SF_Trigger[22].specialFunction
    {1457, 234, varUINT16},        // SF_Trigger[23].TriggerID
    {1458, 236, varUINT8},        // SF_Trigger[23].specialFunction
    {1459, 237, varUINT16},        // SF_Trigger[24].TriggerID
    {1460, 239, varUINT8},        // SF_Trigger[24].specialFunction
    {1461, 240, varUINT16},        // SF_Trigger[25].TriggerID
    {1462, 242, varUINT8},        // SF_Trigger[25].specialFunction
    {1463, 243, varUINT16},        // SF_Trigger[26].TriggerID
    {1464, 245, varUINT8},        // SF_Trigger[26].specialFunction
    {1465, 246, varUINT16},        // SF_Trigger[27].TriggerID
    {1466, 248, varUINT8},        // SF_Trigger[27].specialFunction
    {1467, 249, varUINT16},        // SF_Trigger[28].TriggerID
    {1468, 251, varUINT8},        // SF_Trigger[28].specialFunction
    {1469, 252, varUINT16},        // SF_Trigger[29].TriggerID
    {1470, 254, varUINT8},        // SF_Trigger[29].specialFunction
    {1471, 255, varUINT16},        // SF_Trigger[30].TriggerID
    {1472, 257, varUINT8},        // SF_Trigger[30].specialFunction
    {1473, 258, varUINT16},        // SF_Trigger[31].TriggerID
    {1474, 260, varUINT8},        // SF_Trigger[31].specialFunction
    {1475, 261, varUINT16},        // SF_Trigger[32].TriggerID
    {1476, 263, varUINT8},        // SF_Trigger[32].specialFunction
    {1477, 264, varUINT16},        // SF_Trigger[33].TriggerID
    {1478, 266, varUINT8},        // SF_Trigger[33].specialFunction
    {1479, 267, varUINT16},        // SF_Trigger[34].TriggerID
    {1480, 269, varUINT8},        // SF_Trigger[34].specialFunction
    {1481, 270, varUINT16},        // SF_Trigger[35].TriggerID
    {1482, 272, varUINT8},        // SF_Trigger[35].specialFunction
    {1483, 273, varUINT16},        // SF_Trigger[36].TriggerID
    {1484, 275, varUINT8},        // SF_Trigger[36].specialFunction
    {1485, 276, varUINT16},        // SF_Trigger[37].TriggerID
    {1486, 278, varUINT8},        // SF_Trigger[37].specialFunction
    {1487, 279, varUINT16},        // SF_Trigger[38].TriggerID
    {1488, 281, varUINT8},        // SF_Trigger[38].specialFunction
    {1489, 282, varUINT16},        // SF_Trigger[39].TriggerID
    {1490, 284, varUINT8},        // SF_Trigger[39].specialFunction
    {1611, 285, varUINT8},        // DriveMotors
    {1612, 286, varUINT8},        // TurretRotationMotor
    {1613, 287, varUINT8},        // TurretElevationMotor
    {1811, 288, varINT16},        // TurretElevation_EPMin
    {1812, 290, varINT16},        // TurretElevation_EPMax
    {1813, 292, varBOOL},        // TurretElevation_Reversed
    {1814, 293, varUINT8},        // TurretElevation_MaxSpeedPct
    {1815, 294, varUINT8},        // TurretRotation_MaxSpeedPct
    {1816, 295, varINT16},        // TurretRotation_EPMin
    {1817, 297, varINT16},        // TurretRotation_EPMax
    {1818, 299, varBOOL},        // TurretRotation_Reversed
    {1911, 300, varINT16},        // SteeringServo_EPMin
    {1912, 302, varINT16},        // SteeringServo_EPMax
    {1913, 304, varBOOL},        // SteeringServo_Reversed
    {2011, 305, varBOOL},        // Airsoft
    {2012, 306, varBOOL},        // MechanicalBarrelWithCannon
    {2013, 307, varINT16},        // RecoilDelay
    {2014, 309, varBOOL},        // RecoilReversed
    {2015, 310, varBOOL},        // ServoRecoilWithCannon
    {2016, 311, varINT16},        // RecoilServo_Recoil_mS
    {2017, 313, varINT16},        // RecoilServo_Return_mS
    {2018, 315, varINT16},        // RecoilServo_EPMin
    {2019, 317, varINT16},        // RecoilServo_EPMax
    {2020, 319, varUINT8},        // RecoilServo_PresetNum
    {2211, 320, varBOOL},        // SmokerControlAuto
    {2212, 321, varINT16},        // SmokerIdleSpeed
    {2213, 323, varINT16},        // SmokerFastIdleSpeed
    {2214, 325, varINT16},        // SmokerMaxSpeed
    {2215, 327, varINT16},        // SmokerDestroyedSpeed
    {2216, 329, varUINT8},        // SmokerDeviceType
    {2217, 330, varUINT8},        // SmokerPreHeat_Sec
    {2218, 331, varINT16},        // SmokerHeatIdleAmt
    {2219, 333, varINT16},        // SmokerHeatFastIdleAmt
    {2220, 335, varINT16},        // SmokerHeatMaxAmt
    {2221, 337, varUINT8},        // HotStartTimeout_Sec
    {2411, 338, varBOOL},        // AccelRampEnabled_1
    {2412, 339, varUINT8},        // AccelSkipNum_1
    {2413, 340, varUINT8},        // AccelPreset_1
    {2414, 341, varBOOL},        // DecelRampEnabled_1
    {2415, 342, varUINT8},        // DecelSkipNum_1
    {2416, 343, varUINT8},        // DecelPreset_1
    {2417, 344, varBOOL},        // AccelRampEnabled_2
    {2418, 345, varUINT8},        // AccelSkipNum_2
    {2419, 346, varUINT8},        // AccelPreset_2
    {2420, 347, varBOOL},        // DecelRampEnabled_2
    {2421, 348, varUINT8},        // DecelSkipNum_2
    {2422, 349, varUINT8},        // DecelPreset_2
    {2423, 350, varUINT8},        // BrakeSensitivityPct
    {2424, 351, varUINT16},        // TimeToShift_mS
    {2425, 353, varUINT16},        // EnginePauseTime_mS
    {2426, 355, varUINT16},        // TransmissionDelay_mS
    {2427, 357, varBOOL},        // NeutralTurnAllowed
    {2428, 358, varUINT8},        // NeutralTurnPct
    {2429, 359, varUINT8},        // TurnMode
    {2430, 360, varUINT8},        // DriveType
    {2431, 361, varUINT8},        // MaxForwardSpeedPct
    {2432, 362, varUINT8},        // MaxReverseSpeedPct
    {2433, 363, varUINT8},        // HalftrackTreadTurnPct
    {2434, 364, varBOOL},        // EngineAutoStart
    {2435, 365, varINT32},        // EngineAutoStopTime_mS
    {2436, 369, varUINT8},        // MotorNudgePct
    {2437, 370, varUINT16},        // NudgeTime_mS
    {2438, 372, varBOOL},        // DragInnerTrack
    {2439, 373, varBOOL},        // EnableTrackRecoil
    {2440, 374, varUINT8},        // TrackRecoilKickbackSpeed
    {2441, 375, varUINT8},        // TrackRecoilDecelerateFactor
    {2511, 376, varBOOL},        // EnableBarrelStabilize
    {2512, 377, varUINT8},        // BarrelSensitivity
    {2513, 378, varBOOL},        // EnableHillPhysics
    {2514, 379, varUINT8},        // HillSensitivity
    {2711, 380, varINT16},        // IgnoreTurretDelay_mS
    {2811, 382, varUINT8},        // SoundDevice
    {2812, 383, varUINT16},        // Squeak1_MinInterval_mS
    {2813, 385, varUINT16},        // Squeak1_MaxInterval_mS
    {2814, 387, varUINT16},        // Squeak2_MinInterval_mS
    {2815, 389, varUINT16},        // Squeak2_MaxInterval_mS
    {2816, 391, varUINT16},        // Squeak3_MinInterval_mS
    {2817, 393, varUINT16},        // Squeak3_MaxInterval_mS
    {2818, 395, varBOOL},        // Squeak1_Enabled
    {2819, 396, varBOOL},        // Squeak2_Enabled
    {2820, 397, varBOOL},        // Squeak3_Enabled
    {2821, 398, varUINT8},        // MinSqueakSpeed
    {2822, 399, varBOOL},        // HeadlightSound_Enabled
    {2823, 400, varBOOL},        // TurretSound_Enabled
    {2824, 401, varBOOL},        // BarrelSound_Enabled
    {2825, 402, varUINT16},        // Squeak4_MinInterval_mS
    {2826, 404, varUINT16},        // Squeak4_MaxInterval_mS
    {2827, 406, varUINT16},        // Squeak5_MinInterval_mS
    {2828, 408, varUINT16},        // Squeak5_MaxInterval_mS
    {2829, 410, varUINT16},        // Squeak6_MinInterval_mS
    {2830, 412, varUINT16},        // Squeak6_MaxInterval_mS
    {2831, 414, varBOOL},        // Squeak4_Enabled
    {2832, 415, varBOOL},        // Squeak5_Enabled
    {2833, 416, varBOOL},        // Squeak6_Enabled
    {2834, 417, varUINT8},        // VolumeEngine
    {2835, 418, varUINT8},        // VolumeTrackOverlay
    {2836, 419, varUINT8},        // VolumeEffects
    {2837, 420, varBOOL},        // HeadlightSound2_Enabled
    {2838, 421, varBOOL},        // SoundBankA_Loop
    {2839, 422, varBOOL},        // SoundBankB_Loop
    {3011, 423, varUINT8},        // IR_FireProtocol
    {3012, 424, varUINT8},        // IR_HitProtocol_2
    {3013, 425, varUINT8},        // IR_RepairProtocol
    {3014, 426, varUINT8},        // IR_MGProtocol
    {3015, 427, varBOOL},        // Use_MG_Protocol
    {3016, 428, varBOOL},        // Accept_MG_Damage
    {3017, 429, varUINT8},        // DamageProfile
    {3018, 430, varUINT16},        // CustomClassSettings.reloadTime
    {3019, 432, varUINT16},        // CustomClassSettings.recoveryTime
    {3020, 434, varUINT8},        // CustomClassSettings.maxHits
    {3021, 435, varUINT8},        // CustomClassSettings.maxMGHits
    {3022, 436, varBOOL},        // SendTankID
    {3023, 437, varUINT16},        // TankID
    {3024, 439, varUINT8},        // IR_Team
    {3211, 440, varUINT32},        // USBSerialBaud
    {3212, 444, varUINT32},        // AuxSerialBaud
    {3213, 448, varUINT32},        // MotorSerialBaud
    {3214, 452, varUINT32},        // Serial3TxBaud
    {3215, 456, varBOOL},        // LVC_Enabled
    {3216, 457, varUINT16},        // LVC_Cutoff_mV
    {3411, 459, varBOOL},        // RunningLightsAlwaysOn
    {3412, 460, varUINT8},        // RunningLightsDimLevelPct
    {3413, 461, varBOOL},        // BrakesAutoOnAtStop
    {3414, 462, varUINT16},        // AuxLightFlashTime_mS
    {3415, 464, varUINT16},        // AuxLightBlinkOnTime_mS
    {3416, 466, varUINT16},        // AuxLightBlinkOffTime_mS
    {3417, 468, varUINT8},        // AuxLightPresetDim
    {3418, 469, varUINT8},        // MGLightBlink_mS
    {3419, 470, varBOOL},        // FlashLightsWhenSignalLost
    {3420, 471, varBOOL},        // HiFlashWithCannon
    {3421, 472, varBOOL},        // AuxFlashWithCannon
    {3422, 473, varUINT8},        // SecondMGLightBlink_mS
    {3423, 474, varBOOL},        // CannonReloadBlink
    {3611, 475, varUINT8},        // ScoutCurrentLimit
    {9011, 476, varBOOL},        // PrintDebug
    {9999, 477, varUINT32}        // InitStamp
};


#endif  // Define OP_EEPROM_VARINFO_H