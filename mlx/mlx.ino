/*The following figure shows the connection diagram between MLX90614 
temperature sensor and ESP8266.
Connect the power supply pin (Vin) of the temperature sensor to the 3.3V pin
 of ESP8266 and the GND pin of MLX90614 to the GND pin of ESP8266.
Connect the SDA and SCL pins of the mentioned IR sensor to D2 and
 D1 pins of ESP8266 for transferring data serially. D2 and D1 pins of ESP8266 
also share alternate function of SDA and SCL pins of I2C port of ESP8266.*/
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1);
  };
}

void loop() {
  Serial.print("Ambient temperature = "); 
  Serial.print(mlx.readAmbientTempC());
  Serial.print("°C");      
  Serial.print("   ");
  Serial.print("Object temperature = "); 
  Serial.print(mlx.readObjectTempC()); 
  Serial.println("°C");
  
  Serial.print("Ambient temperature = ");
  Serial.print(mlx.readAmbientTempF());
  Serial.print("°F");      
  Serial.print("   ");
  Serial.print("Object temperature = "); 
  Serial.print(mlx.readObjectTempF()); 
  Serial.println("°F");

  Serial.println("-----------------------------------------------------------------");
  delay(1000);
}
