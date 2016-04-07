## What Is In This Folder? 

This folder contains the custom bootloader used on the TCB - both a precompiled hex, and the source code. The bootloader is a small piece of code that remains on the TCB and allows the main program (firmware) to be rewritten easily using OP Config or the Arduino IDE without any special equipment other than USB cable. 

## Do I Need to Do Anything With This? 

Probably not, the bootloader should already come installed on your TCB from the factory. But if you are a developer, are curious, or for some reason need to re-write the bootloader on your device, read one. 

## Bootloader Details

This bootloader is a modification of the standard Arduino stk500boot bootloader used for the Mega boards. It was first modified by [Arduino Forum user Krupski](http://forum.arduino.cc/index.php?topic=309467.0) who removed the bulky and largely-unused Monitor utility and reduced the overall size of the bootloader to roughly 2kB. 

The Open Panzer project subsequently modified it further for our own purposes. The primary change is the ability to select which serial port you want to flash the chip when writing new firmware. This is done by the position of Dipswitch #5 on the TCB. 

When Dipswitch #5 is in the On position, the bootloader will listen on Serial 0, which is the onboard USB port on the TCB. 

When Dipswitch #5 is in the Off position the bootloader will listen on Serial 1, which is broken out as a six-pin header on the TCB. This header is fully compatible with typical FTDI adapters or cables, or most conveniently of all, the [Adafruit Bluefruit EZ-Link](https://www.adafruit.com/product/1588). This allows wireless flashing of the firmware on the TCB. 

**Note 1:** If you are having problems flashing new firmware to your board, make sure you have Dipswitch #5 set in the correct position! 

**Note 2:** If you have the Adafruit bluetooth device, or some other FTDI device plugged into the Serial 1 connector, and you try to re-flash the board over USB, you may still find it doesn't work even if you have Dipswitch #5 in the correct position (On for USB). This is because these other devices can prevent the board from responding to the reset command sent over the USB port. If you manually hold the RESET button on the TCB, start the firmware update, then release the RESET button, it should work. Alternatively, unplug the device from Serial 1, or if you don't want to do that -  just set Dipswitch #5 to the off position and flash over Serial 1!

## Re-Compiling the Bootloader

To re-compile the bootloader you will need avr-gcc. A version previously came bundled with the Arduino IDE but does no longer. If you are developing in Windows, download the latest version of [WinAVR](https://sourceforge.net/projects/winavr/) and make sure it creates the correct System Environment path variables when you install it. 

Then open a command prompt at the folder where you have saved these bootloader files. Type the following two commands: 
```
prompt> make clean
prompt> make optcb2560
```

The first command will clear the old hex, the second command will compile a new one. 


## Re-Flashing the Bootloader
To flash the bootloader to your chip you will need a special programmer device. The USBasp is a good one and available for only a few bucks all over eBay. The special programmer will come with a dual-row, six pin plug. The TCB has a matching six-pin header footprint that is not populated from the factory, you will have to solder headers to it or use something like the [SparkFun ISP Pogo Adapter](https://www.sparkfun.com/products/11591). 

Plug your programmer into a USB port on your computer. Plug the ISP cable into the TCB. The programmer will not supply power to the TCB so you will also need to connect a battery. 

Then open a command prompt and browse to the following folder in the Arduino installation directory:
`Arduino_Dir\hardware\tools\avr\bin\`

And run the following commands: 



