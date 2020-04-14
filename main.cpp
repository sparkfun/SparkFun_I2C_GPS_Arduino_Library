#include "mbed.h"
#include "src/SparkFun_I2C_GPS_Reading_and_Control/SparkFun_I2C_GPS_Arduino_Library.h"
#include "externs.h"

I2C i2c(I2C1_SDA, I2C1_SCL);
Serial pc(USBTX, USBRX, 115200);
I2CGPS myI2CGPS;
string configString;


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
