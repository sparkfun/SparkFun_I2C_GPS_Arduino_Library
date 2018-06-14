/*
  Configure the Qwiic GPS module by sending PMTK packets
  By: Nathan Seidle, Owen Lyke
  SparkFun Electronics
  Date: April 12th, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Send the 'm' character to view the menu of options.

  This example shows how to configure the module including:
    Update rate to 1 or 10Hz (packet# 220)
    Enable/disable sentences (314)
    Pulse-per-second LED (285)
    Set Balloon mode (80km limit) (886)
    Set baud rate (251)
    Enable DGPS/SBAS
    Reset to factory defaults (104)

  For more information see the MTK NMEA Packet datasheet.

  Note that these settings are stored in battery backed memory and will be retained for
  approximately 15 days without power.

  This example has been updated to include limited support for the new firmware (5.x.x) that
  comes on the new modules. Use the techniques in this example along with the AirPrime software manual
  (https://cdn.sparkfun.com/assets/b/c/4/4/5/AirPrime_XM_XA_Series_Software_User_Guide_r3.pdf)
  to completely customize how the GPS module is set up

  Hardware Connections:
  Attach a Qwiic shield to your RedBoard or Uno.
  Plug the Qwiic sensor into any port.
  Serial.print it out at 115200 baud to serial monitor.
*/

#include <SparkFun_I2C_GPS_Arduino_Library.h> //Use Library Manager or download here: https://github.com/sparkfun/SparkFun_I2C_GPS_Arduino_Library
I2CGPS myI2CGPS; //Hook object to the library

#include <TinyGPS++.h> //From: https://github.com/mikalhart/TinyGPSPlus
TinyGPSPlus gps; //Declare gps object

//The following tells the TinyGPS library to scan for the PMTK001 sentence
//This sentence is the response to a configure command from the user
//Field 1 is the packet number, 2 indicates if configuration was successful
TinyGPSCustom configureCmd(gps, "PMTK001", 1); //Packet number
TinyGPSCustom configureFlag(gps, "PMTK001", 2); //Success/fail flag

// The following tells the TinyGPS library to scan for the PMTK705 sentence.
// This sentence is a response to a query of the firmware version
TinyGPSCustom versionRelease(gps, "PMTK705", 1); // Release string
TinyGPSCustom versionBuild(gps, "PMTK705", 2); // Build ID
TinyGPSCustom versionModel(gps, "PMTK705", 3); // Product model
// There can be a 4th version parameter but we are not terribly interested

#define FW_OLD  1
#define FW_NEW  2
#define FW_UNKNOWN  0

String configString;

boolean debug = false; //Keeps track of the enable/disable of debug printing within the GPS lib

void setup()
{
  Serial.begin(115200);
  Serial.println("GPS Configuration Example");

  if (myI2CGPS.begin() == false)
  {
    Serial.println("Module failed to respond. Please check wiring.");
    while (1); //Freeze!
  }
  Serial.println("GPS module found!");

  //myI2CGPS.enableDebugging(); //Turn on printing of GPS strings
  myI2CGPS.disableDebugging(); //Turn off printing of GPS strings
  debug = false;

  printMenu();
}

void loop()
{
  if (Serial.available())
  {
    byte incoming = Serial.read();
    if (incoming == '1')
    {
      Serial.println(F("Make sure the baud rate is 57600bps or higher to reach 10Hz"));
        //Packet 220: Set time between fixes (update rate)
        //Milliseconds between output. 100 to 10,000 is allowed.
        //Example 1 Hz: ",1000"
        //Example 2 Hz: ",500"
        //Example 10 Hz: ",100"
        //NOTE: You must increase the baud rate to 57600bps or higher to reach 10Hz
        configString = myI2CGPS.createMTKpacket(220, ",100");
        myI2CGPS.sendMTKpacket(configString);
    }
    else if (incoming == '2')
    {
      //Packet 220: Set time between fixes (update rate)
      //Milliseconds between output. 100 to 10,000 is allowed.
      configString = myI2CGPS.createMTKpacket(220, ",1000"); //Set to 1Hz
      myI2CGPS.sendMTKpacket(configString);
    }
    else if (incoming == '3')
    {
      //Packet 285: Set Pulse per second to occur after 3D fix, and blink for 25ms
      configString = myI2CGPS.createMTKpacket(285, ",2,25");
      myI2CGPS.sendMTKpacket(configString);
    }
    else if (incoming == '4')
    {
      //Packet 314: The SET_NMEA_OUTPUT command is very long
      //20 types of sentences are allowed: GLL / RMC / VTG / GGA / GSA / GSV / GRS / GST
      //The number in the position sets the "output once every X position fixes"
      //Example: ",1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0" = GLL only, output every fix
      //Example: ",0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0" = RMC only, output every fix
      //Example: ",0,0,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0" = VTG every fix, GGA every 4th fix
      //Example: ",-1" = Reset to defaults
      configString = myI2CGPS.createMTKpacket(314, ",0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
      myI2CGPS.sendMTKpacket(configString);
    }
    else if (incoming == '5')
    {
      //Packet 886: Set navigation mode Normal=0 / Fitness=1 / Aviation=2 / Balloon=3 (80km space limit!)
      //Normal = general purpose
      //Fitness = Running and walking, less than 5m/s movement
      //Aviation = high dynamic large acceleration movement
      //Balloon = For high altitude ballooning! Allows max height to be up to 80km instead of 10km
      configString = myI2CGPS.createMTKpacket(886, ",3");
      myI2CGPS.sendMTKpacket(configString);
    }
    else if (incoming == '6')
    {
      uint8_t FWversion = getFWversion();

      if(FWversion == FW_OLD)
      {
        //Packet 251: Set serial baud rate to 57600
        configString = myI2CGPS.createMTKpacket(251, ",57600");
        myI2CGPS.sendMTKpacket(configString);
      }
      else if(FWversion == FW_NEW)
      {
        configString = myI2CGPS.createPGCMDpacket(232, ",5");
        myI2CGPS.sendPGCMDpacket(configString);

        configString = myI2CGPS.createMTKpacket(104, "");
        myI2CGPS.sendMTKpacket(configString);
      }
      else
      {
        Serial.println(F("Firmware version unknown. Operation terminated."));
        return;
      }
      

      Serial.println(F("Serial configuration command sent. No ACK is returned for this command."));
    }
    else if (incoming == '7')
    {
      //Packet 301: Set DGPS mode
      //0 = No DGPS source
      //1 = RTCM
      //2 = SBAS(Includes WAAS/EGNOS/GAGAN/MSAS)
      //Note: You cannot enable SBAS/DGPS when update rate is faster than 5Hz
      configString = myI2CGPS.createMTKpacket(301, ",2");
      myI2CGPS.sendMTKpacket(configString);
    }
    else if (incoming == '8')
    {
      if (debug == false)
      {
        myI2CGPS.enableDebugging();
        debug = true;
      }
      else
      {
        myI2CGPS.disableDebugging();
        debug = false;
      }
    }
    else if (incoming == '9')
    {
      //Packet 104: Reset to factory defaults
      //Erases all system and user configurations. Does cold start.
      //Note: Unit does not respond with an ACK packet
      //Note: If you're using an external anteanna you must power cycle the module
      //so that it detects the external antenna
      configString = myI2CGPS.createMTKpacket(104, "");
      myI2CGPS.sendMTKpacket(configString);

      Serial.println(F("Reset command sent. No ACK is returned for this command."));
    }
    else if(incoming == 'a')
    {
      // Check firmware version
      configString = myI2CGPS.createMTKpacket(605, "");
      myI2CGPS.sendMTKpacket(configString);

      Serial.print("Querying GPS version: ");
      while(versionRelease.isUpdated() == false)
      {
        while (myI2CGPS.available()) //available() returns the number of new bytes available from the GPS module
        {
          gps.encode(myI2CGPS.read()); //Feed the GPS parser with a new .read() byte
        }
        Serial.print(F("."));
      }
      Serial.println("done!");
      
      Serial.print("Firmware version: ");
      Serial.println(versionRelease.value());
      Serial.print("Build ID: ");
      Serial.println(versionBuild.value());
      Serial.print("Model: ");
      Serial.println(versionModel.value());
      Serial.println();
      
    }
    else if(incoming == 's')
    {
      char baud_code = 0;
      uint8_t FWversion = FW_UNKNOWN;
      
      if(Serial.available() < 1)
      {
        Serial.println(F("Waiting for baud rate code. Enter a character '0' through '6'"));
      }
      while(Serial.available() < 1)
      {
        Serial.print(F("."));
        delay(100);
      }

      baud_code = Serial.read();

      if((baud_code < '0') || (baud_code > '6'))
      {
        Serial.println(F("Invalid baud rate code. Operation terminated"));
        return;
      }

      FWversion = getFWversion();

      if(FWversion == FW_OLD)
      {
        switch(baud_code)
        {
          case '0' : 
            configString = myI2CGPS.createMTKpacket(251, ",4800");
            myI2CGPS.sendMTKpacket(configString);
            break;
            
          case '1' :
            configString = myI2CGPS.createMTKpacket(251, ",9600");
            myI2CGPS.sendMTKpacket(configString);
            break;

          case '2' :
            configString = myI2CGPS.createMTKpacket(251, ",14400");
            myI2CGPS.sendMTKpacket(configString);
            break;

          case '3' :
            configString = myI2CGPS.createMTKpacket(251, ",19200");
            myI2CGPS.sendMTKpacket(configString);
            break;

          case '4' :
            configString = myI2CGPS.createMTKpacket(251, ",38400");
            myI2CGPS.sendMTKpacket(configString);
            break;

          case '5' :
            configString = myI2CGPS.createMTKpacket(251, ",57600");
            myI2CGPS.sendMTKpacket(configString);
            break;

          case '6' :
            configString = myI2CGPS.createMTKpacket(251, ",115200");
            myI2CGPS.sendMTKpacket(configString);
            break;
        }
      }
      else if(FWversion == FW_NEW)
      {
        configString = myI2CGPS.createPGCMDpacket(232, (","+(String)baud_code));
        myI2CGPS.sendPGCMDpacket(configString);

        configString = myI2CGPS.createMTKpacket(104, "");
        myI2CGPS.sendMTKpacket(configString);
      }
      else
      {
        Serial.println(F("Baud rate NOT updated"));
        return;
      }
      Serial.println(F("Baud rate updated"));
    }
    else
    {
      printMenu();
    }
  }

  while (myI2CGPS.available()) //available() returns the number of new bytes available from the GPS module
  {
    gps.encode(myI2CGPS.read()); //Feed the GPS parser with a new .read() byte
  }

  if (gps.time.isUpdated()) //Check to see if new GPS info is available
  {
    //displayInfo();
  }

  //Check to see if we got a response from any command we recently sent
  if (configureCmd.isUpdated())
  {
    Serial.print("Packet ");
    Serial.print(configureCmd.value());
    Serial.print(": ");

    switch (configureFlag.value()[0])
    {
      case '0':
        Serial.print(F("Invalid command"));
        break;
      case '1':
        Serial.print(F("Unsupported command"));
        break;
      case '2':
        Serial.print(F("Action failed"));
        break;
      case '3':
        Serial.print(F("Command successful"));
        break;
    }
    Serial.println();
  }

  delay(10);
}

void printMenu(void)
{
  Serial.println();
  Serial.println(F("1) Set update rate to 10Hz"));
  Serial.println(F("2) Set update rate to 1Hz"));
  Serial.println(F("3) Enable PPS LED"));
  Serial.println(F("4) Turn off all sentences but RMC&GGA"));
  Serial.println(F("5) Enable high altitude balloon mode"));
  Serial.println(F("6) Set serial baud rate to 57600bps"));
  Serial.println(F("7) Enable DGPS/SBAS"));
  Serial.println(F("8) Enable/Disable Debugging"));
  Serial.println(F("9) Reset module"));
  Serial.println(F("a) Query version information"));
  Serial.println(F("s) Set baud rate:"));
  Serial.println(F("--  0 : 4800"));
  Serial.println(F("--  1 : 9600"));
  Serial.println(F("--  2 : 14400"));
  Serial.println(F("--  3 : 19200"));
  Serial.println(F("--  4 : 38400"));
  Serial.println(F("--  5 : 57600"));
  Serial.println(F("--  6 : 115200"));

  Serial.println();
}

void displayInfo()
{
  //We have new GPS data to deal with!

  if (gps.time.hour() < 10) Serial.print(F("0"));
  Serial.print(gps.time.hour());
  Serial.print(F(":"));
  if (gps.time.minute() < 10) Serial.print(F("0"));
  Serial.print(gps.time.minute());
  Serial.print(F(":"));
  if (gps.time.second() < 10) Serial.print(F("0"));
  Serial.print(gps.time.second());

  Serial.print(" Loc:");
  Serial.print(gps.location.lat(), 6);
  Serial.print(F(","));
  Serial.print(gps.location.lng(), 6);

  Serial.print(F(" SIV:"));
  Serial.print(gps.satellites.value());

  Serial.print(F(" HDOP:"));
  Serial.print(gps.hdop.value()/100.0, 2); //TinyGPS reports DOPs in 100ths

  Serial.println();
}

// This function uses the custom TinyGPS parser (versionRelease, versionBuild, and versionModel)
// to try and dtermine if the GPS is running the new or old firmware. If neither can be determine then
// it will return a third value to signify UNKNOWN
// The old firmware - on Titan X1 models - uses exclusively PMTK commands
// The new firmware added some commands and (unfortunately) removed some of the PMTK commands so it needs to be handled differently
uint8_t getFWversion( void )
{
  // Check the firmware to determine what commands to use:
      configString = myI2CGPS.createMTKpacket(605, "");
      myI2CGPS.sendMTKpacket(configString);
      while(versionRelease.isUpdated() == false)
      {
        while (myI2CGPS.available()) //available() returns the number of new bytes available from the GPS module
        {
          gps.encode(myI2CGPS.read()); //Feed the GPS parser with a new .read() byte
        }
      }
      // Now the most recent firmware version data exists

      if(versionRelease.value()[0] == 'A')
      {
        if(versionRelease.value()[4] >= '5')
        {
          return FW_NEW;
        }
        else
        {
          return FW_OLD;
        }
      }
      else if(versionRelease.value()[0] == 'M')
      {
        if(versionRelease.value()[6] >= '5')
        {
          return FW_NEW;
        }
        else
        {
          return FW_OLD;
        }
      }
      else
      {
        Serial.print(F("GPS firmware version ("));
        Serial.print(versionRelease.value());
        Serial.println(F(") unknown. Operation terminated."));
        return FW_UNKNOWN;
      }
}

