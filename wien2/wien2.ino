#include <SoftwareSerial.h>
#include <Arduino.h>
#include <stdint.h>

class WinsenZE25O3
{
public:
  WinsenZE25O3(uint8_t rxPin);
  int readPPB();
private:
  Stream *sensorInputStream_;
  boolean isChecksumCorrect(byte *sensorMessage);
};
/**
 * Initialize to listen on digital port nr rxPin
 **/
WinsenZE25O3::WinsenZE25O3(uint8_t rxPin) {
  SoftwareSerial* serial = new SoftwareSerial{rxPin, 255};
  serial->begin(9600);
  sensorInputStream_ = serial;
}

/**
 * Returns ozone concentration in PPB or -1 if either
 * there's a checksum mismatch or no data was available.
 **/
int WinsenZE25O3::readPPB() {
  int ozonePPB = -1;

  if (sensorInputStream_->available() > 8) {
    byte sensorMessage[9] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    sensorInputStream_->readBytes(sensorMessage, 9);

    if (isChecksumCorrect(sensorMessage)) {
      ozonePPB = sensorMessage[4] * 256 + sensorMessage[5];
    }
  }

  return ozonePPB;
}

/**
 * Check checksum as per datasheet.
 **/
boolean WinsenZE25O3::isChecksumCorrect(byte *sensorMessage) {
  byte checksum = 0;

  for (int i = 1; i < 8; i++) {
    checksum += sensorMessage[i];
  }

  checksum = (~checksum) + 1;

  return checksum == sensorMessage[8];
}
WinsenZE25O3 sensor{2};// digital pin 2 for receiving the serial data

void setup() {
  Serial.begin(9600);
  Serial.println("code started");
}

void loop() {
  int ppb = sensor.readPPB();
  int analogVal = analogRead(A0);
  Serial.print(ppb);
    Serial.print(",");
    Serial.print(analogVal);
    Serial.print(",");
    Serial.println(estimateIndoorNO2PPB(analogVal));
    delay(1000);
  if (ppb > 0) {
    Serial.print(ppb);
    Serial.print("ppm ,");
    Serial.print(analogVal);
    Serial.print("analog val, estimated");
    Serial.println(estimateIndoorNO2PPB(analogVal));
    delay(1000);
  }
}

double estimateIndoorNO2PPB(int analogVal) {
  // based on fitted model for predicting outdoor NO2 and indoor/outdoor ratio from
  // https://link.springer.com/article/10.1007/s10653-019-00441-0?shared-article-renderer
  return ((7.51826819982211 * analogVal - 636.9571501802109) * 0.73) / 1.88;
}