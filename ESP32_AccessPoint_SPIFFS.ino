// libraries for using ESP32 as Access Point (outputs its own network)
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// set network details
const char* ssid = "our_ssid";
const char* password = "our_secret_password";

AsyncWebServer server(80);

void setup() {
  Serial.begin(9600);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.println(IP);
  server.begin();
}

void loop() {
  
}
