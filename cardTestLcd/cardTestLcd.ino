#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 4  // D2
#define RST_PIN 5 // D1
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;
// Init array that will store new NUID
byte nuidPICC[4];
String userid = "";
void setup()
{
	Serial.begin(115200);
	lcd.begin(16, 2);
	lcd.init();
	lcd.backlight();
	lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
	lcd.print("RFID REGISTER");
	lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
	lcd.print("TEMP CHECKING");
	delay(1000);
	lcd.clear();
	lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
	lcd.print("WELCOME ");
	lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
	lcd.print("PLACE YOUR CARD");
	SPI.begin();	 // Init SPI bus
	rfid.PCD_Init(); // Init MFRC522
	Serial.println();
	Serial.print(F("Reader :"));
	rfid.PCD_DumpVersionToSerial();
	for (byte i = 0; i < 6; i++)
	{
		key.keyByte[i] = 0xFF;
	}
	Serial.println();
	Serial.println(F("This code scan the MIFARE Classic NUID."));
	Serial.print(F("Using the following key:"));
	printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}
void loop()
{
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if (!rfid.PICC_IsNewCardPresent())
		return;
	// Verify if the NUID has been readed
	if (!rfid.PICC_ReadCardSerial())
		return;
	Serial.print(F("PICC type: "));
	MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
	Serial.println(rfid.PICC_GetTypeName(piccType));
	// Check is the PICC of Classic MIFARE type
	if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
		piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
		piccType != MFRC522::PICC_TYPE_MIFARE_4K)
	{
		Serial.println(F("Your tag is not of type MIFARE Classic."));
		return;
	}
	if (rfid.uid.uidByte[0] != nuidPICC[0] ||
		rfid.uid.uidByte[1] != nuidPICC[1] ||
		rfid.uid.uidByte[2] != nuidPICC[2] ||
		rfid.uid.uidByte[3] != nuidPICC[3])
	{
		Serial.println(F("A new card has been detected."));
		// Store NUID into nuidPICC array
		for (byte i = 0; i < 4; i++)
		{
			nuidPICC[i] = rfid.uid.uidByte[i];
			userid += String(rfid.uid.uidByte[i], HEX);
		}
		lcd.clear();
		lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
		lcd.print("RFID REGISTERed");
		lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
		lcd.print(userid);
		Serial.println(F("The NUID tag is:"));
		Serial.print(F("In hex: "));
		printHex(rfid.uid.uidByte, rfid.uid.size);
		Serial.println();
		Serial.print(F("In dec: "));
		printDec(rfid.uid.uidByte, rfid.uid.size);
		Serial.println();
	}
	else
		Serial.println(F("Card read previously."));
	// Halt PICC
	rfid.PICC_HaltA();
	// Stop encryption on PCD
	rfid.PCD_StopCrypto1();
}
/**
		Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte *buffer, byte bufferSize)
{
	for (byte i = 0; i < bufferSize; i++)
	{
		Serial.print(buffer[i] < 0x10 ? " 0" : " ");
		Serial.print(buffer[i], HEX);
	}
}
/**
		Helper routine to dump a byte array as dec values to Serial.
*/
void printDec(byte *buffer, byte bufferSize)
{
	for (byte i = 0; i < bufferSize; i++)
	{
		Serial.print(buffer[i] < 0x10 ? " 0" : " ");
		Serial.print(buffer[i], DEC);
	}
}
