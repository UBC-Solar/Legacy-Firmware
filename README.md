This repository contains embedded code (mainly Arduino) that runs various subsystems in our solar vehicle.

Installation / Setup Instructions

1. Clone this repository into a suitable location.
2. Run `git submodule init && git submodule update` to pull all the libraries.
3. Open Arduino. Go to File > Preferences and set the root folder of this repo as the sketchbook location.

At this point, you should be able to use the Arduino IDE to run any of the firmware sketches. Any necessary custom libraries should be available in the `libraries` folder and the Arduino IDE should automatically detect them.

# How to Configure PlatformIO for Building Firmware

## Download Stuff
You'll need to download Atom and PlatformIO. If you don't already have the Firmware repo, clone it. We'll refer to this directory as "Firmware" from now on and assume that it is in your current working directory.

## Configure Your Project
1. The default configurations for PlatformIO are fine
1. Feel free to configure the editor however you want
1. __Do not import the Firmware project with the wizard__
1. Make a project directory manually, let's call it "solar"
	* `mkdir solar`
1. Put the entire Firmware repo in solar
	* `mv Firmware solar/Firmware`
1. Create a file in solar called platformio.ini
	* `touch solar/platformio.ini`
1. The tree will now look like:
```
* solar/
	* Firmware/
		* All the code directories...
	* platformio.ini
```
1. Copy the following information into your platoformio.ini file
```
[env:uno]
platform = atmelavr
board = uno
framework = arduino

[platformio]
src_dir = Firmware
lib_dir = Firmware/libraries
```
1. Now you can open "solar" as a project in platformIO
1. For any project you want to build, consider a main file named "main.ino"
	1. Create a symlink main.cpp that points to main.ino, in the same directory
		* `ln -s main.ino main.cpp`
	1. Add `#include <Arduino.h>` to the includes section of that file
	1. PlatformIO should be able to build this project now

## Building
If you've configured everything correctly, you should be able to just open
the main file for the project you want to build, and use the build menu to compile
your project.

## Uploading and Serial
Uploading and use of the serial monitor haven't been explored as of yet, but 
will "just work" right?

Known issues with uploading involve configuring permissions to access ports.
