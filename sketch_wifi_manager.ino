#include <WiFi.h>
#include <WiFiManager.h>

const int  LED          = 2;
const char* AP_NAME     = "Wifi-Config-ESP32";
const char* AP_PASSWORD = "config321";
const int   CONFIG_TIMEOUT = 180;

WiFiManager wm;

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  //wm.resetSettings();

  wm.setConfigPortalTimeout(CONFIG_TIMEOUT);

  // LED constantly lit = in configuration mode (AP)
  wm.setAPCallback([](WiFiManager *mgr) {
    Serial.printf("Entering AP mode. Connect to \"%s\" and open http://%s\n",
                  AP_NAME, WiFi.softAPIP().toString().c_str());
    digitalWrite(LED, HIGH);
  });

  // autoConnect: try using saved credentials; if that fails, load the portal
  bool ok = wm.autoConnect(AP_NAME, AP_PASSWORD);

  if (!ok) {
    Serial.println("Failed to connect/configure. Restarting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("\n--- Connected ---");
  Serial.printf("IP........: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Gateway...: %s\n", WiFi.gatewayIP().toString().c_str());
  Serial.printf("RSSI......: %d dBm\n", WiFi.RSSI());
  Serial.printf("MAC.......: %s\n", WiFi.macAddress().c_str());

  // it flashes 3 times to indicate success and then remains lit
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED, LOW);  delay(150);
    digitalWrite(LED, HIGH); delay(150);
  }
}

void loop() {
}