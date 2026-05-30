#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

const int   RELAY_PIN  = 4;
const int   RELAY_ON   = LOW;   // module activates when IN is LOW
const int   RELAY_OFF  = HIGH;
const unsigned long PULSE_DURATION_MS = 1000;

const char* AP_NAME        = "Wifi-Config-ESP32";
const char* AP_PASSWORD    = "config321";

const char* MQTT_BROKER    = "broker.hivemq.com";
const int   MQTT_PORT      = 1883;
const char* MQTT_CLIENT_ID = "esp32-pansini-poc";
const char* TOPIC_STATUS   = "pansini/esp32/status";
const char* TOPIC_COMMAND  = "pansini/esp32/command";
const unsigned long PUBLISH_INTERVAL_MS = 5000;

WiFiClient   espClient;
PubSubClient mqtt(espClient);
WiFiManager  wm;

bool relayActive = false;
unsigned long lastPublish = 0;

void publishStatus() {
  String payload = relayActive ? "{\"relay\":\"on\"}" : "{\"relay\":\"off\"}";
  mqtt.publish(TOPIC_STATUS, payload.c_str());
  Serial.printf("[TX] %s :: %s\n", TOPIC_STATUS, payload.c_str());
}

void setRelay(bool on) {
  relayActive = on;
  digitalWrite(RELAY_PIN, on ? RELAY_ON : RELAY_OFF);
  publishStatus();
}

void pulseRelay() {
  Serial.println("Pulsing relay for 1s...");
  setRelay(true);
  delay(PULSE_DURATION_MS);
  setRelay(false);
}

void onMessage(char* topic, byte* payload, unsigned int length) {
  String msg;
  msg.reserve(length);
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.printf("[RX] %s :: %s\n", topic, msg.c_str());

  if (String(topic) != TOPIC_COMMAND) return;

  if (msg == "on" || msg == "1") {
    setRelay(true);
  } else if (msg == "off" || msg == "0") {
    setRelay(false);
  } else if (msg == "pulse" || msg == "open") {
    pulseRelay();
  } else {
    Serial.printf("Unknown command: %s\n", msg.c_str());
  }
}

void reconnectMqtt() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT... ");
    if (mqtt.connect(MQTT_CLIENT_ID)) {
      Serial.println("OK");
      mqtt.subscribe(TOPIC_COMMAND);
      publishStatus();
    } else {
      Serial.printf("failed rc=%d, retrying in 5s\n", mqtt.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // set relay OFF before anything else
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);

  wm.setConfigPortalTimeout(180);
  if (!wm.autoConnect(AP_NAME, AP_PASSWORD)) {
    Serial.println("WiFi failed, restarting in 5s...");
    delay(5000);
    ESP.restart();
  }
  Serial.printf("WiFi OK, IP %s\n", WiFi.localIP().toString().c_str());

  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(onMessage);
  mqtt.setBufferSize(512);
}

void loop() {
  if (!mqtt.connected()) reconnectMqtt();
  mqtt.loop();

  if (millis() - lastPublish > PUBLISH_INTERVAL_MS) {
    lastPublish = millis();
    publishStatus();
  }
}