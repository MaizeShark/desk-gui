// Spotify Shit

#include "spotify.h"
#include <Arduino.h>
#include <SpotifyEsp32.h>
#include <ArduinoJson.h>

Spotify sp(Config::spotify_client_id, Config::spotify_client_secret, Config::spotify_refresh_token);

void spotify_setup() {
    sp.begin();
    // Let's test it by getting the access token right away
    if (sp.get_access_token()) {
        Serial.println("Successfully got access token!");
    } else {
        Serial.println("Failed to get access token.");
    }
}

void test_spotify() {
    JsonDocument doc;
    JsonDocument filter;

    filter["progress_ms"] = true;                  // Get the current playback progress
    filter["item"]["duration_ms"] = true;          // Get the total duration of the track
    filter["device"]["volume_percent"] = true;     // Get the volume of the active device
    filter["is_playing"] = true;                   // It's also useful to know if it's playing

    response response = sp.current_playback_state(filter);
    Serial.println("Spotify Playback State:");
    if (response.status_code == 200 && !response.reply.isNull()) {
        
        // Print the small, filtered JSON we received
        serializeJsonPretty(response.reply, Serial);
        Serial.println();

        // Extract the values into variables.
        // If a key doesn't exist (e.g., volume on a device that doesn't support it),
        // ArduinoJson will safely return 0 or nullptr.
        long progress_ms = response.reply["progress_ms"];
        long duration_ms = response.reply["item"]["duration_ms"];
        int volume_percent = response.reply["device"]["volume_percent"];
        bool is_playing = response.reply["is_playing"];

        // --- 4. Use the Data ---
        Serial.println("Parsed Information:");
        Serial.printf("  Status: %s\n", is_playing ? "Playing" : "Paused");
        Serial.printf("  Volume: %d%%\n", volume_percent);
        Serial.printf("  Progress: %ld ms / %ld ms\n", progress_ms, duration_ms);
        Serial.printf("  Progress: %02ld:%02ld / %02ld:%02ld\n",
                      (progress_ms / 1000) / 60, (progress_ms / 1000) % 60,
                      (duration_ms / 1000) / 60, (duration_ms / 1000) % 60);

    } else {
        // This will happen if no music is playing (Spotify returns HTTP 204 No Content)
        // or if there was an error.
        Serial.println("Could not get playback state.");
        Serial.printf("Status Code: %d\n", response.status_code);
    }
    Serial.println("----------------------------");
}