#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

const int   LED            = 2;
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

bool ledState = false;
unsigned long lastPublish = 0;

void publishStatus() {
  String payload = ledState ? "{\"led\":\"on\"}" : "{\"led\":\"off\"}";
  mqtt.publish(TOPIC_STATUS, payload.c_str());
  Serial.printf("[TX] %s :: %s\n", TOPIC_STATUS, payload.c_str());
}

void onMessage(char* topic, byte* payload, unsigned int length) {
  String msg;
  msg.reserve(length);
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.printf("[RX] %s :: %s\n", topic, msg.c_str());

  if (String(topic) == TOPIC_COMMAND) {
    if (msg == "on" || msg == "1") {
      ledState = true;
      digitalWrite(LED, HIGH);
    } else if (msg == "off" || msg == "0") {
      ledState = false;
      digitalWrite(LED, LOW);
    }
    publishStatus();
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
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

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
