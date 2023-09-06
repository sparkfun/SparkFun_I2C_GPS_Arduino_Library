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

#include "SparkFun_I2C_GPS_Arduino_Library.h"

#if defined(__MBED__)
#include <iostream>
#include <stdlib.h>
char dummy = 0;
#endif

//Sets up the sensor for constant read
//Returns false if sensor does not respond
#if defined (ARDUINO) || defined(PARTICLE)
boolean I2CGPS::begin(TwoWire &wirePort, uint32_t i2cSpeed)
{
  //Bring in the user's choices
  _i2cPort = &wirePort; //Grab which port the user wants us to use

  _i2cPort->begin();
  _i2cPort->setClock(i2cSpeed);

  _head = 0; //Reset the location holder
  _tail = 0;

  //Ping the module to see if it responds
  _i2cPort->beginTransmission(MT333x_ADDR);
  _i2cPort->write((byte) 0x00); //Write dummy value
  _i2cPort->endTransmission();

  if (_i2cPort->requestFrom(MT333x_ADDR, 1))
    return (true); //Success!
  else
    return (false); //Module failed to respond
}

#elif defined(__MBED__)
bool I2CGPS::begin(I2C &wirePort, uint32_t i2cSpeed)
{
  //Bring in the user's choices
  _i2cPort = &wirePort; //Grab which port the user wants us to use

  _i2cPort->frequency(i2cSpeed);

  _head = 0; //Reset the location holder
  _tail = 0;

  _i2cPort->write(MT333x_ADDR << 1, (char *) &dummy, sizeof(dummy));

  if (0 == _i2cPort->read(MT333x_ADDR << 1|1, (char *) &dummy, sizeof(dummy)))      // might fail if current is unstable on GPIO pins
    return (true);
  else
    return (false);
}
#endif

//Polls the GPS module to see if new data is available
//Reads a 255 byte packet from GPS module
//If new data is there, appends it to the gpsData array
//Function requires 26ms @ 100kHz I2C, 9ms @ 400kHz I2C so call sparingly
void I2CGPS::check()
{
  //TODO: Re-write this function to be less tied to Arduino's 32 byte limit
  //Maybe pass a maxRead during .begin()

  for (uint8_t x = 0; x < MAX_PACKET_SIZE; x++)
  {
    #if defined (ARDUINO) || defined(PARTICLE)
    if (x % 32 == 0)                          //Arduino can only Wire.read() in 32 byte chunks. Yay.
      _i2cPort->requestFrom(MT333x_ADDR, 32); //Request 32 more bytes

    uint8_t incoming = _i2cPort->read();
    #elif defined (__MBED__)
    uint8_t incoming;
    _i2cPort->read(MT333x_ADDR << 1|1, (char *)&incoming, 1);
    #endif

    if (incoming != 0x0A)
    {
      //Record this byte
      gpsData[_head++] = incoming;
      _head %= MAX_PACKET_SIZE; //Wrap variable

      if (_printDebug == true && _head == _tail)
        #if defined (ARDUINO) || defined(PARTICLE)
        _debugSerial->println(F("Buffer overrun"));
        #elif defined (__MBED__)
        _debugSerial->printf("Buffer overrun\n");
        #endif
    }
  }
}

//Returns # of available bytes that can be read
uint8_t I2CGPS::available()
{
  //If tail=head then no new data is available in the local buffer
  //So now check to see if the module has anything new in its buffer
  if (_tail == _head)
  {
    check(); //Check to module to see if new I2C bytes are available
  }

  //Return new data count
  if (_head > _tail)
    return (_head - _tail);
  if (_tail > _head)
    return (MAX_PACKET_SIZE - _tail + _head);
  return (0); //No data available
}

//Returns the next available byte from the gpsData array
//Returns 0 if no byte available
uint8_t I2CGPS::read(void)
{
  if (_tail != _head)
  {
    uint8_t datum = gpsData[_tail++];
    if (_tail == MAX_PACKET_SIZE)
      _tail = 0; //Wrap variable
    return (datum);
  }
  else
    return (0); //No new data
}

//Enables serial printing of local error messages
void I2CGPS::enableDebugging(Stream &debugPort)
{
  _debugSerial = &debugPort; //Grab which port the user wants us to use for debugging

  _printDebug = true; //Should we print the commands we send? Good for debugging
}

//Turn off printing of GPS character streams
void I2CGPS::disableDebugging()
{
  _printDebug = false; //Turn off extra print statements
}

//Functions for sending commands to the GPS module
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Send a given command or configuration string to the module
//The input buffer on the MTK is 255 bytes. Caller must keep strings shorter than 255 bytes
//Any time you end transmission you must give the module 10ms to process bytes
#if defined (ARDUINO) || defined(PARTICLE)
boolean I2CGPS::sendMTKpacket(String command)
{
  if (command.length() > 255)
  {
    if (_printDebug == true)
      _debugSerial->println(F("Command message too long!"));

    return (false);
  }

  //Arduino can only Wire.write() in 32 byte chunks. Yay.
  for (uint8_t chunk = 0; chunk < 7; chunk++) //8 chunks * 32 = 256 bytes total so we need to shave one
  {
    _i2cPort->beginTransmission(MT333x_ADDR);
    for (uint8_t x = 0; x < 32; x++) //Send out 32 bytes
    {
      if ((chunk * 32 + x) == command.length())
        break; //We're done!
      _i2cPort->write(command[(chunk * 32) + x]);
    }
    _i2cPort->endTransmission();

    delay(10); //Slave requires 10 ms to process incoming bytes
  }

  //Send final 31 bytes
  if (command.length() > (7 * 32)) //Do we have more than 224 bytes? Then send last 31
  {
    _i2cPort->beginTransmission(MT333x_ADDR);
    for (uint8_t x = 0; x < 31; x++) //Write any remaining bytes up to 255
    {
      if ((7 * 32 + x) == command.length())
        break; //We're done!
      _i2cPort->write(command[(7 * 32) + x]);
    }
    _i2cPort->endTransmission();
  }

  return (true);
}

#elif defined (__MBED__)
bool I2CGPS::sendMTKpacket(string command)
{
  if (command.length() > 255)
  {
    if (_printDebug == true)
      _debugSerial->printf("Command message too long!\n");

    return (false);
  }
 
  _i2cPort->write(MT333x_ADDR << 1, command.c_str(), command.length());
  ThisThread::sleep_for(10);

  return (true);
}
#endif

//Given a packetType and any settings, return string that is a full
//config sentence complete with CRC and \r \n ending bytes
//PMTK uses a different packet numbers to do configure the module.
//These vary from 0 to 999. See 'MTK NMEA Packet' datasheet for more info.
#if defined (ARDUINO) || defined(PARTICLE)
String I2CGPS::createMTKpacket(uint16_t packetType, String dataField)
{
  //Build config sentence using packetType
  String configSentence = "";
#elif defined (__MBED__)
string I2CGPS::createMTKpacket(uint16_t packetType, string dataField)
{
  //Build config sentence using packetType
  string configSentence = "";
#endif
  configSentence += "$PMTK"; //Default header for all GPS config messages

  //Attach the packetType number
  //Append any leading zeros
  if (packetType < 100)
    configSentence += "0";
  if (packetType < 10)
    configSentence += "0";
#if defined (ARDUINO) || defined(PARTICLE)
  configSentence += String(packetType);
#elif defined (__MBED__)
  configSentence += to_string(packetType);
#endif

  //Attach any settings
  if (dataField.length() > 0)
  {
    configSentence += dataField; //Attach the string of flags
  }

  configSentence += "*"; //Attach end tag

  configSentence += calcCRCforMTK(configSentence); //Attach CRC

  //Attach ending bytes
  configSentence += '\r'; //Carriage return
  configSentence += '\n'; //Line feed

  return (configSentence);
}

//Calculate CRC for MTK messages
//Given a string of characters, XOR them all together and return CRC in string form
#if defined (ARDUINO) || defined(PARTICLE)
String I2CGPS::calcCRCforMTK(String sentence)
{
#elif defined (__MBED__)
string I2CGPS::calcCRCforMTK(string sentence)
{
#endif
  uint8_t crc = 0;

  //We need to ignore the first character $
  //And the last character *
  for (uint8_t x = 1; x < sentence.length() - 1; x++)
    crc ^= sentence[x]; //XOR this byte with all the others

  #if defined (ARDUINO) || defined(PARTICLE)
  String output = "";
  #elif defined (__MBED__)
  string output = "";
  #endif
  if (crc < 10)
    output += "0"; //Append leading zero if needed

  #if defined (ARDUINO) || defined(PARTICLE)
  output += String(crc, HEX);
  #elif defined (__MBED__)
  static char outhex[4];
  sprintf(outhex, "%02X", crc);

  output += outhex;
  #endif

  return (output);
}

#if defined (ARDUINO) || defined(PARTICLE)
boolean I2CGPS::sendPGCMDpacket(String command)
{
#elif defined (__MBED__)
bool I2CGPS::sendPGCMDpacket(string command)
{
#endif
  return (sendMTKpacket(command)); // Send process is the same, re-named to ease user's minds
}

#if defined (ARDUINO) || defined(PARTICLE)
String I2CGPS::createPGCMDpacket(uint16_t packetType, String dataField)
{
  //Build config sentence using packetType
  String configSentence = "";
#elif defined (__MBED__)
string I2CGPS::createPGCMDpacket(uint16_t packetType, string dataField)
{
  //Build config sentence using packetType
  string configSentence = "";
#endif
  configSentence += "$PGCMD,"; //Default header for all PGCMD messages

  //Attach the packetType number
  //Append any leading zeros
  if (packetType < 100)
    configSentence += "0";
  if (packetType < 10)
    configSentence += "0";
#if defined (ARDUINO) || defined(PARTICLE)
  configSentence += String(packetType);
#elif defined (__MBED__)
  configSentence += to_string(packetType);
#endif

  //Attach any settings
  if (dataField.length() > 0)
  {
    configSentence += dataField; //Attach the string of flags
  }

  configSentence += "*"; //Attach end tag

  configSentence += calcCRCforMTK(configSentence); //Attach CRC - uses the same crc as PTMK

  //Attach ending bytes
  configSentence += '\r'; //Carriage return
  configSentence += '\n'; //Line feed

  return (configSentence);
}
