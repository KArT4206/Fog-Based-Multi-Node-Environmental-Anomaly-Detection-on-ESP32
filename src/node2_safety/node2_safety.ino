#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"

// Node 2: safety sensing
static const int PIR_PIN = 27;
static const int SOUND_PIN = 32;
static const int LDR_PIN = 33;
static const int SOUND_ACTIVE_STATE = LOW; // Change to HIGH if your module is inverted.
static const unsigned long SEND_INTERVAL_MS = 2500;
static const unsigned long WIFI_TIMEOUT_MS = 15000;

unsigned long lastSend = 0;

bool connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[Node2] WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT_MS) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[Node2] IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  return false;
}

void ensureWiFi() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
}

void sendTelemetry(int motion, int sound, int ldr) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.setConnectTimeout(2500);
  http.setTimeout(2500);

  String url = String(GATEWAY_HTTP_BASE) +
               "/api/node2?motion=" + String(motion) +
               "&sound=" + String(sound) +
               "&ldr=" + String(ldr);
  if (!http.begin(url)) return;
  int code = http.GET();
  Serial.print("[Node2] HTTP: ");
  Serial.println(code);
  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
  connectWiFi();
}

void loop() {
  ensureWiFi();
  if (millis() - lastSend < SEND_INTERVAL_MS) {
    delay(20);
    return;
  }
  lastSend = millis();

  int motion = digitalRead(PIR_PIN) == HIGH ? 1 : 0;
  int sound = digitalRead(SOUND_PIN) == SOUND_ACTIVE_STATE ? 1 : 0;
  int ldr = analogRead(LDR_PIN);

  Serial.println("---- NODE 2 ----");
  Serial.printf("Motion: %s\n", motion ? "DETECTED" : "clear");
  Serial.printf("Sound: %s\n", sound ? "DETECTED" : "clear");
  Serial.printf("LDR: %d ADC\n", ldr);
  sendTelemetry(motion, sound, ldr);
}
