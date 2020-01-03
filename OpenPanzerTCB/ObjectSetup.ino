


// This function assigns the correct motor object to the drive, turret and recoil motors based on the user's selections which we retrieved from EEPROM
void InstantiateMotorObjects() 
{ 
    // Start these as true, and set to false below if necessary
    RCOutput1_Available = true;    
    RCOutput2_Available = true;    
    RCOutput3_Available = true;    
    RCOutput4_Available = true;    
    MotorA_Available = true;
    MotorB_Available = true;
    uint8_t SteeringServoNum = 255; // Initialize to value that means nothing

   
    // DRIVE MOTORS
    // -------------------------------------------------------------------------------------------------------------------------------------->>
    // Sanity check - make sure DriveType is valid
    if (eeprom.ramcopy.DriveType < DT_TANK || eeprom.ramcopy.DriveType > LAST_DT)
    {   // Default to tank if we have some invalid value, and update EEPROM too
        eeprom.ramcopy.DriveType = DT_TANK;
        EEPROM.updateInt(offsetof(_eeprom_data, DriveType), DT_TANK);
    }

    // These drive types involve un-mixed drive and steering outputs. It could be a car, or a tank driven by some device that already takes care of mixing (DKLM gearbox, Tamiya DMD unit)
    if (eeprom.ramcopy.DriveType == DT_CAR || eeprom.ramcopy.DriveType == DT_DKLM || eeprom.ramcopy.DriveType == DT_DMD)
    {   
        switch (eeprom.ramcopy.DriveMotors)
        {
            case OP_SCOUT:
                // For a single rear drive motor (or a single propulsion motor), connect it to M1
                DriveMotor = new OPScout_SerialESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,OPScout_DRIVE_Address,&MotorSerial,&eeprom.ramcopy.MotorSerialBaud, false, eeprom.ramcopy.ScoutCurrentLimit);    
                DriveMotor->begin();
                
                // For ancient Tamiya gearboxes, the DKLM "Propulsion Dynamic" gearboxes, and any others that use a single motor for drive and a secondary motor to shift power from one tread to the other, 
                // we have a "SteeringMotor" which will be the otherwise unused second output of the dual-motor serial controller 
                SteeringMotor = new OPScout_SerialESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,OPScout_DRIVE_Address,&MotorSerial,&eeprom.ramcopy.MotorSerialBaud, false, eeprom.ramcopy.ScoutCurrentLimit);
                SteeringMotor->begin();    
                break;
                
            case SABERTOOTH:
                // For a single rear drive motor (or a single propulsion motor), connect it to M1
                DriveMotor = new Sabertooth_SerialESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Sabertooth_DRIVE_Address,&MotorSerial);
                DriveMotor->begin();
                
                // For ancient Tamiya gearboxes, the DKLM "Propulsion Dynamic" gearboxes, and any others that use a single motor for drive and a secondary motor to shift power from one tread to the other, 
                // we have a "SteeringMotor" which will be the otherwise unused second output of the dual-motor serial controller 
                SteeringMotor = new Sabertooth_SerialESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Sabertooth_DRIVE_Address,&MotorSerial);
                SteeringMotor->begin();
                break;
    
            case POLOLU:
                // For a single rear drive motor (or a single propulsion motor), connect it to M0
                DriveMotor = new Pololu_SerialESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Pololu_DRIVE_ID,&MotorSerial);
                DriveMotor->begin();

                // For ancient Tamiya gearboxes, the DKLM "Propulsion Dynamic" gearboxes, and any others that use a single motor for drive and a secondary motor to shift power from one tread to the other, 
                // we have a "SteeringMotor" which will be the otherwise unused second output of the dual-motor serial controller 
                SteeringMotor = new Pololu_SerialESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Pololu_DRIVE_ID,&MotorSerial);
                SteeringMotor->begin();
                break;
    
            case ONBOARD:
                // For a single rear drive motor (or a single propulsion motor), connect it to  MOTOR A
                DriveMotor = new Onboard_ESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                DriveMotor->begin();                
                MotorA_Available = false;

                if (eeprom.ramcopy.DriveType == DT_DKLM)
                {
                    // For ancient Tamiya gearboxes, the DKLM "Propulsion Dynamic" gearboxes, and any others that use a single motor for drive and a secondary motor to shift power from one tread to the other, 
                    // we have a "SteeringMotor" which will be MOTOR B. Technically one should not be using this option for those gearboxes because they will exceed the current draw limits of the 
                    // onboard driver, but perhaps some day this same code will be used on different hardware. 
                    SteeringMotor = new Onboard_ESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                    SteeringMotor->begin();
                    MotorB_Available = false;       // This becomes unavailable for general purpose controller
                }
                // else - if car, we leave MotorB available as a general purpose controller. 
                break;
                
            case SERVO_ESC:
                if (eeprom.ramcopy.DriveType == DT_DKLM)
                {
                    // For ancient Tamiya gearboxes, the DKLM "Propulsion Dynamic" gearboxes, and any others that use a single motor for drive and a secondary motor to mechanically shift power from one tread to the other. 
                    // Drive motor will be on RC Output 1
                    DriveMotor = new Servo_ESC (SERVONUM_LEFTTREAD,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                    DriveMotor->begin();                    
                    RCOutput1_Available = false;    // This slot becomes unavailable for general purpose servo
                    
                    // Steering motor will be RC Output 2
                    SteeringMotor = new Servo_ESC (SERVONUM_RIGHTTREAD,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                    SteeringMotor->begin();
                    RCOutput2_Available = false;    // This slot becomes unavailable for general purpose servo
                }
                else if (eeprom.ramcopy.DriveType == DT_DMD)
                {
                    // When using the Tamiya DMD we are doing the same thing as the DKLM but Tamiya uses a different order (Channel 1 steering, Channel 2 throttle)
                    // Steering motor will be RC Output 1
                    SteeringMotor = new Servo_ESC (SERVONUM_LEFTTREAD,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                    SteeringMotor->begin();
                    RCOutput1_Available = false;    // This slot becomes unavailable for general purpose servo                    

                    // Drive motor will be on RC Output 2
                    DriveMotor = new Servo_ESC (SERVONUM_RIGHTTREAD,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                    DriveMotor->begin();                 
                    RCOutput2_Available = false;    // This slot becomes unavailable for general purpose servo                    
                }
                else // Car or halftrack with single drive
                {
                    // For a single rear drive motor, connect it to the "Left" servo port (Servo 1)
                    DriveMotor = new Servo_ESC (SERVONUM_LEFTTREAD,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                    DriveMotor->begin();                    
                    RCOutput1_Available = false;
                    // We will also create a steering servo output, see below
                }
                break;
    
            default:
                // We shouldn't end up here but in case we do, we need to define something or else the program will croak at runtime.
                // We set it to SABERTOOTH, and save it to EEPROM so we don't end up here again next time
                eeprom.ramcopy.DriveMotors = SABERTOOTH;
                EEPROM.updateInt(offsetof(_eeprom_data, DriveMotors), SABERTOOTH);
                DriveMotor = new Sabertooth_SerialESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Sabertooth_DRIVE_Address,&MotorSerial);
                DriveMotor->begin();
                SteeringMotor = new Sabertooth_SerialESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Sabertooth_DRIVE_Address,&MotorSerial);
                SteeringMotor->begin();
        }
    }
    else
    {   // The user wants independent tread speeds, either tank or halftrack or direct control of each track
        switch (eeprom.ramcopy.DriveMotors)
        {
            case OP_SCOUT:
                // Left drive to M1, Right drive to M2. 
                LeftTread = new OPScout_SerialESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,OPScout_DRIVE_Address,&MotorSerial,&eeprom.ramcopy.MotorSerialBaud, eeprom.ramcopy.DragInnerTrack, eeprom.ramcopy.ScoutCurrentLimit);    
                RightTread = new OPScout_SerialESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,OPScout_DRIVE_Address,&MotorSerial,&eeprom.ramcopy.MotorSerialBaud, eeprom.ramcopy.DragInnerTrack, eeprom.ramcopy.ScoutCurrentLimit);
                break;
                
            case SABERTOOTH:
                // Left drive to M1, Right drive to M2. 
                LeftTread = new Sabertooth_SerialESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Sabertooth_DRIVE_Address,&MotorSerial);
                RightTread = new Sabertooth_SerialESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Sabertooth_DRIVE_Address,&MotorSerial);
                break;
    
            case POLOLU:
                // Left drive to M0, Right drive to M1
                LeftTread = new Pololu_SerialESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Pololu_DRIVE_ID,&MotorSerial);
                RightTread = new Pololu_SerialESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Pololu_DRIVE_ID,&MotorSerial);
                break;
    
            case ONBOARD:
                // Left drive to Motor A, Right drive to Motor B
                LeftTread = new Onboard_ESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                RightTread = new Onboard_ESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                MotorA_Available = false;
                MotorB_Available = false;
                break;
                
            case SERVO_ESC:
                // Left drive to RC Output 1, Right to RC Output 2
                LeftTread = new Servo_ESC (SERVONUM_LEFTTREAD,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                RightTread = new Servo_ESC (SERVONUM_RIGHTTREAD,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                RCOutput1_Available = false;
                RCOutput2_Available = false;
                break;
    
            default:
                // We shouldn't end up here but in case we do, we need to define something or else the program will croak at runtime
                // We set it to SABERTOOTH, and save it to EEPROM so we don't end up here again next time
                eeprom.ramcopy.DriveMotors = SABERTOOTH;
                EEPROM.updateInt(offsetof(_eeprom_data, DriveMotors), SABERTOOTH);
                LeftTread = new Sabertooth_SerialESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Sabertooth_DRIVE_Address,&MotorSerial);
                RightTread = new Sabertooth_SerialESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Sabertooth_DRIVE_Address,&MotorSerial);
        }
        // Now initialize the motors
        RightTread->begin();        
        LeftTread->begin();
    }

    // STEERING SERVO
    // -------------------------------------------------------------------------------------------------------------------------------------->>
    // Cars and halftrack requre a steering servo. In the case of a car or a halftrack without independent tread control, the steering servo will get 
    // assigned to the Right tread servo port (RC Output 2), leaving RC Output 1 free for the drive output should they choose to control it with an RC ESC (likely).
    // However if they want independent tread control and they are using RC then both RC Outputs 1 and 2 will be taken, in that case we re-purpose the barrel elevation 
    // output for the steering servo, which also means they will be prevented from using RC Output for barrel elevation
    if (eeprom.ramcopy.DriveType == DT_CAR || (eeprom.ramcopy.DriveType == DT_HALFTRACK && eeprom.ramcopy.DriveMotors != SERVO_ESC))
    {   // They are using a single axle drive, or if they are using independent treads they are not using RC for it - in this case 
        // steering will be assigned to RC Output 2
        SteeringServo = new Servo_ESC (SERVONUM_RIGHTTREAD,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
        // Initialize the servo
        SteeringServo->begin();
        // This slot is unavailable for general purpose servo
        RCOutput2_Available = false;
        // Save this temporarily for the next bit
        SteeringServoNum = SERVONUM_RIGHTTREAD;
    }
    else if (eeprom.ramcopy.DriveType == DT_HALFTRACK && eeprom.ramcopy.DriveMotors == SERVO_ESC)
    {   // Here they want independent control by RC, meaning 1 and 2 are taken and we will assign steering to 4 instead (barrel elevation)
        SteeringServo = new Servo_ESC (SERVONUM_TURRETELEVATION,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
        // Initialize the servo
        SteeringServo->begin();
        // This slot is unavailable for general purpose servo
        RCOutput4_Available = false;
        // Save this temporarily for the next bit
        SteeringServoNum = SERVONUM_TURRETELEVATION;
    }
    
    // We may also have some custom end-points defined for the steering servo. 
    // These end-points need to be set after the begin() statement, which initializes the endpoints to defaults.
    if (SteeringServoNum == SERVONUM_TURRETELEVATION || SteeringServoNum == SERVONUM_RIGHTTREAD)
    {   
        // The end-points are actually handled in the servo class, not the motor class. Since SteeringServo is a pointer to the motor class, 
        // and not the servo class we need to call the servo class directly using TankServos (even though in this case SteeringServo is also a subclass of servo)
        TankServos.setMinPulseWidth(SteeringServoNum, eeprom.ramcopy.SteeringServo_EPMin);
        TankServos.setMaxPulseWidth(SteeringServoNum, eeprom.ramcopy.SteeringServo_EPMax);
        
        // Use the reversed setting of the motor class since servo doesn't have one. This won't affect the reverse status of the drive motors in the case of halftrack, that is dependent on the radio channel reversed status
        SteeringServo->set_Reversed(eeprom.ramcopy.SteeringServo_Reversed);
    }
    

    // TURRET MOTOR DEFINITION - ROTATION
    // -------------------------------------------------------------------------------------------------------------------------------------->>
    switch (eeprom.ramcopy.TurretRotationMotor)
    {
        case OP_SCOUT:      // M1
            TurretRotation = new OPScout_SerialESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,OPScout_TURRET_Address,&MotorSerial,&eeprom.ramcopy.MotorSerialBaud, false, eeprom.ramcopy.ScoutCurrentLimit);
            break;
        case SABERTOOTH:    // M1
            TurretRotation = new Sabertooth_SerialESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Sabertooth_TURRET_Address,&MotorSerial);
            break;
        case POLOLU:        // M0
             TurretRotation = new Pololu_SerialESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Pololu_TURRET_ID,&MotorSerial);
            break;
        case ONBOARD:       // Motor A
            TurretRotation = new Onboard_ESC (SIDEA,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
            MotorA_Available = false;
            break;
        case SERVO_ESC:
            TurretRotation = new Servo_ESC (SERVONUM_TURRETROTATION,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
            RCOutput3_Available = false;
            break;
        case SERVO_PAN:
            TurretRotation = new Servo_PAN (SERVONUM_TURRETROTATION,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
            RCOutput3_Available = false;
            break;
        case DRIVE_DETACHED:
            // In this case we don't need a turret motor object, and even if we create one, it won't be controlled in any way by the turret stick. 
            // However we do in fact create one, because so much code refers to the TurretRotation object it would be difficult to maintain as well as
            // cumbersome to always add a check for drive type before any action to be taken on the object. We will of course need to check the drive 
            // type in some cases, most importantly when processing turret stick movements, which should apply only if the drive type is not DRIVE_DETACHED.
            // Most other instances are harmless, but would become bugs if an object didn't exist. We use a special Null_Motor object that does nothing. 
            TurretRotation = new Null_Motor();
            break;
        default:
            // We shouldn't end up here but in case we do, we need to define something or else the program will croak at runtime
            // We set it to SERVO_ESC, and save it to EEPROM so we don't end up here next time. 
            eeprom.ramcopy.TurretRotationMotor = SERVO_ESC;
            EEPROM.updateInt(offsetof(_eeprom_data, TurretRotationMotor), SERVO_ESC);
            TurretRotation = new Servo_ESC (SERVONUM_TURRETROTATION,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
            RCOutput3_Available = false;
    }
    // Now initialize the motor
    TurretRotation->begin(); 
    // The user also has the option of limiting max turret rotation speed. We can easily do this by using the set_MaxSpeedPct function of the motor object. 
    // Of course this will cause unintended consequences if the motor type is set to SERVO_ESC and they are using an unmodified hobby servo (ie, not a continuous rotation servo). 
    // In that case limiting the max speed will actually just limit the servo's travel. However for turret rotation it is unlikely an unmodified hobby servo would be used. 
    // We could do a check for SERVO_ESC, but that won't help, because the user could have plugged in a continuous rotation servo, or be using a hobby ESC, and still selected SERVO_ESC,
    // and in those cases limiting the max speed would indeed work as intended. 
    if (eeprom.ramcopy.TurretRotation_MaxSpeedPct < 100) { TurretRotation->set_MaxSpeedPct(eeprom.ramcopy.TurretRotation_MaxSpeedPct); }   
    
    // We may also have some custom end-points defined for the rotation motor, if it is set to servo. 
    // These end-points need to be set after the begin() statement, which initializes the endpoints to defaults.
    if (eeprom.ramcopy.TurretRotationMotor == SERVO_ESC || eeprom.ramcopy.TurretRotationMotor == SERVO_PAN)
    {
        // The end-points are actually handled in the servo class, not the motor class. Since TurretRotation is a pointer to the motor class, 
        // and not the servo class we need to call the servo class directly using TankServos (even though in this case TurretRotation is also a subclass of servo)
        TankServos.setMinPulseWidth(SERVONUM_TURRETROTATION, eeprom.ramcopy.TurretRotation_EPMin);
        TankServos.setMaxPulseWidth(SERVONUM_TURRETROTATION, eeprom.ramcopy.TurretRotation_EPMax);
        
        // For turret rotation, we use the reversed setting of the motor class. 
        // FYI, we don't have a reversed setting for traditional motors (Sabertooth, onboard, etc...) because in those cases you can just swap the motor wires. 
        TurretRotation->set_Reversed(eeprom.ramcopy.TurretRotation_Reversed);
    }

    // TURRET MOTOR DEFINITION - ELEVATION
    // -------------------------------------------------------------------------------------------------------------------------------------->>
    switch (eeprom.ramcopy.TurretElevationMotor)
    {
        case OP_SCOUT:      // M2
            TurretElevation = new OPScout_SerialESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,OPScout_TURRET_Address,&MotorSerial,&eeprom.ramcopy.MotorSerialBaud, false, eeprom.ramcopy.ScoutCurrentLimit);
            break;
        case SABERTOOTH:    // M2
            TurretElevation = new Sabertooth_SerialESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Sabertooth_TURRET_Address,&MotorSerial);
            break;
        case POLOLU:        // M1
            TurretElevation = new Pololu_SerialESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,Pololu_TURRET_ID,&MotorSerial);
            break;
        case ONBOARD:       // Motor B
            TurretElevation = new Onboard_ESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
            MotorB_Available = false;
            break;
        case SERVO_ESC:
        case SERVO_PAN:
            if (eeprom.ramcopy.DriveType == DT_HALFTRACK && eeprom.ramcopy.DriveMotors == SERVO_ESC)
            {   // This case should not happen. If the user wants independent treads, controlled by RC, *and* a steering servo (halftrack),
                // then we have decreed the barrel elevation RC output will be re-purposed for the steering servo, and therefore the barrel 
                // elevation must be controlled by something other than RC (like onboard or serial control)
                // What do we do? We change the barrel elevation to Onboard and proceed as if that had been the selection. 
                eeprom.ramcopy.TurretElevationMotor = ONBOARD;
                // We change it in EEPROM as well, so it will be fixed next time
                EEPROM.updateInt(offsetof(_eeprom_data, TurretElevationMotor), ONBOARD);
                // Now create the object
                TurretElevation = new Onboard_ESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                MotorB_Available = false;
            }    
            else
            {
                // It's ok to use RC output 
                if (eeprom.ramcopy.TurretElevationMotor == SERVO_ESC)
                {
                    TurretElevation = new Servo_ESC (SERVONUM_TURRETELEVATION,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                    RCOutput4_Available = false;
                }
                else if (eeprom.ramcopy.TurretElevationMotor == SERVO_PAN)
                {
                    TurretElevation = new Servo_PAN (SERVONUM_TURRETELEVATION,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                    // TurretElevation is a pointer of class Motor. But if we are using barrel stabilization it will be useful to have an 
                    // object of Servo_PAN type directly, we call this one Barrel. The two are the same, but Barrel will expose some methods that TurretElevation won't have.
                    // FYI: Barrel stabilization can only be enabled if TurretElevation is set to pan servo, so you won't see this under any other category. 
                    Barrel = new Servo_PAN (SERVONUM_TURRETELEVATION,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                    Barrel->begin();    // Initialize the barrel
                    RCOutput4_Available = false;
                }
            }
            break;
        case DRIVE_DETACHED:
            // In this case we don't need a turret motor object, and even if we create one, it won't be controlled in any way by the turret stick. 
            // However we do in fact create one, because so much code refers to the TurretRotation object it would be difficult to maintain as well as
            // cumbersome to always add a check for drive type before any action to be taken on the object. We will of course need to check the drive 
            // type in some cases, most importantly when processing turret stick movements, which should apply only if the drive type is not DRIVE_DETACHED.
            // Most other instances are harmless, but would become bugs if an object didn't exist. We use a special Null_Motor object that does nothing. 
            TurretElevation = new Null_Motor();
            break;
        default:
            // We shouldn't end up here but in case we do, we need to define something or else the program will croak at runtime
            if (eeprom.ramcopy.DriveType == DT_HALFTRACK && eeprom.ramcopy.DriveMotors == SERVO_ESC)
            {
                // If we have repurposed the barrel elevation RC output for steering servo, set it to Onboard
                eeprom.ramcopy.TurretElevationMotor = ONBOARD;
                EEPROM.updateInt(offsetof(_eeprom_data, TurretElevationMotor), ONBOARD);
                TurretElevation = new Onboard_ESC (SIDEB,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                MotorB_Available = false;
            }
            else
            {
                // Otherwise set it to SERVO_ESC, and save it to EEPROM so we don't end up here next time. 
                eeprom.ramcopy.TurretElevationMotor = SERVO_ESC;
                EEPROM.updateInt(offsetof(_eeprom_data, TurretElevationMotor), SERVO_ESC);            
                TurretElevation = new Servo_ESC (SERVONUM_TURRETELEVATION,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0);
                RCOutput4_Available = false;
            }
    }
    // Now initialize the motor
    TurretElevation->begin();    
    // The user also has the option of limiting max turret elevation speed. We can easily do this by using the set_MaxSpeedPct function of the motor object. 
    // Of course this will cause unintended consequences if the motor type is set to SERVO_ESC and they are using an unmodified hobby servo (ie, not a continuous rotation servo). 
    // For barrel elevation, that very well could be the case although we suggest that SERVO_PAN is a better choice for hobby servos in this application, which the user would
    // hopefully use instead, and in which case the speed limitation will work. 
    // Note we do not set a speed limitation on the Barrel copy object. That one is only used for barrel stabilization and in that case we don't want any limit on the speed of the servo. 
    if (eeprom.ramcopy.TurretElevation_MaxSpeedPct < 100) { TurretElevation->set_MaxSpeedPct(eeprom.ramcopy.TurretElevation_MaxSpeedPct); }
    
    // We may also have some custom end-points defined for the elevation motor, if it is set to servo. 
    // These end-points need to be set after the begin() statement, which initializes the endpoints to defaults.
    if (eeprom.ramcopy.TurretElevationMotor == SERVO_ESC || eeprom.ramcopy.TurretElevationMotor == SERVO_PAN)
    {
        // The end-points are actually handled in the servo class, not the motor class. Since TurretElevation is a pointer to the motor class, 
        // and not the servo class we need to call the servo class directly using TankServos (even though in this case TurretElevation is also a subclass of servo)
        TankServos.setMinPulseWidth(SERVONUM_TURRETELEVATION, eeprom.ramcopy.TurretElevation_EPMin);
        TankServos.setMaxPulseWidth(SERVONUM_TURRETELEVATION, eeprom.ramcopy.TurretElevation_EPMax);
        
        // For turret elevation, we use the reversed setting of the motor class. 
        // FYI, we don't have a reversed setting for traditional motors (Sabertooth, onboard, etc...) because in those cases you can just swap the motor wires. 
        TurretElevation->set_Reversed(eeprom.ramcopy.TurretElevation_Reversed);
        
        // If we have a duplicate barrel object, set its reversed flag as well. 
        if (eeprom.ramcopy.TurretElevationMotor == SERVO_PAN)
        { Barrel->set_Reversed(eeprom.ramcopy.TurretElevation_Reversed); }
    }

    // RECOIL SERVO DEFINITION
    // -------------------------------------------------------------------------------------------------------------------------------------->>
	// We still pass an external min/max speed although it won't be used for this object. 
	// What will be used are recoil/return times, along with a reverse setting if the servo needs to be reversed. These can be modified
	// later but will be initialized to sensible defaults.
        RecoilServo = new Servo_RECOIL (SERVONUM_RECOIL,MOTOR_MAX_REVSPEED,MOTOR_MAX_FWDSPEED,0,eeprom.ramcopy.RecoilServo_Recoil_mS,eeprom.ramcopy.RecoilServo_Return_mS,eeprom.ramcopy.RecoilReversed);
        // Recoil servos also have custom end-points. Because RecoilServo is a motor of class Servo, we can call setMin/MaxPulseWidth from the servo class directly, rather than from TankServos
        RecoilServo->setMinPulseWidth(SERVONUM_RECOIL, eeprom.ramcopy.RecoilServo_EPMin);
        RecoilServo->setMaxPulseWidth(SERVONUM_RECOIL, eeprom.ramcopy.RecoilServo_EPMax);
        // The reversed setting needs to be applied both to the motor class (flag) as well as to the servo class (actual recoil movement settings). 
        RecoilServo->set_Reversed(eeprom.ramcopy.RecoilReversed);                       // motor class method
        RecoilServo->setRecoilReversed(SERVONUM_RECOIL, eeprom.ramcopy.RecoilReversed); // servo class method
        // The begin function will make sure the recoil servo is initialized to its "battery" position
        RecoilServo->begin();

    // HENG LONG/OTHER SMOKER OUTPUT MOTOR DEFINITION
    // -------------------------------------------------------------------------------------------------------------------------------------->>
    	// There is only one smoker output, we use SIDEA. It does not do reverse, so min speed is always 0 (not negative). Max speed can be set by user. 
        // In Auto mode, the smoker object has three settings specified by the user: idle, fast idle and max speed. Those are also passed to the constructor. 
        if (eeprom.ramcopy.SmokerControlAuto)
        {   // If the user wants the smoker to be controlled automatically with engine speed, we pass the user's max speed setting
            Smoker = new OP_Smoker(0, MOTOR_MAX_FWDSPEED, 0, eeprom.ramcopy.SmokerIdleSpeed, eeprom.ramcopy.SmokerFastIdleSpeed, eeprom.ramcopy.SmokerMaxSpeed, eeprom.ramcopy.SmokerHeatIdleAmt, eeprom.ramcopy.SmokerHeatFastIdleAmt, eeprom.ramcopy.SmokerHeatMaxAmt, &MotorSerial, &eeprom.ramcopy.MotorSerialBaud, eeprom.ramcopy.SmokerDeviceType, eeprom.ramcopy.SmokerPreHeat_Sec);
        }
        else
        {   // But if the user wants to disconnect the smoker from the engine and control it manually, we set max speed to MOTOR_MAX_FWDSPEED (100%)
            // We still pass the Idle and FastIdle speeds but they will not be used. 
            Smoker = new OP_Smoker(0, MOTOR_MAX_FWDSPEED, 0, eeprom.ramcopy.SmokerIdleSpeed, eeprom.ramcopy.SmokerFastIdleSpeed, eeprom.ramcopy.SmokerHeatIdleAmt, eeprom.ramcopy.SmokerHeatFastIdleAmt, eeprom.ramcopy.SmokerHeatMaxAmt, MOTOR_MAX_FWDSPEED, &MotorSerial, &eeprom.ramcopy.MotorSerialBaud, eeprom.ramcopy.SmokerDeviceType, eeprom.ramcopy.SmokerPreHeat_Sec);
        }
        Smoker->begin();
        // Also save a local copy of this variable so the user can alter it on the fly and yet still restore it from the eeprom.ramcopy version
        SmokerPreHeat_Sec = eeprom.ramcopy.SmokerPreHeat_Sec;

    // OPTIONAL ONBOARD MOTOR CONTROL
    // -------------------------------------------------------------------------------------------------------------------------------------->>
        // If either of the two onboard motor controllers (A & B) have not been assigned to any drive or turret function, we let the user
        // control them directly for whatever purpose, using an analog function trigger. We also can conveniently set the external speed range of the
        // motor objects to the actual range of the input we know we will receive from an analog trigger (0-1023 where 511 is center). 
        if (MotorA_Available) { MotorA = new Onboard_ESC (SIDEA, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); MotorA->begin(); }
        if (MotorB_Available) { MotorB = new Onboard_ESC (SIDEB, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); MotorB->begin(); }
        //DebugSerial->print(F("Motor A Available: ")); PrintLnTrueFalse(MotorA_Available);
        //DebugSerial->print(F("Motor B Available: ")); PrintLnTrueFalse(MotorB_Available);
}


// OPTIONAL RC OUTPUTS
// -------------------------------------------------------------------------------------------------------------------------------------->>
void InstantiateOptionalServoOutputs()
{
    // Temp vars
    boolean RCOutput1_Assigned = false;
    boolean RCOutput2_Assigned = false;
    boolean RCOutput3_Assigned = false;
    boolean RCOutput4_Assigned = false;
    boolean RCOutput6_Assigned = false;
    boolean RCOutput7_Assigned = false;
    boolean RCOutput8_Assigned = false;
    
    // If any of the first four RC outputs are unused by drive, turret or steering, we can set them to generic RC outputs .
    // Also depending on the sound card the user selects, RC outputs 6, 7, 8 may also be available. 
    // Freeing them from these tasks allows the user to control them directly for whatever purpose. They essentially "pass-through" whatever 
    // radio channel has been assigned to them. 
    // In OP Config the user has the option of doing regular RC passthrough (good for servos, continuous rotation servos, or ESCs), or they 
    // can select a Pan Servo passthrough. Which one they picked will influence what kind of object we need to create. The only way to know
    // is to run through the list of the user's function triggers and see. And they may not have any at all, in which case we don't need
    // to create anything. 

    // It is also technically possible for the user to assign more than one trigger to the same RC pass-through. This would be bad news, so
    // in this routine we make sure that no more than one trigger is assigned to an RC output.

    // Only check if one of these is even available for use

    // This has ballooned into something ridiculous in terms of the coding. But I don't have time now to fancify and shorten it. 
    if (RCOutput1_Available || RCOutput2_Available || RCOutput3_Available || RCOutput4_Available || RCOutput6_Available || RCOutput7_Available || RCOutput8_Available)
    {   // Loop through every function-trigger pair
        for (int i = 0; i <MAX_FUNCTION_TRIGGERS; i++)
        {   // A valid function-trigger will have a function number and a TriggerID > 0
            if (eeprom.ramcopy.SF_Trigger[i].specialFunction != SF_NULL_FUNCTION && eeprom.ramcopy.SF_Trigger[i].TriggerID > 0)
            {
                switch (eeprom.ramcopy.SF_Trigger[i].specialFunction)
                {
                    case SF_RC1_PASS:       // RC Output 1 passthrough
                        if (RCOutput1_Available && !RCOutput1_Assigned) 
                        {   //RC Output on LEFTTREAD
                            RCOutput1 = new Servo_ESC (SERVONUM_LEFTTREAD, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            RCOutput1->begin(); 
                            RCOutput1_Assigned = true;
                        }
                        break;
                    case SF_RC2_PASS:       // RC Output 2 passthrough
                        if (RCOutput2_Available && !RCOutput2_Assigned) 
                        {   //RC Output on RIGHTTREAD
                            RCOutput2 = new Servo_ESC (SERVONUM_RIGHTTREAD, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            RCOutput2->begin(); 
                            RCOutput2_Assigned = true;
                        }
                        break;
                    case SF_RC3_PASS:       // RC Output 3 passthrough
                        if (RCOutput3_Available && !RCOutput3_Assigned) 
                        {   //RC Output on TURRETROTATION
                            RCOutput3 = new Servo_ESC (SERVONUM_TURRETROTATION, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            RCOutput3->begin(); 
                            RCOutput3_Assigned = true;
                        }
                        break;
                    case SF_RC4_PASS:       // RC Output 4 passthrough
                        if (RCOutput4_Available && !RCOutput4_Assigned) 
                        {   //RC Output on TURRETELEVATION
                            RCOutput4 = new Servo_ESC (SERVONUM_TURRETELEVATION, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            RCOutput4->begin(); 
                            RCOutput4_Assigned = true;
                        }
                        break;
                    case SF_RC6_PASS:       // RC Output 6 passthrough
                        if (RCOutput6_Available && !RCOutput6_Assigned) 
                        {   //RC Output on TURRETELEVATION
                            RCOutput6 = new Servo_ESC (SERVONUM_PROP3, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            RCOutput6->begin(); 
                            RCOutput6_Assigned = true;
                        }
                        break;
                    case SF_RC7_PASS:       // RC Output 7 passthrough
                        if (RCOutput7_Available && !RCOutput7_Assigned) 
                        {   //RC Output on TURRETELEVATION
                            RCOutput7 = new Servo_ESC (SERVONUM_PROP2, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            RCOutput7->begin(); 
                            RCOutput7_Assigned = true;
                        }
                        break;
                    case SF_RC8_PASS:       // RC Output 8 passthrough
                        if (RCOutput8_Available && !RCOutput8_Assigned) 
                        {   //RC Output on TURRETELEVATION
                            RCOutput8 = new Servo_ESC (SERVONUM_PROP1, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            RCOutput8->begin(); 
                            RCOutput8_Assigned = true;
                        }
                        break;
                                                
                    case SF_RC1_PASS_PAN:   // Pan Servo on RC Output 1
                        if (RCOutput1_Available && !RCOutput1_Assigned) 
                        {   // Servo_PAN on LEFTTTREAD
                            ServoOutput1 = new Servo_PAN (SERVONUM_LEFTTREAD, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            ServoOutput1->begin(); 
                            RCOutput1_Assigned = true;
                        }
                        break;
                    case SF_RC2_PASS_PAN:   // Pan Servo on RC Output 2
                        if (RCOutput2_Available && !RCOutput2_Assigned) 
                        {   // Servo_PAN on RIGHTTREAD
                            ServoOutput2 = new Servo_PAN (SERVONUM_RIGHTTREAD, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            ServoOutput2->begin(); 
                            RCOutput2_Assigned = true;
                        }
                        break;
                    case SF_RC3_PASS_PAN:   // Pan Servo on RC Output 3
                        if (RCOutput3_Available && !RCOutput3_Assigned) 
                        {   // Servo_PAN on TURRETROTATION
                            ServoOutput3 = new Servo_PAN (SERVONUM_TURRETROTATION, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            ServoOutput3->begin(); 
                            RCOutput3_Assigned = true;
                        }
                        break;
                    case SF_RC4_PASS_PAN:   // Pan Servo on RC Output 4
                        if (RCOutput4_Available && !RCOutput4_Assigned) 
                        {   // Servo_PAN on TURRETELEVATION
                            ServoOutput4 = new Servo_PAN (SERVONUM_TURRETELEVATION, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            ServoOutput4->begin(); 
                            RCOutput4_Assigned = true;
                        }
                        break;                                                        
                    case SF_RC6_PASS_PAN:   // Pan Servo on RC Output 6
                        if (RCOutput6_Available && !RCOutput6_Assigned) 
                        {   // Servo_PAN on TURRETELEVATION
                            ServoOutput6 = new Servo_PAN (SERVONUM_PROP3, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            ServoOutput6->begin(); 
                            RCOutput6_Assigned = true;
                        }
                        break;     
                    case SF_RC7_PASS_PAN:   // Pan Servo on RC Output 7
                        if (RCOutput7_Available && !RCOutput7_Assigned) 
                        {   // Servo_PAN on TURRETELEVATION
                            ServoOutput7 = new Servo_PAN (SERVONUM_PROP2, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            ServoOutput7->begin(); 
                            RCOutput7_Assigned = true;
                        }
                        break;     
                    case SF_RC8_PASS_PAN:   // Pan Servo on RC Output 8
                        if (RCOutput8_Available && !RCOutput8_Assigned) 
                        {   // Servo_PAN on TURRETELEVATION
                            ServoOutput8 = new Servo_PAN (SERVONUM_PROP1, ANALOG_SPECFUNCTION_MIN_VAL, ANALOG_SPECFUNCTION_MAX_VAL, ANALOG_SPECFUNCTION_CENTER_VAL); 
                            ServoOutput8->begin(); 
                            RCOutput8_Assigned = true;
                        }
                        break;                                                     
                }
            }
        }
    }
    //DebugSerial->print(F("RCOutput 1 Available: ")); PrintLnTrueFalse(RCOutput1_Available);
    //DebugSerial->print(F("RCOutput 2 Available: ")); PrintLnTrueFalse(RCOutput2_Available);
    //DebugSerial->print(F("RCOutput 3 Available: ")); PrintLnTrueFalse(RCOutput3_Available);
    //DebugSerial->print(F("RCOutput 4 Available: ")); PrintLnTrueFalse(RCOutput4_Available);
    //DebugSerial->print(F("RCOutput 6 Available: ")); PrintLnTrueFalse(RCOutput6_Available);
    //DebugSerial->print(F("RCOutput 7 Available: ")); PrintLnTrueFalse(RCOutput7_Available);
    //DebugSerial->print(F("RCOutput 8 Available: ")); PrintLnTrueFalse(RCOutput8_Available);
}


void SetupPins()
{
// These pins are defined in OP_Settings.h

// IO pins
    // These can be setup as input or output depending on the user's preference. 
    // IO "A" setup
    IO_Pin[IOA].Settings = eeprom.ramcopy.PortA_Settings;
    if (IO_Pin[IOA].Settings.dataDirection == OUTPUT)    
    {
        IO_Output[IOA].begin(pin_IO_A, false);      // False for n on-inverting
        IO_Pin[IOA].Settings.dataType ? IO_Output[IOA].on() : IO_Output[IOA].off();   // The default pin state is defined by the dataType setting
    }   
    else                                            
    {   // Use pullups if the input is "digital" (only needs to read on/off)
        // But if they want a full analog range, don't use pullups because it will prevent us from going all the way to ground (0)
        if (IO_Pin[IOA].Settings.dataType) pinMode(pin_IO_A, INPUT_PULLUP); 
        else pinMode(pin_IO_A, INPUT); 
        PortA_ReadValue();  // Get the present value
    }
    // IO "B" setup
    IO_Pin[IOB].Settings = eeprom.ramcopy.PortB_Settings;
    if (IO_Pin[IOB].Settings.dataDirection == OUTPUT)
    {
        IO_Output[IOB].begin(pin_IO_B, false);      // False for non-inverting
        IO_Pin[IOB].Settings.dataType ? IO_Output[IOB].on() : IO_Output[IOB].off();   // The default pin state is defined by the dataType setting
    }
    else                                            
    {   // Use pullups if the input is "digital" (only needs to read on/off)
        // But if they want a full analog range, don't use pullups because it will prevent us from going all the way to ground (0)
        if (IO_Pin[IOB].Settings.dataType) pinMode(pin_IO_B, INPUT_PULLUP);
        else pinMode(pin_IO_B, INPUT);        
        PortB_ReadValue(); // Get the present value
    }

// Voltage sensor
    pinMode(pin_BattVoltage, INPUT);
                                                
// Dipswitch pins
    // These are held to ground when On, but left floating when Off - so we use the input pullups to keep them high when Off
    pinMode(pin_Dip1, INPUT_PULLUP);        // Input    - Dipswitch 1
    pinMode(pin_Dip2, INPUT_PULLUP);        // Input    - Dipswitch 2
    pinMode(pin_Dip3, INPUT_PULLUP);        // Input    - Dipswitch 3
    pinMode(pin_Dip4, INPUT_PULLUP);        // Input    - Dipswitch 4
    pinMode(pin_Dip5, INPUT_PULLUP);        // Input    - Dipswitch 5

// Repair selection switch
    pinMode(pin_RepairTank, INPUT_PULLUP);  // Input    - Repair tank selection switch

// Pushbutton
    // Held to ground when pushed
    pinMode(pin_Button, INPUT_PULLUP);      // Input	- Pushbutton input

// Board LEDs
    pinMode(pin_RedLED, OUTPUT);            // Output   - Red   LED on board
    pinMode(pin_GreenLED, OUTPUT);	        // Output	- Green LED on board
    // Initialize all outputs to Off, EXCEPT for the Red LED, which we leave
    // on until the board has finished with all the startup routines
    GreenLedOff();
    RedLedOn();

// Transistorized Light outputs
    pinMode(pin_Light1, OUTPUT);	        // Output   - Light 1 output (Headlights)
    pinMode(pin_Light2, OUTPUT);            // Output   - Light 2 output
    analogWrite(pin_Light2, 1);
    pinMode(pin_Brakelights, OUTPUT);	    // Output   - Brake light output. PWM capable.
    pinMode(pin_HitNotifyLEDs, OUTPUT);	    // Output   - Hit notification LEDs if using the Tamiya apple
    // This one is PNP, so logic high is off
    digitalWrite(pin_MuzzleFlash, HIGH);
    pinMode(pin_MuzzleFlash, OUTPUT);	    // Output	- Trigger output for Taigen High Intensity muzzle flash unit
    // Machine gun LED has to be manipulated directly, we can't use the Arduino functions
    MG_DDR |= (1 << MG_PORTPIN);            // Output   - Machine gun - set the port pin to output ("or" that bit of the data direction register with a 1)

    // Initialize all these outputs to off
    // These are NPN MOSFETs, logic low is off
    digitalWrite(pin_Light1, LOW);
    digitalWrite(pin_Light2, LOW);
    digitalWrite(pin_Brakelights, LOW);
    digitalWrite(pin_HitNotifyLEDs, LOW);
    MG_PORT &= ~(1 << MG_PORTPIN);          // Machine Gun LED must be set directly. We "and-not" the port pin bit with 1 to set it to 0, this turns it off

    // Aux Output
    pinMode(pin_AuxOutput, OUTPUT);         // Output   - Aux output. PWM capable. Has flyback diode, can drive a relay directly or a (very small) motor 
    // The default state (high or low) depends on the presence or absence of certain functions.
    // If the inverse flash function is assigned, we want the default value to be High, but in all other cases we want the default to be Low
    // NOTE: If you setup Timer 4 after this statement, rather than before, the HIGH setting won't be saved. 
    if (isFunctionAssigned(SF_AUXOUT_INV_FLASH))    digitalWrite(pin_AuxOutput, HIGH); 
    else                                            digitalWrite(pin_AuxOutput, LOW);       

    // Onboard motorized outputs
    pinMode(OB_MA_PWM, OUTPUT);             // Motor A
    pinMode(OB_MB_PWM, OUTPUT);             // Motor B
    pinMode(OB_SMOKER_PWM, OUTPUT);         // Smoker
    
    // Mechanical Recoil Trigger
    pinMode(pin_MechRecoilMotor, OUTPUT);   // Output   - Transistor for Asiatam, Tamiya or similar mechanical recoil units
    // Also NPN, logic low is off
    digitalWrite(pin_MechRecoilMotor, LOW);
    
}







