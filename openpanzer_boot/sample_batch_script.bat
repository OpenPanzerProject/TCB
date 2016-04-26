REM ----------------------------------------------------------------------------------------------------------->>
REM - This will burn a bootloader. First it will set the fuse bits and unlock the bootloader section, then it 
REM - will burn the bootloader and re-lock the bootloader section
REM ----------------------------------------------------------------------------------------------------------->>

REM - Do NOT use white space between the variable name, the equal sign, and the value! 


REM - FUSES - DON'T CHANGE THESE, THEY ARE CORRECT FOR THE OPEN PANZER TCB Mk 1
REM ----------------------------------------------------------------------------------------------------------->>
set EFUSE=0xFD
set HFUSE=0xDA
set LFUSE=0xF7
set ULOCKF=0x3F
set LOCKF=0x0F



REM - SET YOUR AVRDUDE AND CONF FILE PATHS
REM ----------------------------------------------------------------------------------------------------------->>
REM - Notice I've moved Arduino to the root of C:. Not having any spaces in the file name will save you infinite heartache. 
REM - You also for sure don't want any parentheses in the file name either, AVRDUDE will croak for sure if it sees those

set conf_file="C:\YOUR_ARDUINO_INSTALL_DIR\hardware\tools\avr\etc\avrdude.conf"

set avrdude_exe=C:\YOUR_ARDUINO_INSTALL_DIR\hardware\tools\avr\bin\avrdude.exe



REM - SET YOUR BOOTLOADER HEX PATH
REM ----------------------------------------------------------------------------------------------------------->>
set boot_hex="C:\WHEREVER_YOU_PUT_IT\openpanzer_boot\optcb2560_boot.hex"



REM - SET YOUR PROGRAMMER
REM ----------------------------------------------------------------------------------------------------------->>
REM - See http://www.nongnu.org/avrdude/user-manual/avrdude_4.html for a list of programmers, or type avrdude -c ?
REM - Use this for the USBasp. If using the USBasp, you can also ignore the warning message avrdude will give you about sck 
set programmer=usbasp           



REM - THIS WILL DO THE EXECUTING - DON'T CHANGE ANYTHING BELOW HERE
REM ----------------------------------------------------------------------------------------------------------->>
REM - The start cmd.exe /k lets us open the command prompt first and then leave it open so we can see if it worked. 
REM - First half: chip erase (-e), unlock boot section, write fuses. Next comes a "+" symbol to add the second half: 
REM - Second half: now we write the bootloader to flash, then finish by locking the boot section
start cmd.exe /k %avrdude_exe% -C %conf_file% -p atmega2560 -c %programmer% -P usb -v -e -U lock:w:%ULOCKF%:m -U efuse:w:%EFUSE%:m -U hfuse:w:%HFUSE%:m -U lfuse:w:%LFUSE%:m + %avrdude_exe% -C %conf_file% -p atmega2560 -c %programmer% -P usb -v -U flash:w:%boot_hex%:i -U lock:w:%LOCKF%:m 

