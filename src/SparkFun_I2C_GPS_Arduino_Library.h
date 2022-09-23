/*
  This is a library written for the MediaTek MT3333 based GPS module with I2C firmware
  specially loaded. SparkFun sells these at its website: www.sparkfun.com

  Written by Nathan Seidle @ SparkFun Electronics, April 25th, 2017

  This GPS module is special because it can use an I2C interface to communicate.

  This library handles the pulling in of data over I2C. We recommend a parsing engine
  such as TinyGPS.

  https://github.com/sparkfun/SparkFun_I2C_GPS_Arduino_Library

  Do you like this library? Help support SparkFun. Buy a board!

  Development environment specifics:
  Arduino IDE 1.8.1

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#include <Wire.h>
#elif defined(ARDUINO) && ARDUINO < 100
#include "WProgram.h"
#include <Wire.h>
#elif defined(__MBED__)
#include "mbed.h"
#include "externs.h"
#include <cstdlib>
#include <string>
#include "TinyGPSPlus/TinyGPS++.h"
#elif defined(PARTICLE)
#include"Particle.h"
#endif



#define MT333x_ADDR 0x10 //7-bit unshifted default I2C Address

#define MAX_PACKET_SIZE 255
//If packet size is ever more than 255 the head and tail variables will need to be
//changed to something other than uint8_t

#define I2C_SPEED_STANDARD        100000
#define I2C_SPEED_FAST            400000

class I2CGPS {
  public:

    //By default use Wire, standard I2C speed, and the default AK9750 address
    #if defined(ARDUINO) || defined(PARTICLE)
    boolean begin(TwoWire &wirePort = Wire, uint32_t i2cSpeed = I2C_SPEED_STANDARD);
    #elif defined (__MBED__)
    bool begin(I2C &wirePort = i2c, uint32_t i2cSpeed = I2C_SPEED_STANDARD);
    #endif

    void check(); //Checks module for new data
    uint8_t available(); //Returns available number of bytes. Will call check() if zero is available.
    uint8_t read(); //Returns the next available byte

    #if defined(ARDUINO) || defined(PARTICLE)
    void enableDebugging(Stream &debugPort = Serial); //Output various extra messages to help with debug
    #elif defined (__MBED__)
    void enableDebugging(Stream &debugPort = pc);
    #endif

    void disableDebugging();

    #if defined(ARDUINO) || defined(PARTICLE)
    boolean sendMTKpacket(String command);
    String createMTKpacket(uint16_t packetType, String dataField);
    String calcCRCforMTK(String sentence); //XORs all bytes between $ and *

    boolean sendPGCMDpacket(String command);
    String createPGCMDpacket(uint16_t packetType, String dataField);
    // Uses MTK CRC

    #elif defined (__MBED__)
    bool sendMTKpacket(string command);
    string createMTKpacket(uint16_t packetType, string dataField);
    string calcCRCforMTK(string sentence); //XORs all bytes between $ and *

    bool sendPGCMDpacket(string command);
    string createPGCMDpacket(uint16_t packetType, string dataField);
    #endif

    //Variables
    uint8_t gpsData[MAX_PACKET_SIZE]; //The place to store valid incoming gps data

  private:

    //Variables
    #if defined(ARDUINO) || defined(PARTICLE)
    TwoWire *_i2cPort; //The generic connection to user's chosen I2C hardware
    boolean _printDebug = false; //Flag to print the serial commands we are sending to the Serial port for debug
    #elif defined (__MBED__)
    I2C *_i2cPort; //The generic connection to user's chosen I2C hardware
    bool _printDebug = false; //Flag to print the serial commands we are sending to the Serial port for debug
    #endif
    uint8_t _i2caddr;

    

    Stream *_debugSerial; //The stream to send debug messages to if enabled

    uint8_t _head; //Location of next available spot in the gpsData array. Limited to 255.
    uint8_t _tail; //Location of last spot read from gpsData array. Limited to 255.
};
