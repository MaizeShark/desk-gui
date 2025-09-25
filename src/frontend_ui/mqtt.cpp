// src/mqtt.cpp

#include "mqtt.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Create the underlying WiFi and MQTT client objects
WiFiClient espClient;
PubSubClient client(espClient);

// To hold the function pointer for the music update
static MusicInfoUpdateCallback music_info_handler = nullptr;

// For reconnection logic
long lastReconnectAttempt = 0;

/**
 * @brief The master callback function for handling all incoming MQTT messages.
 */
#define MAX_MQTT_PAYLOAD_SIZE 128 

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    #ifdef DEBUG_MQTT
        Serial.printf("[MQTT] Message arrived on topic: %s. Length: %u\n", topic, length);
        
        // --- Safe Debug Payload Printing ---
        // To print the payload, we copy it to a new, null-terminated buffer.
        // Use a fixed-size buffer to prevent stack overflow from a large payload.
        char msg_for_debug[MAX_MQTT_PAYLOAD_SIZE + 1];
        unsigned int len_to_copy = min((unsigned int)MAX_MQTT_PAYLOAD_SIZE, length);
        memcpy(msg_for_debug, payload, len_to_copy);
        msg_for_debug[len_to_copy] = '\0';
        Serial.printf("[MQTT] Message: %s%s\n", msg_for_debug, (length > MAX_MQTT_PAYLOAD_SIZE) ? "..." : "");
    #endif

    // --- Handle Image Topic (JSON payload) ---
    if (strcmp(topic, Config::topic_image) == 0) {
        JsonDocument doc;

        DeserializationError error = deserializeJson(doc, payload, length, DeserializationOption::NestingLimit(5));
        if (error) {
            Serial.print(F("[MQTT] deserializeJson() failed: "));
            Serial.println(error.c_str());
            return;
        }
        const char* url = doc["url"];
        const char* track = doc["track"];
        const char* artist = doc["artist"];

        if (url == nullptr || track == nullptr || artist == nullptr) {
            Serial.println("[MQTT] JSON received, but missing required fields (url, track, artist)");
            return;
        }

        if (music_info_handler != nullptr) {
            music_info_handler(url, track, artist);
        } else {
            Serial.println("[MQTT] Received music info, but no handler is registered!");
        }
        return; // Done
    }

    // --- Handle Music Status Topic ---
    else if (strcmp(topic, Config::topic_music) == 0) {
        JsonDocument doc;

        DeserializationError error = deserializeJson(doc, payload, length);
        if (error) {
            Serial.print(F("[MQTT] deserializeJson() on music/status failed: "));
            Serial.println(error.c_str());
            return;
        }

        // Check that we got the data we expect.
        // For numbers, using containsKey() is safer than checking for 0, since 0 can be a valid value.
        if (!doc["length"].is<int>() || !doc["elapsed"].is<int>()) {
            Serial.println("[MQTT] JSON on music/status missing required fields (length, elapsed)");
            return;
        }
        
        // Safely extract the integer values
        int track_length_secs = doc["length"];
        int track_elapsed_secs = doc["elapsed"];

        #ifdef DEBUG_MQTT
            Serial.printf("[MQTT] Music Status Update: %d / %d seconds\n", track_elapsed_secs, track_length_secs);
        #endif

        // A single, reusable buffer on the stack is highly efficient.
        // Size 12 is very safe for any reasonable time format.
        char time_buffer[12];

        // Format the total track length.
        // "%d:%02d" means: integer, literal ':', 2-digit integer padded with a leading zero.
        snprintf(time_buffer, sizeof(time_buffer), "%d:%02d", track_length_secs / 60, track_length_secs % 60);
        String track_length_formatted = String(time_buffer);

        // Reuse the same buffer to format the elapsed time.
        snprintf(time_buffer, sizeof(time_buffer), "%d:%02d", track_elapsed_secs / 60, track_elapsed_secs % 60);
        String track_elapsed_formatted = String(time_buffer);

        #ifdef DEBUG_MQTT
            Serial.printf("[MQTT] Formatted Times - Elapsed: %s, Length: %s\n", track_elapsed_formatted.c_str(), track_length_formatted.c_str());
        #endif

        lv_label_set_text(ui_position_label, track_elapsed_formatted.c_str());
        lv_label_set_text(ui_length_label, track_length_formatted.c_str());
        
        return; // Done
    }


    // --- Handle other topics (assume C-string payloads) ---

    // SAFE: Check length against our fixed-size buffer to prevent buffer overflow.
    if (length > MAX_MQTT_PAYLOAD_SIZE) {
        Serial.printf("[MQTT] Payload on topic '%s' is too large (%u bytes). Ignoring.\n", topic, length);
        return;
    }

    char msg_buffer[MAX_MQTT_PAYLOAD_SIZE + 1];
    memcpy(msg_buffer, payload, length);
    msg_buffer[length] = '\0';


    // --- Handle Brightness Topic ---
    if (strcmp(topic, Config::topic_brightness) == 0) {
        int brightness = atoi(msg_buffer);
        brightness = constrain(brightness, 0, 255); // Safety check
        Serial.printf("[SYSTEM] Setting display brightness to %d\n", brightness);
        my_lcd.setBrightness(brightness);
    }

    // --- Handle Command Topic ---
    else if (strcmp(topic, Config::topic_command) == 0) {
        Serial.printf("[MQTT] Received command: %s\n", msg_buffer);

        // EFFICIENT: Use strcasecmp() for case-insensitive comparison instead of String.equalsIgnoreCase()
        if (strcasecmp(msg_buffer, "reboot") == 0) {
            Serial.println("[SYSTEM] Rebooting device...");
            publish_status("[SYSTEM] rebooting");
            delay(200); // Allow time for MQTT message to send
            ESP.restart();
        } 
        else if (strcasecmp(msg_buffer, "led_on") == 0) {
            Serial.println("[SYSTEM] Turning LEDs on");
            ledsOn = true;
        }
        else if (strcasecmp(msg_buffer, "led_off") == 0) {
            Serial.println("[SYSTEM] Turning LEDs off");
            ledsOn = false;
        }
        //TODO: Add more else-if blocks for other commands
        else {
            Serial.println("[MQTT] Unknown command");
        }
    }
}


/**
 * @brief Attempts to connect to the MQTT broker and subscribe to topics.
 */
bool reconnect() {
    String clientId = "esp32-gui-";
    clientId += String(random(0xffff), HEX);

    Serial.print("[MQTT] Attempting MQTT connection...");
    if (client.connect(clientId.c_str(), Config::broker_user, Config::broker_pass)) {
        Serial.println("connected!");

        // Publish an "online" status message
        publish_status("online");

        // *** Resubscribe to all topics upon reconnection ***
        client.subscribe(Config::topic_command);
        client.subscribe(Config::topic_brightness);
        client.subscribe(Config::topic_image);
        client.subscribe(Config::topic_music);
        
        #ifdef DEBUG_MQTT
            Serial.println("[MQTT] Subscribed to topics:");
            Serial.printf("[MQTT] Subscribed to: %s\n", Config::topic_command);
            Serial.printf("[MQTT] Subscribed to: %s\n", Config::topic_brightness);
            Serial.printf("[MQTT] Subscribed to: %s\n", Config::topic_image);
            Serial.printf("[MQTT] Subscribed to: %s\n", Config::topic_music);
        #endif

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

void register_music_info_update_callback(MusicInfoUpdateCallback callback) {
    music_info_handler = callback;
}

bool is_mqtt_connected() {
    return client.connected();
}