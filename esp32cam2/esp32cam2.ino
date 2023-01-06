#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <ESP32WebServer.h>
#include <Camera.h>

const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";

const char *server = "YOUR_PHP_SERVER_URL";

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
}

void loop()
{
    // Capture an image and save it to the ESP32's filesystem
    Camera.takePicture();
    String imageName = "image.jpg";
    Camera.saveToFS(imageName);

    // Send the image file to the PHP server
    WiFiClient client;
    if (!client.connect(server, 80))
    {
        Serial.println("Connection to server failed!");
        return;
    }
    // First, create the image file from the camera data
    File imageFile = SPIFFS.open("/image.jpg", "w");
    // Write the image data to the file
    imageFile.write(cameraData, imageSize);
    imageFile.close();

    // Then, create the POST request
    HTTPClient http;
    http.begin("http://192.168.1.45/rfidattendance/upload.php"); // specify the server URL
    http.addHeader("Content-Type", "multipart/form-data");

    // Create a multipart message with the file
    String body = "--AaB03x\r\n";
    body += "Content-Disposition: form-data; name='image'; filename='image.jpg'\r\n";
    body += "Content-Type: image/jpeg\r\n\r\n";

    // Add the file data to the message
    File image = SPIFFS.open("/image.jpg", "r");
    body += image.readString();
    image.close();

    // Add the closing boundary to the message
    body += "\r\n--AaB03x--\r\n";

    // Set the message body and send the request
    http.addHeader("Content-Length", String(body.length()));
    http.POST(body);

    // Check the response
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        Serial.printf("HTTP Response code: %d\n", httpCode);
    }
}