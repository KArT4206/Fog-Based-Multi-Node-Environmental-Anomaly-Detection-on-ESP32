#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include "config.h"

// Node 1: environmental sensing
static const int LDR_PIN = 34;
static const int MQ135_ADC_PIN = 35;  // Safe divider required when MQ135 is powered at 5V.
static const int SDA_PIN = 21;
static const int SCL_PIN = 22;
static const unsigned long SEND_INTERVAL_MS = 3000;
static const unsigned long WIFI_TIMEOUT_MS = 15000;

Adafruit_BMP280 bmp;
BH1750 lightMeter;
unsigned long lastSend = 0;

bool connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[Node1] WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT_MS) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[Node1] IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  Serial.println("[Node1] WiFi connection failed");
  return false;
}

void ensureWiFi() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
  analogSetPinAttenuation(MQ135_ADC_PIN, ADC_11db);

  Wire.begin(SDA_PIN, SCL_PIN);

  bool bmpOk = bmp.begin(0x76);
  if (!bmpOk) bmpOk = bmp.begin(0x77);
  if (!bmpOk) {
    Serial.println("[Node1] BMP280 not detected");
  } else {
    bmp.setSampling(
      Adafruit_BMP280::MODE_NORMAL,
      Adafruit_BMP280::SAMPLING_X2,
      Adafruit_BMP280::SAMPLING_X16,
      Adafruit_BMP280::FILTER_X16,
      Adafruit_BMP280::STANDBY_MS_125
    );
  }

  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("[Node1] BH1750 not detected");
  }

  connectWiFi();
}

void sendTelemetry(float temp, float pressure, float lux, int ldr, int gas) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.setConnectTimeout(2500);
  http.setTimeout(2500);

  String url = String(GATEWAY_HTTP_BASE) +
               "/api/node1?temp=" + String(temp, 2) +
               "&pressure=" + String(pressure, 2) +
               "&lux=" + String(lux, 2) +
               "&ldr=" + String(ldr) +
               "&gas=" + String(gas);

  if (!http.begin(url)) return;
  int code = http.GET();
  Serial.print("[Node1] HTTP: ");
  Serial.println(code);
  http.end();
}

void loop() {
  ensureWiFi();
  if (millis() - lastSend < SEND_INTERVAL_MS) {
    delay(20);
    return;
  }
  lastSend = millis();

  float temp = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;
  float lux = lightMeter.readLightLevel();
  int ldr = analogRead(LDR_PIN);
  int gas = analogRead(MQ135_ADC_PIN);

  Serial.println("---- NODE 1 ----");
  Serial.printf("Temp: %.2f C\n", temp);
  Serial.printf("Pressure: %.2f hPa\n", pressure);
  Serial.printf("BH1750: %.2f lux\n", lux);
  Serial.printf("LDR: %d ADC\n", ldr);
  Serial.printf("MQ135: %d ADC\n", gas);

  if (isfinite(temp) && isfinite(pressure) && isfinite(lux)) {
    sendTelemetry(temp, pressure, lux, ldr, gas);
  }
}
