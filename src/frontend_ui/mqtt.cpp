// src/mqtt.cpp

#include "mqtt.h"
#include "globals.h" // For access to ui elements and global state
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h> // Needed for MAC address

// --- Configuration ---
const long MQTT_RECONNECT_INTERVAL_MS = 5000;
#define MAX_MQTT_PAYLOAD_SIZE 256 // Increased slightly for safety with JSON

// --- Client Objects ---
WiFiClient espClient;
PubSubClient client(espClient);

// --- State Variables ---
static unsigned long lastReconnectAttempt = 0;
static MusicInfoUpdateCallback music_info_handler = nullptr;

// --- Forward Declarations for Internal Functions ---
void mqtt_callback(char* topic, byte* payload, unsigned int length);
bool mqtt_connect_attempt();
void log_mqtt_error(int8_t state);

// =========================================================================
// PUBLIC FUNCTIONS (as defined in mqtt.h)
// =========================================================================

void mqtt_setup() {
    client.setServer(Config::broker_host, Config::broker_port);
    client.setCallback(mqtt_callback);
    Serial.println("[MQTT] Client configured.");
}

void mqtt_loop() {
    // If we are connected, just run the client loop and exit.
    if (client.connected()) {
        client.loop();
        return;
    }

    // If we get here, we are not connected.
    // First, check if WiFi is even up. No point trying if it's not.
    if (WiFi.status() != WL_CONNECTED) {
        return; 
    }

    // If WiFi is connected, but MQTT is not, try to reconnect periodically.
    unsigned long now = millis();
    if (now - lastReconnectAttempt > MQTT_RECONNECT_INTERVAL_MS) {
        lastReconnectAttempt = now;
        // Attempt to reconnect (this is our new non-blocking function)
        if (mqtt_connect_attempt()) {
            // Success! Reset timer to allow immediate re-check if it disconnects again.
            lastReconnectAttempt = 0;
        }
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

// =========================================================================
// INTERNAL "HELPER" FUNCTIONS
// =========================================================================

/**
 * @brief The master callback function for handling all incoming MQTT messages.
 */
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    // For safety, let's copy the payload to a null-terminated buffer on the stack
    char msg_buffer[MAX_MQTT_PAYLOAD_SIZE + 1];
    unsigned int len_to_copy = min((unsigned int)MAX_MQTT_PAYLOAD_SIZE, length);
    memcpy(msg_buffer, payload, len_to_copy);
    msg_buffer[len_to_copy] = '\0';
    
    #ifdef DEBUG_MQTT
        Serial.printf("[MQTT] Message on topic: %s\n", topic);
        Serial.printf("[MQTT] Payload: %s\n", msg_buffer);
    #endif

    // --- Handle Image Topic (JSON payload) ---
    if (strcmp(topic, Config::topic_image) == 0) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload, length);
        if (error) {
            Serial.printf("[MQTT] JSON Error: %s\n", error.c_str());
            return;
        }
        if (music_info_handler && doc["url"] && doc["track"] && doc["artist"]) {
            music_info_handler(doc["url"], doc["track"], doc["artist"]);
        }
        return;
    }

    // --- Handle Music Status Topic (JSON payload) ---
    if (strcmp(topic, Config::topic_music) == 0) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload, length);
        if (error) {
            Serial.printf("[MQTT] JSON Error: %s\n", error.c_str());
            return;
        }
        if (doc["length"].is<int>() && doc["elapsed"].is<int>()) {
            int len_sec = doc["length"];
            int pos_sec = doc["elapsed"];
            
            // OPTIMIZATION: Use lv_label_set_text_fmt to avoid creating temporary String objects
            lv_label_set_text_fmt(ui_position_label, "%d:%02d", pos_sec / 60, pos_sec % 60);
            lv_label_set_text_fmt(ui_length_label, "%d:%02d", len_sec / 60, len_sec % 60);

            lv_slider_set_range(ui_progress_bar, 0, len_sec > 0 ? len_sec : 1);
            lv_slider_set_value(ui_progress_bar, constrain(pos_sec, 0, len_sec), LV_ANIM_ON);
        }
        return;
    }

    // --- Handle Brightness Topic ---
    if (strcmp(topic, Config::topic_brightness) == 0) {
        int brightness = atoi(msg_buffer);
        my_lcd.setBrightness(constrain(brightness, 0, 255));
    }

    // --- Handle Command Topic ---
    else if (strcmp(topic, Config::topic_command) == 0) {
        if (strcasecmp(msg_buffer, "reboot") == 0) {
            publish_status("rebooting");
            delay(200);
            ESP.restart();
        } else if (strcasecmp(msg_buffer, "led_on") == 0) {
            ledsOn = true;
        } else if (strcasecmp(msg_buffer, "led_off") == 0) {
            ledsOn = false;
        }
    }
}

/**
 * @brief Attempts to connect to the MQTT broker ONCE and returns immediately.
 * @return True on success, false on failure.
 */
bool mqtt_connect_attempt() {
    Serial.print("[MQTT] Attempting connection...");

    // OPTIMIZATION: Use the device's MAC address for a unique client ID.
    // This avoids String allocations and potential client ID collisions.
    String clientId = "ESP32-Music-";
    clientId += WiFi.macAddress();
    
    if (client.connect(clientId.c_str(), Config::broker_user, Config::broker_pass)) {
        Serial.println(" Connected!");
        publish_status("online");

        // Resubscribe to all topics upon (re)connection
        client.subscribe(Config::topic_command);
        client.subscribe(Config::topic_brightness);
        client.subscribe(Config::topic_image);
        client.subscribe(Config::topic_music);
        
        Serial.println("[MQTT] Subscribed to all topics.");
        return true;
    } else {
        // Use our helper to print a detailed error message
        log_mqtt_error(client.state());
        return false;
    }
}

/**
 * @brief Prints a human-readable error for a given PubSubClient state code.
 */
void log_mqtt_error(int8_t state) {
    Serial.print("[MQTT] Failed, state code: ");
    Serial.print(state);
    switch (state) {
        case -4: Serial.println(" (The server didn't respond within the keep-alive time.)"); break;
        case -3: Serial.println(" (The connection was broken.)"); break;
        case -2: Serial.println(" (The connection was refused.)"); break;
        case -1: Serial.println(" (The network connection failed.)"); break;
        case 1:  Serial.println(" (Connection refused: unacceptable protocol version.)"); break;
        case 2:  Serial.println(" (Connection refused: identifier rejected.)"); break;
        case 3:  Serial.println(" (Connection refused: server unavailable.)"); break;
        case 4:  Serial.println(" (Connection refused: bad user name or password.)"); break;
        case 5:  Serial.println(" (Connection refused: not authorized.)"); break;
        default: Serial.println(" (Unknown error.)"); break;
    }
}