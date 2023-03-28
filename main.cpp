// Include all libraries required for current project
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>

// Network credentials
const char* ssid = "TestNet";
const char* password = "123456789";

// Initialize webserver on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// GPIO Pins to try LED w/ Button and Potentiometer
const int LED_PIN = 21;
const int BUTTON_PIN = 22;
const int POT_PIN = 15; // ESP32 GPIO 15, 2, and 4 are ACD channels 0, 1, 2

// JSON variable to hold pot readings
JSONVar data;

// Timer variables for webpage to refresh
unsigned long lastTime = 0;
unsigned long timerDelay = 500; // refresh sensor output every 500ms

// Stores LED state
String ledState;

// Function to return reading from potentiometer as JSON string
String getSensorReadings(){
  data["potentiometer"] = analogRead(POT_PIN);
  String jsonString = JSON.stringify(data);
  return jsonString;
}

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Replaces placeholder with LED state value
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(LED_PIN)){
      ledState = "ON";
    }
    else{
      ledState = "OFF";
    }
    Serial.print(ledState);
    return ledState;
  }
  return String();
}

void setup() {
  // Setup monitor and pin modes
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // initialize ESP for softAP and print IP for browser
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.println(IP);

  initSPIFFS(); // initialze file system on ESP32

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // send static webpage from SPIFFS
  server.serveStatic("/", SPIFFS, "/");
 
  // Route to get latest sensor reading at "IP/readings"
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  // initialize server
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Send Events to the client with the Sensor Readings Every 10 seconds
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());
    lastTime = millis();
  }
}