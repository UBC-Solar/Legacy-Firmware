This repository contains embedded code (mainly Arduino) that runs various subsystems in our solar vehicle.

Installation / Setup Instructions

1. Clone this repository into a suitable location.
2. Run `git submodule init && git submodule update` to pull all the libraries.
3. Open Arduino. Go to File > Preferences and set the root folder of this repo as the sketchbook location.

At this point, you should be able to use the Arduino IDE to run any of the firmware sketches. Any necessary custom libraries should be available in the `libraries` folder and the Arduino IDE should automatically detect them.
