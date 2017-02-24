![Open Panzer](http://www.openpanzer.org/images/github/tcb_intro_git.jpg)
# Introduction
  * [Open Panzer](http://www.openpanzer.org) is a community project to develop open source products for the RC tank market. 
  * The TCB (tank control board) is a highly capable circuit board designed specifically for RC model tanks, but also suitable for halftracks, cars, or even battleships. The hardware is based on an ATmega 2560 processor and the firmware consists of an Arduino sketch as well as various C++ libraries. The repository you are in now contains the TCB source code. 
  * OP Config is a desktop application used to configure the many options on the TCB and to simplify flashing new firmware to the TCB. It was written in C++ using the open source application development software called ["Qt"](http://www.qt.io/developers/). It has its own firmware repository, [see here](https://github.com/OpenPanzerProject/OP-Config)

## Resources
  * For the TCB hardware files and bill of materials, see the [Open Panzer Downloads page](http://openpanzer.org/downloads).
  * For more information on the Open Panzer project, see the [OpenPanzer Wiki](http://wiki.openpanzer.org).
  * To discuss the project, feel free to join the [Open Panzer Community](http://openpanzer.org/forum/index.php?action=forum).

## General Users
If you are not a developer, you probably don't need to worry about anything on this site. Instead [get your own TCB board](http://openpanzer.org/buy), [download the OP Config Windows desktop program](http://www.openpanzer.org/downloads), check out the [OpenPanzer Wiki](http://wiki.openpanzer.org) for instructions, and have fun tanking! 

## Developers
The processor used on the TCB is an ATmega 2560 and the firmware can be compiled in the Arduino IDE. If you want to modify the source code you can either A) clone the TCB repository if you are familiar with Git, or B) click the green **Clone or download** button at the top of this page, then select **Download ZIP**. 

![Download ZIP](http://openpanzer.org/images/github/CloneOrDownloadTCB.png "Download ZIP")

Unzip the file on your computer. Inside the ZIP file enter the `TCB-master` folder and:
  * Copy the **`libraries`** and **`OpenPanzerTCB`** folders to your Arduino Sketchbook directory. If you don't know your sketchbook folder you can view its location in the Arduino IDE by going to **File > Preferences > Sketchbook Location**. You may already have a `libraries` folder in your sketchbook directory - if so, that's fine, just overwrite it (this will simply add the new libraries). 
  * That gets you the source code, but we also need to install the board definition files in the Arduino IDE. This is a simple process. Open the Arduino IDE and go to **File > Preferences** (or if using OS X, go to Arduino > Preferences). A window will appear like the one shown below: 
![Add JSON to Preferences](http://openpanzer.org/images/github/boards/Preferences_JSON.png "Add JSON to Preferences")
  * Paste the following URL to the 'Additional Boards Manager URLs' input field: 
`https://openpanzerproject.github.io/OpenPanzerBoards/package_openpanzer_index.json`
  * If there are already other entries in that field just add the link at the end separated from the others with a comma. 
  * Next close the Preferences window, then go to the **Tools** menu and select **Board > Boards Manager**. Once the Board Manager opens, click on the category drop down menu on the top left hand side of the window and select **Contributed** - or, just type "Open Panzer" into the search bar. When you find the Open Panzer Boards option, click on the **Install** button and wait for the process to complete. 
  ![Boards Manager](http://openpanzer.org/images/github/boards/BoardsManager.png "Boards Manager")
  * Finally, **quit and reopen the Arduino IDE** to ensure the new board packages are properly installed. You should now be able to select the new board listed in the **Tools->Board** menu.
  ![TCB Board shown in List](http://openpanzer.org/images/github/boards/BoardList.png "TCB Board shown in List")
  * Now you are reading to start coding!
   
## Helpful Tip for Coders
If you are running Windows, it is highly recommended you install Arduino to a clean folder like `C:\Arduino` rather than the default location which is something like `C:\Program Files (x86)\` - which has spaces and parentheses in the file name. As a developer this can save you much grief later. 

In addition to the various tabs in the sketch, most of the functionality actually resides in the many C++ libraries. These will be in your `Sketches\libraries\` folder and they will all begin with the prefix **OP_**. See the [Libraries Reference](http://openpanzer.org/wiki/doku.php?id=wiki:devl:libref) page in the Wiki for a brief explanation of each one. 

## License
Firmware for the TCB is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

For more specific details see [http://www.gnu.org/licenses](http://www.gnu.org/licenses), the [Quick Guide to GPLv3.](http://www.gnu.org/licenses/quick-guide-gplv3.html) and the [copying.txt](https://github.com/OpenPanzerProject/TCB/blob/master/COPYING.txt) file in the codebase.

The GNU operating system which is under the same license has an informative [FAQ here](http://www.gnu.org/licenses/gpl-faq.html).
