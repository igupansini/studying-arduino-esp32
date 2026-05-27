#include <WiFi.h>

const char* WIFI_SSID     = "IGOR_WIFI";
const char* WIFI_PASSWORD = "pwd123";

const int LED = 2;  // blue led

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start > 20000) {  // timeout 20s
      Serial.println("\Failed. Status: " + String(WiFi.status()));
      return;
    }
  }

  Serial.println("\n--- Connected ---");
  Serial.printf("IP........: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Gateway...: %s\n", WiFi.gatewayIP().toString().c_str());
  Serial.printf("RSSI......: %d dBm\n", WiFi.RSSI());
  Serial.printf("MAC.......: %s\n", WiFi.macAddress().c_str());

  digitalWrite(LED, HIGH);  // remains lit = connected
}

void loop() {
}