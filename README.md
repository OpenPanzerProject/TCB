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



