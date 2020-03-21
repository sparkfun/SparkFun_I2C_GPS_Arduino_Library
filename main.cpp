#include "mbed.h"
#include "lib/SparkFun_I2C_GPS_Reading_and_Control/SparkFun_I2C_GPS_Arduino_Library.h"
#include "externs.h"

I2C i2c(I2C1_SDA, I2C1_SCL);
Serial pc(USBTX, USBRX, 115200);
I2CGPS myI2CGPS;
TinyGPSPlus gps;
string configString;

//Display new GPS info
void displayInfo()
{
  //We have new GPS data to deal with!
  pc.printf("\n");

  // display date time
  if (gps.time.isValid())
  {
    pc.printf("Datum: %d/%02d/%02d  --  %02d:%02d:%02d\n",
                         gps.date.year(), gps.date.month(), gps.date.day(),
                         gps.time.hour(), gps.time.minute(), gps.time.second());
  }
  else
  {
    pc.printf("Time not yet valid\n");
  }

  // display location
  if (gps.location.isValid())
  {
    pc.printf("Location: %f, %f\n", gps.location.lat(), gps.location.lng());
  }
  else
  {
    pc.printf("Location not yet valid\n");
  }

  // display altitude
  if (gps.altitude.isValid())
  {
    pc.printf("Altitude in meters: %f in feet: %f\n", gps.altitude.meters(), gps.altitude.feet());
  }

  // display satellite stats
  if (gps.satellites.isValid())
  {
    pc.printf(" Satellites in View: %d", gps.satellites.value());
  }

  // display HDOP
  if (gps.hdop.isValid())
  {
    pc.printf(" HDOP: %.2f\n", (gps.hdop.value()/100.0));
  }
}


// main() runs in its own thread in the OS
int main() {

  while (myI2CGPS.begin(i2c, 400000) == false) {
    pc.printf("Module failed to respond. Please check wiring.\n");
    ThisThread::sleep_for(500);
  }
  pc.printf("GPS module found!\n");

  /* if GPS module is found let us configure it */
  // setup PPS LED
  configString = myI2CGPS.createMTKpacket(285, ",4,25");
  myI2CGPS.sendMTKpacket(configString);


  while (true) {
    while (myI2CGPS.available()) // available() returns the number of new bytes
                                 // available from the GPS module
    {
      uint8_t incoming = myI2CGPS.read(); //Read the latest byte from Qwiic GPS
      if(incoming == '$') pc.printf("\n"); //Break the sentences onto new lines
      pc.printf("%c", incoming); //Print this character
    }
  }
  return 0;
}
