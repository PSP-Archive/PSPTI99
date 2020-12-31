
    Welcome to PSPTI99

Original Author of TI99/Sim

  Marc Rousseau  (see http://www.mrousseau.org/programs/ti99sim/)

Author of the PSP port version 

  Ludovic.Jacomme also known as Zx-81 (see http://zx81.zx81.free.fr)


1. INTRODUCTION
   ------------

  TI99Sim is a very good emulator of Texas instruments TI99 home computer
  running on Windows and Unix. The emulator faithfully imitates the TI99/4
  model (see http://www.mrousseau.org/programs/ti99sim/)

  PSPTI99 is a port on PSP of the version 0.1.0 of TI99Sim.

  Special thanks to Danzel and Jeff Chen for their virtual keyboard,
  and to all PSPSDK developpers.

  This package is under GPL Copyright, read COPYING file for
  more information about it.


2. INSTALLATION
   ------------

  Unzip the zip file, and copy the content of the directory fw3.x or fw1.5
  (depending of the version of your firmware) on the psp/game, psp/game150,
  or psp/game3xx if you use custom firmware 3.xx-OE.

  Put your cartridge images generated using software available on 
  http://www.mrousseau.org/programs/ti99sim/ in the cartridges folder.

  It has been developped on linux for Firmware 3.71-m33 and i hope it works
  also for other firmwares.

  For any comments or questions on this version, please visit 
  http://zx81.zx81.free.fr or http://zx81.dcemu.co.uk


3. CONTROL
   ------------

3.1 - Virtual keyboard

  In the TI99 emulator window, there are three different mapping 
  (standard, left trigger, and right Trigger mappings). 
  You can toggle between while playing inside the emulator using 
  the two PSP trigger keys.

  -------------------------------------
  PSP        TI99            (standard)

  Square     Space
  Triangle   ENTER
  Circle     1
  Cross      Joystick Fire
  Up         Up
  Down       Down
  Left       Left 
  Right      Right

  Analog     Joystick

  -------------------------------------
  PSP        TI99   (left trigger)

  Square     Fps 
  Triangle   Load state
  Circle     Joystick
  Cross      Save state   
  Up         Up
  Down       Down
  Left       Render
  Right      Render

  -------------------------------------
  PSP        TI99   (right trigger)

  Square     Space
  Triangle   Reset
  Circle     4
  Cross      Auto-fire
  Up         Up
  Down       Down
  Left       Dec-fire
  Right      Inc-fire

  Analog     Joystick
  
  Press Start  + L + R   to exit and return to eloader.
  Press Select           to enter in emulator main menu.
  Press Start            open/close the On-Screen keyboard

  In the main menu

    RTrigger   Reset the emulator

    Triangle   Go Up directory
    Cross      Valid
    Circle     Valid
    Square     Go Back to the emulator window

  The On-Screen Keyboard of "Danzel" and "Jeff Chen"

    Use Analog stick to choose one of the 9 squares, and
    use Triangle, Square, Cross and Circle to choose one
    of the 4 letters of the highlighted square.

    Use LTrigger and RTrigger to see other 9 squares 
    figures.

3.2 - IR keyboard

  You can also use IR keyboard. Edit the pspirkeyb.ini file
  to specify your IR keyboard model, and modify eventually
  layout keyboard files in the keymap directory.

  The following mapping is done :

  IR-keyboard   PSP

  Cursor        Digital Pad

  Tab           Start
  Ctrl-W        Start

  Escape        Select
  Ctrl-Q        Select

  Ctrl-E        Triangle
  Ctrl-X        Cross
  Ctrl-S        Square
  Ctrl-F        Circle
  Ctrl-Z        L-trigger
  Ctrl-C        R-trigger

  In the emulator window you can use the IR keyboard to
  enter letters, special characters and digits.


4. LOADING KEY MAPPING FILES
   ------------

  For given games, the default keyboard mapping between PSP Keys and TI99 keys,
  is not suitable, and the game can't be played on PSPTI99.

  To overcome the issue, you can write your own mapping file. Using notepad for
  example you can edit a file with the .kbd extension and put it in the kbd 
  directory.

  For the exact syntax of those mapping files, have a look on sample files already
  presents in the kbd directory (default.kbd etc ...).

  After writting such keyboard mapping file, you can load them using 
  the main menu inside the emulator.

  If the keyboard filename is the same as the cartridge file (.ctg)
  then when you load this tape file, the corresponding keyboard file is 
  automatically loaded !

  You can now use the Keyboard menu and edit, load and save your
  keyboard mapping files inside the emulator. The Save option save the .kbd
  file in the kbd directory using the "Game Name" as filename. The game name
  is displayed on the right corner in the emulator menu.

  
7. COMPILATION
   ------------

  It has been developped under Linux using gcc with PSPSDK. 
  To rebuild the homebrew run the Makefile in the src archive.

