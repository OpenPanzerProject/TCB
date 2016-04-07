This folder contains the source code for the custom TCB bootloader, as well as a precompiled hex. 

This bootloader is a modification of the standard Arduino stk500boot bootloader used for the Mega boards. It was first modified by Arduino forum user Krupski who removed the Monitor utility and reduced the overall size to roughly 2kB. 

The Open Panzer project subsequently modified it further for our own purposes. The primary change is the ability to select over which serial port you want to flash the chip when writing new firmware. This is done by the position of Dipswitch #5 on the TCB. 

When Dipswitch #5 is in the On position, the bootloader will listen on Serial 0, which is the onboard USB port on the TCB. 

When Dipswitch #5 is in the Off position the bootloader will listen on Serial 1, which is broken out as a six-pin header on the TCB. This header is fully compatible with typical FTDI adapters or cables, or most conveniently of all, the [Adafruit Bluefruit EZ-Link](https://www.adafruit.com/product/1588). This allows wireless flashing of the firmware on the TCB. 

**Note 1:** If you are having problems flashing new firmware to your board, make sure you have Dipswitch #5 set in the correct position! 

**Note 2:** If you have the Adafruit bluetooth device, or some other FTDI device plugged into the Serial 1 connector, and you try to re-flash the board over USB, you may still find it doesn't work even if you have Dipswitch #5 in the correct position (On for USB). This is because these other devices can prevent the board from responding to the reset command sent over the USB port. If you manually hold the RESET button on the TCB, start the firmware update, then release the RESET button, it should work. Alternatively, unplug the device from Serial 1, or if you don't want to do that -  just set Dipswitch #5 to the off position and flash over Serial 1!

