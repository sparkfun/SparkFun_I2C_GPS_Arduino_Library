SparkFun MediaTek I2C Interface Library
===========================================================

[![SparkFun GPS Breakout - XA1110 (Qwiic)](https://cdn.sparkfun.com//assets/parts/1/2/3/4/0/14414-SparkFun_GPS_Breakout_-_XA1110__Qwiic_-01.jpg)](https://www.sparkfun.com/products/14414)

[*SparkFun GPS Breakout - XA1110 (Qwiic) (SEN-14414)*](https://www.sparkfun.com/products/14414)

The Qwiic GPS uses the Titan X1 GPS module with special I2C firmware. This library talks to the GPS module over I2C.

For Arduino we recommend using the [TinyGPS++ library](https://github.com/mikalhart/TinyGPSPlus) in addition to this library. Works like a champ together.

For mBed-OS use [codygrays port of the TinyGPS++ library](https://github.com/codygray/TinyGPSPlus/tree/universal) instead.

Library written by Nathan Seidle ([SparkFun](http://www.sparkfun.com)).

Repository Contents
-------------------

* **/examples** - Example sketches for the library (.ino). Run these from the Arduino IDE. 
* **/src** - Source files for the library (.cpp, .h). These are hybridized versions that work under Arduino and mBed as well.
* **keywords.txt** - Keywords from this library that will be highlighted in the Arduino IDE. 
* **library.properties** - General library properties for the Arduino package manager.
* **main.cpp** - Basic example code for mBed-OS.
* **externs.h** - Needed by mBed-OS to let .h and .cpp files use the same I2C and Serial definition.

Documentation
--------------

* **[Installing an Arduino Library Guide](https://learn.sparkfun.com/tutorials/installing-an-arduino-library)** - Basic information on how to install an Arduino library.
* **[Product Repository](https://github.com/sparkfunx/Qwiic_GPS-TitanX1)** - Main repository (including hardware files) for the Titan X1 Qwiic breakout board.

License Information
-------------------

This product is _**open source**_! 

Various bits of the code have different licenses applied. Anything SparkFun wrote is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round! Anything Maxim wrote has its own license. Anything that was co-writing with Peter Jansen is BSD.

Please use, reuse, and modify these files as you see fit. Please maintain attribution to SparkFun Electronics and release anything derivative under the same license.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
