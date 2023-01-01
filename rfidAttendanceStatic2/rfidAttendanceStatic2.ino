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

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myservo; // create servo object to control a servo
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
float bodyTemp = 35.0;
float temp = 0.0;
//************************************************************************
#define SS_PIN 2    // D2
#define RST_PIN 16  // D1
#define ServoPin 15 // D5 is GPIO14
//************************************************************************
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
//************************************************************************
/* Set these to your desired credentials. */
const char *ssid = "Service Provider";
const char *password = "@Matinik298-2021";
const char *device_token = "79f3acf63de9173d";
//************************************************************************
String URL = "http://192.168.1.45/rfidattendance/getdata.php"; // computer IP or the server domain
String getData, Link;
String OldCardID = "";
unsigned long previousMillis = 0;
const unsigned char Active_buzzer = 20;

//************************************************************************
void setup()
{
  delay(1000);
  Serial.begin(115200);
  if (!mlx.begin())
  {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    // while (1)
    ;
  };
  lcd.init();
  pinMode(Active_buzzer, OUTPUT);
  myservo.attach(ServoPin);
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  //---------------------------------------------
  connectToWiFi();

  //************************************************************************
  // Serial.begin(115200);
  // while (!Serial)
  //  ;

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
  //  Serial.println(CardID);

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
    lcd.clear();
    lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
    lcd.print("Scanning");
    lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
    lcd.print("CHECKING Data");

    float temps = checkTemperatures();
    checkTemperature(temps);
    getData = "?card_uid=" + String(Card_uid) + "&device_token=" + String(device_token) + "&temp=" + String(temps); // Add the Card ID to the GET array in order to send it
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
      //      lcd.clear();
      //      lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
      //      lcd.print("CHECKING CARD");
      //      lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
      //      lcd.print("CARD NOT FOUND");
      tone(Active_buzzer, 523); // DO note 523 Hz
      delay(1000);
      tone(Active_buzzer, 587); // RE note ...
      delay(1000);
      noTone(Active_buzzer);
      //      myservo.write(180); // tell servo to go to position
      //      delay(1000);
      //      myservo.write(0); // tell servo to go to position
      if (payload.substring(0, 5) == "login")
      {
        String user_name = payload.substring(5);
        Serial.println(user_name);
        lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
        lcd.print("SUCESSFUL, check temp");
        lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
        lcd.print("sensor on 5 sec");
        //checkTemperature();
      }

      else if (payload.substring(0, 6) == "logout")
      {
        String user_name = payload.substring(6);
        Serial.println(user_name);
        lcd.clear();
        lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
        lcd.print("LOGGING OUT");
        lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
        lcd.print("THANK YOU!");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
        lcd.print("WELCOME ");
        lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
        lcd.print("PLACE YOUR CARD");
        //checkTemperature();
        delay(100);
      }
      else if (payload == "succesful")
      {
        lcd.clear();
        lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
        lcd.print("SUCESSFUL, check temp");
        lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
        lcd.print("sensor on 5 sec");
        //checkTemperature();
      }
      else if (payload == "available")
      {
        lcd.clear();
        lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
        lcd.print("SUCESSFUL, check temp");
        lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
        lcd.print("sensor on 5 sec");
        //checkTemperature();
      }
      delay(100);
      http.end(); // Close connection
    }
  }
}
//********************connect to the WiFi******************
void checkTemperature(int read)
{
  delay(5000);
  Serial.print("Ambient temperature in use = ");
  Serial.print(mlx.readAmbientTempC());
  // temp = mlx.readObjectTempC();
  temp = read;
  Serial.print("°C");
  Serial.print("   ");
  Serial.print("Object temperature in use = ");
  Serial.print(temp);

  Serial.println("°C");
  if (temp > bodyTemp)
  {
    String temps = String(temp, 3);
    lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
    lcd.print("high temp CLOSE");
    lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
    lcd.print(temps);
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
    temp = 0;
  }
  else if (temp > 30 && temp < bodyTemp)
  {
    openGate();
    temp = 0;
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
  String temps = String(mlx.readObjectTempC(), 3);
  lcd.setCursor(0, 0); // Set cursor to character 2 on line 0
  lcd.print("open, temp is :");
  lcd.setCursor(0, 1); // Move cursor to character 2 on line 1
  lcd.print(temp);
  myservo.write(180); // tell servo to go to position
  delay(3000);
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
int checkTemperatures()
{
  delay(5000);
  Serial.print("temperature in use  = ");
  Serial.print(mlx.readObjectTempC());
  Serial.print("°C");
  Serial.print("   ");
  Serial.print("Object temperature sent to server  = ");
  Serial.println(mlx.readObjectTempC());
  return mlx.readObjectTempC();
}
