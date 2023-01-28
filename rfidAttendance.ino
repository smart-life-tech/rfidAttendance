//*******************************libraries********************************
// RFID-----------------------------
#include <SPI.h>
#include <MFRC522.h>
// NodeMCU--------------------------
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_MLX90614.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
// http://localhost/rfidattendance/getdata.php"?card_uid=Card_uid&device_token=device_token
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myservo; // create servo object to control a servo
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
float bodyTemp = 30.0;
//************************************************************************
#define SS_PIN 2   // D2
#define RST_PIN 16 // D1
#define ServoPin 9 // D5 is GPIO14
int cameraTrigger = 10;
//************************************************************************
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
//************************************************************************
/* Set these to your desired credentials. */
const char *ssid = "Service Provider";
const char *password = "@Matinik298-2021";
const char *device_token = "8fd0551bffd173a4";
//************************************************************************
String URL = "http://192.168.1.45/rfidattendance/getdata.php"; // computer IP or the server domain
String getData, Link;
String OldCardID = "";
unsigned long previousMillis = 0;
const unsigned char Active_buzzer = 15;
int last_temp = 0;
//************************************************************************
void setup()
{
  delay(1000);
  Serial.begin(115200);
  Serial.println("code started on port 80");
  // if (!mlx.begin())
  // {
  //  Serial.println("Error connecting to MLX sensor. Check wiring.");
  // while (1)
  // ;
  //};
  lcd.init();
  pinMode(Active_buzzer, OUTPUT);
  myservo.attach(ServoPin);
  pinMode(cameraTrigger, OUTPUT);
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  //---------------------------------------------
  connectToWiFi();

  //************************************************************************

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
  digitalWrite(cameraTrigger, LOW);
}
void loop()
{
  // check if there's a connection to Wi-Fi or not
  if (!WiFi.isConnected())
  {
    connectToWiFi(); // Retry to connect to Wi-Fi
  }
  //---------------------------------------------
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
  // Serial.println(CardID);
  // checkTemperature();// if its resetting comment out this line
  SendCardID(CardID);
  delay(1000);
}
//************send the Card UID to the website*************
void SendCardID(String Card_uid)
{
  Serial.println("Sending the Card ID");
  if (WiFi.isConnected())
  {
    WiFiClient wifiClient;
    HTTPClient http; // Declare object of class HTTPClient
                     // GET Data
    last_temp = mlx.readObjectTempC();
    Serial.print("temp to be sent :");
    Serial.println(last_temp);
    getData = "?card_uid=" + String(Card_uid) + "&device_token=" + String(device_token) + "&temp=" + String(last_temp); // Add the Card ID to the GET array in order to send it
    // GET methode
    Link = URL + getData;
    http.begin(wifiClient, Link); // initiate HTTP request   //Specify content-type header

    int httpCode = http.GET();         // Send the request
    String payload = http.getString(); // Get the response payload

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
      http.end(); // Close connection
    }
  }
}
//********************connect to the WiFi******************
void checkTemperature()
{
  delay(2000);
  // Serial.print("Ambient temperature = ");
  // Serial.print(mlx.readAmbientTempC());
  // Serial.print("°C");
  // Serial.print("   ");
  Serial.print("Object temperature = ");
  last_temp = mlx.readObjectTempC();
  Serial.print(mlx.readObjectTempC());
  Serial.println("°C");
  if (mlx.readObjectTempC() > bodyTemp)
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
  digitalWrite(cameraTrigger, HIGH);
  delay(2000);
  digitalWrite(cameraTrigger, LOW);
  myservo.write(0); // tell servo to go to position
  lcd.clear();
  lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
  lcd.print("WELCOME ");
  lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
  lcd.print("PLACE YOUR CARD");
}
void connectToWiFi()
{
  WiFi.mode(WIFI_OFF); // Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // IP address assigned to your ESP

  delay(1000);
}
