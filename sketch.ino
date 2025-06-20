#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* postUrl = "http://web-production-fac26.up.railway.app/device-data";
const String baseGetUrl = "http://web-production-fac26.up.railway.app/api/v1/iot-devices/";

// Identificación del dispositivo e inventario asociado
String deviceIdentifier = "esp32-001";
long inventoryId = 7;

// Pines y configuraciones
#define RXD2 16
#define TXD2 17
#define GPS_BAUDRATE 9600
#define LED_PIN 12

TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, RXD2, TXD2);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" WiFi connected");
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
      // Construir URL para verificar si el dispositivo está reservado
      String checkUrl = baseGetUrl + String(inventoryId) + "/is-reserved";
      HTTPClient checkHttp;
      checkHttp.begin(checkUrl);
      int checkCode = checkHttp.GET();

      if (checkCode == 200) {
        bool isReserved = checkHttp.getString() == "true";
        checkHttp.end();

        if (isReserved) {
          Serial.println("🚫 Dispositivo reservado. No se enviará información.");
          digitalWrite(LED_PIN, HIGH);
        } else {
          digitalWrite(LED_PIN, LOW);
          Serial.println("✅ Dispositivo libre. Enviando datos...");

          HTTPClient http;
          http.begin(postUrl);
          http.addHeader("Content-Type", "application/json");

          String payload = "{\"deviceIdentifier\":\"" + deviceIdentifier +
                          "\",\"latitude\":" + String(latitude, 6) +
                          ",\"longitude\":" + String(longitude, 6) +
                          ",\"inventoryId\":" + String(inventoryId) + "}";

          int postCode = http.POST(payload);
          Serial.print("HTTP POST Response code: ");
          Serial.println(postCode);
          http.end();
        }
      } else {
        checkHttp.end();
        Serial.println("❌ Error al verificar reserva.");
        // Parpadeo del LED como señal de error
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_PIN, HIGH);
          delay(200);
          digitalWrite(LED_PIN, LOW);
          delay(200);
        }
      }
    } else {
      Serial.println("❌ WiFi desconectado.");
    }

    delay(10000); // Esperar 10 segundos para el siguiente intento
  } else {
    Serial.println("⌛ Esperando datos GPS...");
    delay(1000);
  }
}
