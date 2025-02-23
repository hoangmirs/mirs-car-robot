#include "camera.h"
#include "web_server.h"
#include <Arduino.h>

// Global variables - defined here
int gpLed = 4; // Light
String WiFiAddr = "";
byte txdata[3] = {0xA5, 0, 0x5A};

// WiFi credentials
const char *ssid = "M&D";
const char *password = "Dory@1234";

// Global objects
Camera camera;
WebServer *server = nullptr;

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Initialize camera
  if (!camera.init())
  {
    Serial.println("Camera initialization failed");
    return;
  }

  // Configure LED
  pinMode(gpLed, OUTPUT);
  digitalWrite(gpLed, LOW);

  // Initialize web server
  server = new WebServer(camera);
  server->setWiFiCredentials(ssid, password);

  Serial.print("WiFi connecting");
  if (!server->connectToWiFi())
  {
    Serial.println("WiFi connection failed");
    return;
  }
  Serial.println("WiFi connected");

  server->start();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(server->getWiFiAddress());
  WiFiAddr = server->getWiFiAddress();
  Serial.println("' to connect");
}

void loop()
{
  // Do nothing. Everything is done in another task by the web server
  delay(10000);
}
