#include "WiFi.h"
#include "globals.h"

void mqtt_init() {
  // Connect to Wi-Fi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Configure MQTT callbacks
  mqtt.onConnected([]{
    Serial.println("MQTT connected!");
    // Subscribe to a topic
    mqtt.subscribe("demo/topic", 1); // QoS 1
  });

  mqtt.onDisconnected([]{
    Serial.println("MQTT disconnected.");
  });

  mqtt.onMessage([](const char* topic, size_t topic_len, const uint8_t* data, size_t len){
    Serial.print("Received message on topic: ");
    Serial.write(topic, topic_len);
    Serial.print(" => ");
    Serial.write(data, len);
    Serial.println();
  });

  // Initialize and connect to MQTT broker
  mqtt.begin(broker, "esp32-client-id");
  mqtt.connect();
}