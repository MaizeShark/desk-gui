// src/frontend_ui/mqtt.h

#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include "globals.h"

// Define a function pointer type for our image update callback.
// This allows main.cpp to provide a function that knows how to handle the image data.
// It will receive the raw image data (payload) and its length.
using ImageUpdateCallback = void (*)(const byte* payload, unsigned int length);

/**
 * @brief Initializes the MQTT client and sets up the server and callback.
 *        Does NOT connect yet. Call from your main setup().
 */
void mqtt_setup();

/**
 * @brief Connects/reconnects to the MQTT broker and subscribes to topics.
 *        This function should be called from the main loop() to handle disconnections.
 */
void mqtt_loop();

/**
 * @brief Publishes a status message to Config::topic_status.
 * @param message The null-terminated string to publish.
 */
void publish_status(const char* message);

/**
 * @brief Publishes the current brightness value to Config::topic_brightness.
 * @param brightness The brightness value (0-255).
 */
void publish_brightness(uint8_t brightness);

/**
 * @brief Registers the function to be called when a new image is received.
 * @param callback The function to call.
 */
void register_image_update_callback(ImageUpdateCallback callback);

/**
 * @brief Checks if the MQTT client is currently connected.
 * @return true if connected, false otherwise.
 */
bool is_mqtt_connected();

#endif // MQTT_H