//*******************************libraries********************************
// RFID-----------------------------
#include <SPI.h>
#include <MFRC522.h>

#include <Adafruit_MLX90614.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);
Servo myservo; // create servo object to control a servo
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
float bodyTemp = 30.0;
//************************************************************************
#define SS_PIN 10  // D2
#define RST_PIN 5 // D1

#define ServoPin 9 // D5 is GPIO14
//************************************************************************
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
//************************************************************************
/* Set these to your desired credentials. */
const char *ssid = "Alsan Air WiFi 4";
const char *password = "11122235122@kap1";
const char *device_token = "2c4f3c61aa79d533";
//************************************************************************
String URL = "http://192.168.1.8/rfidattendance/getdata.php"; // computer IP or the server domain
String getData, Link;
String OldCardID = "";
unsigned long previousMillis = 0;
const unsigned char Active_buzzer = 15;
Servo servo;
//************************************************************************
void setup()
{
  delay(1000);
  Serial.begin(115200);
  pinMode(Active_buzzer, OUTPUT);
  //Wire.begin(D2, 10); //  I2C ON D2 AND  THE S3 PN ON THE 8266
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  //---------------------------------------------
  //connectToWiFi();

  //************************************************************************
  // Serial.begin(115200);
  // while (!Serial);

  if (!mlx.begin())
  {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    // while (1);
  }
  lcd.init();
  lcd.clear();
  lcd.backlight(); // Make sure backlight is on

  // Print a message on both lines of the LCD.
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
  Serial.println("main loop started");
  servo.attach(ServoPin);
}
void loop()
{
 
  if (millis() - previousMillis >= 15000)
  {
    previousMillis = millis();
    OldCardID = "";
  }
  delay(50);
  //---------------------------------------------
  // look for new card
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return; // got to start of loop if there is no card present
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return; // if read card serial(0) returns 1, the uid struct contians the ID of the read card.
  }
  String CardID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    CardID += mfrc522.uid.uidByte[i];
  }
  //---------------------------------------------
  if (CardID == OldCardID)
  {
    return;
  }
  else
  {
    OldCardID = CardID;
  }
  //---------------------------------------------
  //  Serial.println(CardID);
  SendCardID(CardID);
  delay(1000);
}
//************send the Card UID to the website*************
void SendCardID(String Card_uid)
{
  Serial.println("Sending the Card ID");
  if (true)
  {
    //WiFiClient wifiClient;
   // HTTPClient http; // Declare object of class HTTPClient
    // GET Data
    getData = "?card_uid=" + String(Card_uid) + "&device_token=" + String(device_token); // Add the Card ID to the GET array in order to send it
    // GET methode
    Link = URL + getData;
    //http.begin(wifiClient, Link); // initiate HTTP request   //Specify content-type header

    int httpCode = 200;         // Send the request
    String payload = "available"; // Get the response payload

    //    Serial.println(Link);   //Print HTTP return code
    Serial.println(httpCode); // Print HTTP return code
    Serial.println(Card_uid); // Print Card ID
    Serial.println(payload);  // Print request response payload

    if (httpCode == 200)
    {
      lcd.clear();
      lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
      lcd.print("CORRECT CARD");
      lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
      lcd.print("CHECKING DB");
      if (payload.substring(0, 5) == "login")
      {
        String user_name = payload.substring(5);
        Serial.println(user_name);
      }
      else if (payload.substring(0, 6) == "logout")
      {
        String user_name = payload.substring(6);
        Serial.println(user_name);
      }
      else if (payload == "succesful")
      {
        lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
        lcd.print("SUCESSFUL LOGGING");
        lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
        lcd.print("CHECKING TEMP");
        checkTemperature();
      }
      else if (payload == "available")
      {
        lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
        lcd.print("SUCESSFUL LOGGING");
        lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
        lcd.print("CHECKING TEMP");
        checkTemperature();
      }
      delay(100);
     
    }
  }
}
//********************connect to the WiFi******************
void checkTemperature()
{
  delay(2000);
  Serial.print("Ambient temperature = ");
  Serial.print(mlx.readAmbientTempC());
  Serial.print("°C");
  Serial.print("   ");
  Serial.print("Object temperature = ");
  Serial.print(mlx.readObjectTempC());
  Serial.println("°C");
  if (/*mlx.readAmbientTempC()*/ 100 > bodyTemp)
  {
    lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
    lcd.print("GATE CLOSED");
    lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
    lcd.print("TEMP TOO HIGH");
    tone(Active_buzzer, 523); // DO note 523 Hz
    delay(1000);
    tone(Active_buzzer, 587); // RE note ...
    delay(1000);
    noTone(Active_buzzer);
    lcd.clear();
    lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
    lcd.print("WELCOME ");
    lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
    lcd.print("PLACE YOUR CARD");
  }
  else
  {
    openGate();
  }
  /*
    Serial.print("Ambient temperature = ");
    Serial.print(mlx.readAmbientTempF());
    Serial.print("°F");
    Serial.print("   ");
    Serial.print("Object temperature = ");
    Serial.print(mlx.readObjectTempF());
    Serial.println("°F");
  */
}
void openGate()
{
  lcd.clear();
  lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
  lcd.print("GATE OPENED");
  lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
  lcd.print("TEMP IS NORMAL");
  myservo.write(90); // tell servo to go to position
  delay(2000);
  myservo.write(0); // tell servo to go to position
  lcd.clear();
  lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
  lcd.print("WELCOME ");
  lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
  lcd.print("PLACE YOUR CARD");
}

