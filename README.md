![Open Panzer](http://www.openpanzer.org/images/github/openpanzer2.png)
# The TCB (Tank Control Board)
Open Panzer is a project to develop open source products for the RC tank market. The TCB is a highly capable control board designed specifically for RC model tanks, but also suitable for halftracks, cars, or even battleships. 

## Resources
For more information on the Open Panzer project, see the [OpenPanzer Wiki](http://www.openpanzer.org/wiki).  
To discuss the project, feel free to join the [Open Panzer Community](http://www.openpanzer.org/forum/). 

## General Users
If you don't intend to modify the source code or care about any of the nitty gritty details, that's fine. And you don't even need to be here! Instead go to the [OpenPanzer Wiki](http://www.openpanzer.org/wiki), learn how to get your own TCB board, [download](http://www.openpanzer.org/downloads) the OP Config Windows desktop program, and start tanking! 

## Developers
The processor used on the TCB is an ATmega 2560 and the firmware can be compiled in the Arduino IDE. If you want to modify the source code, either clone the TCB repository if you are familiar with Git, or simply click the **Download ZIP** button at the top right of this page. Extract the ZIP file to your computer. Inside the ZIP file enter the `TCB-master` folder and copy the **libraries** and **OpenPanzerTCB** folders to your Arduino Sketches directory. You may already have a libraries folder in your sketches directory - if so, that's fine, just overwrite it (this will simply add the new libraries). 

Then open `xxx\Sketches\OpenPanzerTCB\OpenPanzerTCB.ino` with the Arduino IDE to see the sketch code. 

To compile or upload the code from the Arduino IDE, go to the Tools menu and make sure the Board is set to "Arduino Mega or Mega 2560" and make sure the Processor is set to "ATmega2560 (Mega 2560)". 

Most of the functionality actually resides in the many C++ libraries. These will be in your `Sketches\libraries\` folder and they will all begin with the prefix **OP_**. 

## License
Firmware for the TCB, as well as the source files for OP Config (the desktop configuration program) are free software: you can redistribute them and/or modify them under the terms of the GNU General Public License version 3 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

For more specific details see [http://www.gnu.org/licenses](http://www.gnu.org/licenses), the [Quick Guide to GPLv3.](http://www.gnu.org/licenses/quick-guide-gplv3.html) and the [copying.txt](https://github.com/OpenPanzerProject/TCB/blob/master/COPYING.txt) file in the codebase.

The GNU operating system which is under the same license has an informative [FAQ here](http://www.gnu.org/licenses/gpl-faq.html).
