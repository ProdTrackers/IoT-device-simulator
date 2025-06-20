#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* serverUrl = "http://web-production-fac26.up.railway.app/device-data";

// Harcodeado osi osi 
String deviceIdentifier = "esp32-001";
long inventoryId = 7;

#define RXD2 16
#define TXD2 17
#define GPS_BAUDRATE 9600

TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, RXD2, TXD2);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void loop() {
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }

  if (gps.location.isValid()) {
    double latitude = gps.location.lat();
    double longitude = gps.location.lng();

    Serial.print("Latitude: ");
    Serial.println(latitude, 6);
    Serial.print("Longitude: ");
    Serial.println(longitude, 6);

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/json");

      String payload = "{\"deviceIdentifier\":\"" + deviceIdentifier +
                      "\",\"latitude\":" + String(latitude, 6) +
                      ",\"longitude\":" + String(longitude, 6) +
                      ",\"inventoryId\":" + String(inventoryId) + "}";

      int httpResponseCode = http.POST(payload);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
    }
    delay(10000); // Send every 10 seconds
  } else {
    Serial.println("Waiting for valid GPS data...");
    delay(1000);
  }
}