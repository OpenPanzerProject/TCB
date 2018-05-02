
// RETURN SETTINGS FROM BOARD SWITCHES
// -------------------------------------------------------------------------------------------------------------------------------------------------->


// FIGHT/REPAIR SWITCH: Is this tank a repair tank? 
// -------------------------------------------------------------------------------------------------------------------------------------------------->
boolean GetIsRepair(void)
{   // Repair tank setting. If LOW (held to ground), tank is fighter. If HIGH (through input pullup), tank is repair. 
    // But the actual pin read is done in the OP_Tank class.
    return Tank.isRepairTank();
}

// DIPSWITCH 1 & 2: TANK WEIGHT CLASS
// -------------------------------------------------------------------------------------------------------------------------------------------------->
char GetWeightClass(void)
{   
    // Weight classes are selected by dipswitches 1 & 2
    // Both on:      CUSTOM - set by user
    // 1 on,  2 off: Light
    // 1 off, 2 on:  Medium
    // Both off:     Heavy

    if (DipSwitchOn(1) && DipSwitchOn(2))           return WC_CUSTOM;
    else if (DipSwitchOn(1) && DipSwitchOff(2))     return WC_LIGHT;
    else if (DipSwitchOff(1) && DipSwitchOn(2))     return WC_MEDIUM;
    else if (DipSwitchOff(1) && DipSwitchOff(2))    return WC_HEAVY;
}

// DIPSWITCH 3-4: MENU SELECTION
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// Four menus can be selected by dip-switches. After setting the dipswitch, enter the menu by pressing the input button for 2 seconds. 
uint8_t GetMenuNumber(void)
{
    if (DipSwitchOn(3) && DipSwitchOn(4))           return 1;       // Barrel elevation pan servo setup
    else if (DipSwitchOn(3) && DipSwitchOff(4))     return 2;       // Steering servo for cars/halftracks
    else if (DipSwitchOff(3) && DipSwitchOn(4))     return 3;       // Turret rotation servo setup
    else if (DipSwitchOff(3) && DipSwitchOff(4))    return 4;       // Recoil servo setup
}

// DIPSWITCH 5 - PC Communication Serial Port
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// If dipswitch 5 is ON, communication with the PC will be through the USB port. If it is OFF, communication will be through Aux Serial (Serial 1).
// We can attach a Bluetooth module to Serial 1 and therefore update settings from OP Config wirelessly. 
// NOTE: This port is only used for reading/writing settings. It is NOT used for *flashing* new firmware to the board - that always goes through the USB port.
// NOTE: Remember that the USB port and Aux Serial will show up as different COM ports on your computer.
// NOTE: The baud rate for the Aux Serial port defaults to 115,200 but you can change it on the Misc tab of OP Config. Baud rate for USB will always be 115200. 
boolean UseAuxSerialForPCComm(void)
{   
    return DipSwitchOff(5);
}

void SetActiveCommPort()
{
    static uint8_t useAlternate = 0xAA;
    static uint32_t lastTime = millis();

    // The local static variable useAlternate keeps track of the current setting.
    // But when we call this function, we check the current setting against the actual position of Dipswitch #5. 
    // If the current setting doesn't match, the user made a change, so update the current setting, and
    // adjust the pointer to the serial port for both Debug output and the PCComm class.

    // We haven't yet initialized - do so now. This will only get run once
    if (useAlternate == 0xAA)
    {
        if (UseAuxSerialForPCComm())  
        {                             
            PCComm.switchToAltSerial();     // Use Serial 1 for PC comm
            DebugSerial = &Serial1;         // Use Serial 1 for debugging messages
            useAlternate = 0xFF;
        }
        else 
        {
            PCComm.revertToDefaultSerial(); // Use Serial 0 for PC comm
            DebugSerial = &Serial;          // Use Serial 0 for debugging messages
            useAlternate = 0x00;
        }
    }

    // This will get run every other time this function is called, which will be once per loop
    // It will only change the serial port of the switch has changed since last time, and only
    // if a certain amount of time has passed since the last check (switch debounce)
    if (millis() - lastTime > 50)   // 50ms debounce
    {
        lastTime = millis();
        if (UseAuxSerialForPCComm() && useAlternate == 0x00)
        {
            PCComm.switchToAltSerial();     // Use Serial 1 for PC comm
            DebugSerial = &Serial1;         // Use Serial 1 for debugging messages
            if (DEBUG) DebugSerial->println(F("Comms changed to Serial 1"));        
            useAlternate = 0xFF;
        }
        else if (!UseAuxSerialForPCComm() && useAlternate == 0xFF)
        {
            PCComm.revertToDefaultSerial(); // Use Serial 0 for PC comm
            DebugSerial = &Serial;          // Use Serial 0 for debugging messages
            if (DEBUG) DebugSerial->println(F("Comms changed to USB"));        
            useAlternate = 0x00;
        }
    }
}

// BASIC DIPSWITCH UTILITIES
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// The dipswitches are held to ground when "ON"
// Physically these are left floating when "OFF", but we have input pullups turned on, so they are actually high. 
// In short, low is ON and high is OFF.
boolean DipSwitchOn(uint8_t switchNum)
{   
    return !DipSwitchOff(switchNum);    // If it's off, it's not on
}

boolean DipSwitchOff(uint8_t switchNum)
{   
    switch (switchNum)
    {
        case 1:     return digitalRead(pin_Dip1);   break;
        case 2:     return digitalRead(pin_Dip2);   break;
        case 3:     return digitalRead(pin_Dip3);   break;
        case 4:     return digitalRead(pin_Dip4);   break;
        case 5:     return digitalRead(pin_Dip5);   break;
        default:    return false;
    }        
}

