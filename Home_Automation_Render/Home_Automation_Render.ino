#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

const char *ssid = "Galaxy A14 5G";
const char *password = "masleschar3002";
String serverAddress = "http://192.168.24.215:5000";

#define FAN_PIN D7
#define LIGHT_PIN D8
#define SERVER_LED D4

// Store previous states to detect changes
bool prevFanState = false;
bool prevLightState = false;
unsigned long lastPollTime = 0;
const unsigned long POLL_INTERVAL = 1000; // Poll every 1 second

void setup() {
    Serial.begin(9600);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(LIGHT_PIN, OUTPUT);
    pinMode(SERVER_LED, OUTPUT);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(SERVER_LED, HIGH);
        delay(300);
        digitalWrite(SERVER_LED, LOW);
        delay(800);
        Serial.print(".");
    }
    
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Check server connection
    if (checkServer()) {
        Serial.println("Successfully connected to server");
        digitalWrite(SERVER_LED, HIGH);
    } else {
        Serial.println("Failed to connect to server");
        blinkError();
    }
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        unsigned long currentTime = millis();
        
        // Poll server periodically
        if (currentTime - lastPollTime >= POLL_INTERVAL) {
            pollServer();
            lastPollTime = currentTime;
        }
    } else {
        // WiFi disconnected
        digitalWrite(SERVER_LED, LOW);
        Serial.println("WiFi disconnected. Attempting to reconnect...");
        WiFi.begin(ssid, password);
    }
}

bool checkServer() {
    WiFiClient wifiClient;
    HTTPClient http;
    String url = serverAddress + "/activate";
    
    http.begin(wifiClient, url);
    int httpCode = http.GET();
    http.end();
    
    return httpCode == 200;
}

void pollServer() {
    WiFiClient wifiClient;
    HTTPClient http;
    String url = serverAddress + "/get_status";
    
    http.begin(wifiClient, url);
    int httpCode = http.GET();
    
    if (httpCode == 200) {
        String payload = http.getString();
        
        // Parse JSON response
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            bool fanState = doc["fan"];
            bool lightState = doc["light"];
            
            // Update outputs only if states have changed
            if (fanState != prevFanState) {
                digitalWrite(FAN_PIN, fanState ? HIGH : LOW);
                prevFanState = fanState;
                Serial.print("Fan state changed to: ");
                Serial.println(fanState ? "ON" : "OFF");
            }
            
            if (lightState != prevLightState) {
                digitalWrite(LIGHT_PIN, lightState ? HIGH : LOW);
                prevLightState = lightState;
                Serial.print("Light state changed to: ");
                Serial.println(lightState ? "ON" : "OFF");
            }
        } else {
            Serial.println("Failed to parse JSON");
            blinkError();
        }
    } else {
        Serial.print("Server request failed with code: ");
        Serial.println(httpCode);
        blinkError();
    }
    
    http.end();
}

void blinkError() {
    for (int i = 0; i < 3; i++) {
        digitalWrite(SERVER_LED, HIGH);
        delay(100);
        digitalWrite(SERVER_LED, LOW);
        delay(100);
    }
}