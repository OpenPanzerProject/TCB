#ifndef OP_EEPROM_STRUCT_H
#define OP_EEPROM_STRUCT_H

// EEPROM DATA STRUCTURE
//--------------------------------------------------------------------------------------------------------------------------------------->>
// This defines every variable held in EEPROM

// THESE MUST BE THE PRECISE SAME DATA-TYPES, and in the PRECISE SAME ORDER, as listed in the EEPROM_Vars Excel file
//--------------------------------------------------------------------------------------------------------------------------------------->>


struct _eeprom_data {                                  
// First Var
	uint8_t FirstVar;

// Stick Channel settings
	stick_channel_settings ThrottleSettings;
	stick_channel_settings TurnSettings;
	stick_channel_settings ElevationSettings;
	stick_channel_settings AzimuthSettings;

// Aux Channel settings
	aux_channel_settings Aux_ChannelSettings[AUXCHANNELS];

// External I/O ports
	external_io_settings PortA_Settings;
	external_io_settings PortB_Settings;

// Special function triggers
	_functionTrigger SF_Trigger[MAX_FUNCTION_TRIGGERS];	// Info for each trigger, up to MAX_FUNCTION_TRIGGERS

// 4 main motor drive types
	Drive_t DriveMotors;                       // (in fact a char)
	Drive_t TurretRotationMotor;
	Drive_t TurretElevationMotor;

//	Elevation motor min/max/reversed
	int16_t TurretElevation_EPMin;             // If the elevation motor selection is of type servo, we allow customized endpoints
	int16_t TurretElevation_EPMax;
	boolean TurretElevation_Reversed;		   // And we have a motor-specific reversed flag as well, so it doesn't have to be tied to the stick reversed setting

// Mechanical Barrel and Recoil Servo settings
	boolean Airsoft;						   // If true, we are controlling an airsoft unit. If false, we are controlling mechanical recoil unit. 
	boolean MechanicalBarrelWithCannon;		   // If true, mechanical barrel (airsoft or recoil) will trigger automatically with cannon fire. If false, 
											   // the user can trigger it manually, otherwise it remains inactive. 
	int16_t RecoilDelay;				   	   // The Taigen mechanical recoil units in particular, need to run for a brief period of time
											   // before the spring engages and actually recoils the barrel. If the flash and sounds go off instantly,
											   // they won't appear co-ordinated with the recoil movement. You can specify a delay in milliseconds to 
 											   // account for this - the mechanical (AND servo) recoils wil be triggered, the program will wait for RecoilDelay mS, 
											   // then the flash and sound will be triggered. Set this to 0 to disable the delay. 
	boolean RecoilReversed;                    // Reverse the direction of the recoil servo
	boolean ServoRecoilWithCannon;			   // If true, servo recoil will automatically occur with cannon fire (and airsoft or mech recoil). If false, servo recoil trigger can be set by user. 
	int16_t RecoilServo_Recoil_mS;             // Time in milliseconds for barrel to recoil
	int16_t RecoilServo_Return_mS;             // Time in milliseconds for barrel to return
	int16_t RecoilServo_EPMin;                 // Recoil servo endpoint adjustments. Min will be recoiled position
	int16_t RecoilServo_EPMax;                 // Max will be returned position


// On board smoker output
	boolean SmokerControlAuto;				   // If true, Smoker will be set according to engine speed. If false, smoker can be set manually with an analog function trigger.
	int16_t SmokerIdleSpeed;
	int16_t SmokerFastIdleSpeed;
	int16_t SmokerMaxSpeed;

// Driving adjustments
	boolean AccelRampEnabled;
	uint8_t AccelSkipNum;
	ACCEL_DRIVE_PRESET AccelPreset;
	boolean DecelRampEnabled;
	uint8_t DecelSkipNum;
	DECEL_DRIVE_PRESET DecelPreset;
	uint8_t BrakeSensitivityPct;
	uint16_t TimeToShift_mS;
	uint16_t EnginePauseTime_mS;				 // How long to wait before we can turn the engine on/off since the last engine on/off. Should be at least as long as shutdown sound.
	uint16_t TransmissionDelay_mS;				 // Set this to the length of your engine startup sound
	boolean NeutralTurnAllowed;                  // true if neutral turns are allowed, false if not. 
	uint8_t NeutralTurnPct;                      // What percent of DRIVEMOTOR_MAX_FWDSPEED are we allowed to reach during neutral turn
	uint8_t TurnMode;                            // Mode 1: turn subtracted from inner track only, even if it means throwing the inner into reverse.
												 //         This mode essentially equals Mode 2 if Param_TurnScale is true
												 // Mode 2: turn subtracted from inner track but the slowest it can go is 0 (no negative). This is the way it's done in real life. 
												 // Mode 3: inner track slowed, outer sped up
	DRIVETYPE DriveType;						 // Type 1: Tank - two independent treads used for forward movement and steering
												 // Type 2: Halftrack - again two independent treads that can be used to steer, but with front steering wheels as well (servo output)
												 // Type 3: Car - steering accomplished entirey by the front wheels, single rear drive. Can also be used with halftracks, but rear treads 
												 //         will not be independent. 
	uint8_t MaxReverseSpeedPct;					 // Reverse speed as percent of full speed
	uint8_t HalftrackTreadTurnPct;				 // What percent of turn command gets applied to the treads in Halftrack mode (100% is always applied to the steering servo)
	boolean EngineAutoStart;					 // If true, engine will auto-start on first blip of throttle. If false, start engine with user selected trigger.
	int32_t EngineAutoStopTime_mS;				 // If positive, after this amount of time at idle the engine will turn itself off (in milliseconds). 0 to disable. 
	uint8_t MotorNudgePct;					 	 // To overcome the initial inertia of a stopped vehicle, we can cause throttle to jump to a higher level temporarily when 
												 //    first moving from a stop. 0 will disable the effect, otherwise 1-100 will cause the throttle to begin at 1-100% of total. 
	uint16_t NudgeTime_mS;						 // How long will the nudge effect last in milliseconds. 
	
// IMU Physics
	boolean EnableBarrelStabilize;				 // If an accelerometer is present, and turret elevation motor is type SERVO_PAN, this will stabilize the barrel
	uint8_t BarrelSensitivity;					 // Sensitivity number from 1 - 10
	boolean EnableHillPhysics;					 // If an accelerometer is present, this will cause the tank to slow down on uphill climbs, and speed up on downhill. 
	uint8_t HillSensitivity;					 // Sensitivity number from 1 - 10

// Turret stick adjustments
    int16_t IgnoreTurretDelay_mS;				 // If we are triggering special functions from the turret stick, we can set a slight delay so that quick stick movements to the extremes
												 // will trigger the special position without moving the turret. Ignored if all special functions are assigned to auxillary channels.
												 // See GetRxCommands() on the RadioInputs tab of the sketch for more. 
// Sound settings
	uint8_t  SoundDevice;						 // Which sound device are we connected to (Benedini TBS-Mini, etc.)
	uint16_t Squeak1_MinInterval_mS;			 // Minimum length of time between Squeak 1 intervals
	uint16_t Squeak1_MaxInterval_mS;			 // Maximum length of time between Squeak 1 intervals 
	uint16_t Squeak2_MinInterval_mS;			 // Minimum length of time between Squeak 2 intervals
	uint16_t Squeak2_MaxInterval_mS;			 // Maximum length of time between Squeak 2 intervals 
	uint16_t Squeak3_MinInterval_mS;			 // Minimum length of time between Squeak 3 intervals
	uint16_t Squeak3_MaxInterval_mS;			 // Maximum length of time between Squeak 3 intervals 
	boolean  Squeak1_Enabled;					 // Is Squeak1 enabled or not
	boolean  Squeak2_Enabled;					 // Is Squeak2 enabled or not
	boolean  Squeak3_Enabled;					 // Is Squeak3 enabled or not
	uint8_t	 MinSqueakSpeedPct;					 // Prevent squeaks from occuring when vehicle is moving slower than this percent of movement
	boolean  HeadlightSound_Enabled;			 // Is the headlight sound enabled or not
	boolean  TurretSound_Enabled;				 // Is turret rotation sound enabled or not

// Battle settings
	IRTYPES IR_FireProtocol;                     // Which battle protocol are we *sending* by cannon fire
	IRTYPES IR_HitProtocol_2;					 // Optional second protocol to take hits *from*
	IRTYPES IR_RepairProtocol;					 // Which repair protocol are we using
	IRTYPES IR_MGProtocol;						 // Which machine gun protocol are we using
	boolean Use_MG_Protocol;                     // If true, the Machine Gun IR code will be sent when firing the machine gun, otherwise, it will be skipped
	boolean Accept_MG_Damage;                    // If true, the vehicle will take damage with MG fire. 
	DAMAGEPROFILES DamageProfile;				 // Profile for setting damage on each hit. 
	weightClassSettings CustomClassSettings;	 // User custom weight class: reload time, recovery time, max hits, max machine gun hits
	boolean SendTankID;							 // Do we include the Tank ID in the cannon IR transmission
	uint16_t TankID;							 // Tank ID number	
	IRTEAMS	IR_Team;							 // Does the tank belong to a team. Only applies to a few protocols.

// Board settings
	uint32_t USBSerialBaud;                      // Hardware Serial 0
	uint32_t AuxSerialBaud;                      // Hardware Serial 1
	uint32_t MotorSerialBaud;                    // Hardware Serial 2
	uint32_t Serial3TxBaud;						 // Hardware Serial 3. Tx only. And disabled completely if using SBus.
	boolean  LVC_Enabled;						 // Enable low-voltage cutoff 
	uint16_t LVC_Cutoff_mV;					     // Minimum voltage level (in millivolts)
		
// Light settings
	boolean  RunningLightsAlwaysOn;				 // If true, brake light output will always be on at the DIM level, unless braking, then it will bright. If false, user can turn this on/off with some input
	uint8_t  RunningLightsDimLevelPct;			 // What brightness level are the running lights - some number between 0-100
	boolean  BrakesAutoOnAtStop;				 // If true, brake lights will come on whenever the vehicle is stopped. If false, they will only come on during braking. 
	uint16_t AuxLightFlashTime_mS;				 // If Aux output is set to Flash, what is the on time in milliseconds.
	uint16_t AuxLightBlinkOnTime_mS;			 // Only applies if Aux lights are triggered to blink. This is the time the blink stays on. 
	uint16_t AuxLightBlinkOffTime_mS;			 // This is the time the blink stays off. 
	uint8_t  AuxLightPresetDim;					 // The user has the option of turning on the Aux light to a pre-set dim level
	uint8_t  MGLightBlink_mS;					 // Some number between 0-255. The Machine Gun blinks on/off at the same rate (on period is not different from off period).
	boolean  FlashLightsWhenSignalLost; 		 // If radio signal lost, flash all lights. 
	boolean  HiFlashWithCannon;					 // If true, the high-intensity flash unit will automatically be triggered with cannon fire. If False, user can still trigger it manually.
			
// Program setting
	boolean PrintDebug;							 // If true, TCB will print debugging messages out the DebugSerial serial port

// Marker
	uint32_t InitStamp;       		 		     
};


#endif	// Define OP_EEPROM_STRUCT_H