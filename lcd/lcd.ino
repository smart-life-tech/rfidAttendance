/*Wiring an I2C LCD Display to an ESP8266
Connecting I2C LCD to ESP8266 is very easy as you only need to connect 4 pins.
 Start by connecting the VCC pin to the VIN on the ESP8266 and GND to ground.
Now we are left with the pins which are used
for I2C communication. We are going to use
  the default I2C pins (GPIO#4 and GPIO#5) of the ESP8266. 
Connect the SDA pin to the ESP8266’s D2 (GPIO#4) 
and the SCL pin to the ESP8266’s D1 (GPIO#5).*/
#include <Wire.h>

void setup() {
  Serial.begin (115200);

  // Leonardo: wait for serial port to connect
  while (!Serial) 
    {
    }

  Serial.println ();
  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;
  
  Wire.begin();
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
      {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (1);  // maybe unneeded?
      } // end of good response
  } // end of for loop
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");
}  // end of setup

void loop() {}