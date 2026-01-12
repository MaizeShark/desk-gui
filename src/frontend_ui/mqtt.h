// src/frontend_ui/mqtt.h

#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include "globals.h"
#include "config.h"

using MusicInfoUpdateCallback = void (*)(const char* url, const char* track, const char* artist);

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
 * @brief Publishes the current elapsed time value to Config::topic_position_set.
 * @param elapsed The elapsed time in seconds.
 */
void publish_elapsed_time(uint16_t elapsed);

/**
 * @brief Publishes the current volume value to Config::topic_volume_set if in Volume mode.
 * @param volume The volume value (0-100).
 */
void update_volume(long volume);

/**
 * @brief Checks if the MQTT client is currently connected.
 * @return true if connected, false otherwise.
 */
bool is_mqtt_connected();

/**
 * @brief Registers the function to be called when new music info is received
 * @param callback The function to call with the parsed URL, track, and artist.
 */
void register_music_info_update_callback(MusicInfoUpdateCallback callback);

#endif // MQTT_H