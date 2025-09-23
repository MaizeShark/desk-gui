// src/mqtt.cpp

#include "mqtt.h"
#include <WiFi.h>
#include <PubSubClient.h>

// Create the underlying WiFi and MQTT client objects
WiFiClient espClient;
PubSubClient client(espClient);

// To hold the function pointer for the image update
static ImageUpdateCallback image_update_handler = nullptr;

// For reconnection logic
long lastReconnectAttempt = 0;

/**
 * @brief The master callback function for handling all incoming MQTT messages.
 */
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    Serial.printf("Message arrived on topic: %s. Length: %u\n", topic, length);

    // --- Handle Image Topic ---
    if (strcmp(topic, Config::topic_image) == 0) {
        if (image_update_handler != nullptr) {
            // Call the registered callback function with the image data
            image_update_handler(payload, length);
        } else {
            Serial.println("Received image, but no handler is registered!");
        }
        return; // Done
    }

    // For other topics, we'll probably want a null-terminated string
    // IMPORTANT: Make a copy to safely null-terminate it.
    char msg_buffer[length + 1];
    memcpy(msg_buffer, payload, length);
    msg_buffer[length] = '\0';
    String message = String(msg_buffer);


    // --- Handle Brightness Topic ---
    if (strcmp(topic, Config::topic_brightness) == 0) {
        int brightness = message.toInt();
        brightness = constrain(brightness, 0, 255); // Safety check
        Serial.printf("Setting display brightness to %d\n", brightness);
        my_lcd.setBrightness(brightness);
        // Optional: you could re-publish here to confirm the change, but it
        // might cause a loop if the other side isn't careful.
        // It's better to only publish when the value is changed *locally*.
    }

    // --- Handle Command Topic ---
    else if (strcmp(topic, Config::topic_command) == 0) {
        Serial.printf("Received command: %s\n", message.c_str());
        if (message.equalsIgnoreCase("reboot")) {
            Serial.println("Rebooting device...");
            publish_status("rebooting");
            delay(200);
            ESP.restart();
        } 
        else if (message.equalsIgnoreCase("led_on")) {
            Serial.println("Turning LEDs on");
            // Your logic to turn LEDs on
            ledsOn = true;
        }
        else if (message.equalsIgnoreCase("led_off")) {
            Serial.println("Turning LEDs off");
            // Your logic to turn LEDs off
            ledsOn = false;
        }
        // Add more else-if blocks for other commands
        else {
            Serial.println("Unknown command");
        }
    }
}


/**
 * @brief Attempts to connect to the MQTT broker and subscribe to topics.
 */
bool reconnect() {
    String clientId = "esp32-gui-";
    clientId += String(random(0xffff), HEX);

    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientId.c_str(), Config::broker_user, Config::broker_pass)) {
        Serial.println("connected!");

        // Publish an "online" status message
        publish_status("online");

        // *** Resubscribe to all topics upon reconnection ***
        client.subscribe(Config::topic_command);
        client.subscribe(Config::topic_brightness);
        client.subscribe(Config::topic_image);

        Serial.printf("Subscribed to: %s\n", Config::topic_command);
        Serial.printf("Subscribed to: %s\n", Config::topic_brightness);
        Serial.printf("Subscribed to: %s\n", Config::topic_image);

        return true;
    } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        return false;
    }
}


// --- Public Functions (declared in mqtt.h) ---

void mqtt_setup() {
    client.setServer(Config::broker_host, Config::broker_port);
    client.setCallback(mqtt_callback);
    // Note: PubSubClient by default has a 256 byte buffer. If your images are larger,
    // you might need to increase this.
    // client.setBufferSize(2048); // Example: for larger payloads
}

void mqtt_loop() {
    if (!client.connected()) {
        long now = millis();
        // Try to reconnect every 5 seconds
        if (now - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = now;
            // Attempt to reconnect
            if (reconnect()) {
                lastReconnectAttempt = 0; // Reset timer on successful connect
            }
        }
    } else {
        // If connected, process messages
        client.loop();
    }
}

void publish_status(const char* message) {
    if (client.connected()) {
        client.publish(Config::topic_status, message);
    }
}

void publish_brightness(uint8_t brightness) {
    if (client.connected()) {
        char buffer[4]; // Max "255" + null terminator
        snprintf(buffer, sizeof(buffer), "%d", brightness);
        client.publish(Config::topic_brightness, buffer, true); // Retained message
    }
}

void register_image_update_callback(ImageUpdateCallback callback) {
    image_update_handler = callback;
}

bool is_mqtt_connected() {
    return client.connected();
}