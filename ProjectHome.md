This project is meant to put several QRP accessories into one box: a cw keyer, antenna tuner, and frequency counter. Having it all in one place decreases the chances that you will get to the top of a mountain and only then remember that you left some part of the station in your car. All of these functions are centered around an ATmega368 microprocessor.  Down the line, this project could be easily repurposed for integration within a transceiver.

In developing the firmware for this project, I decided that rather than invent the wheel, I'd start with code written by [K3NG](http://radioartisan.wordpress.com/arduino-cw-keyer/) for his cw keyer project. I hope he can forgive me for gutting his code and losing a lot of the capability of his keyer as I try to focus the feature set to what is needed for this specific project. The K3NG keyer code lives in a [repository at SourceForge](http://sourceforge.net/projects/k3ngarduinocwke/).

I will occasionally post about this project on my [blog](http://blog.templaro.com) or [GooglePlus feed](https://plus.google.com/107575177248113874358/posts).  I occasionally tweak an [early design document](https://docs.google.com/document/d/13Vf-5hEtwC-lTQcX9tBAL5b0oiqlLPKNp65mWQ7x7qU/edit), which is also maintained online.

I've collected several documents on this site including programs developed along the way to test individual subsystems, the schematic in Eagle format, and the firmware code itself ([sig\_other/sig\_other.ino](http://code.google.com/p/significant-other/source/browse/#hg%2Fsig_other)).