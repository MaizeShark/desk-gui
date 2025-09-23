// src/music_player.cpp

#include "music_player.h"
#include "globals.h"
#include "mqtt.h"
#include <HTTPClient.h>
#include <string.h> // For strncpy

// --- Task Communication ---
// Use fixed-size char arrays. This is thread-safe and avoids dynamic memory issues.
struct MusicInfo {
    char url[256];    // A generous buffer for the URL
    char track[128];  // Buffer for track name
    char artist[128]; // Buffer for artist name
};

static QueueHandle_t music_info_queue;

// --- Private Data ---
constexpr size_t MAX_IMAGE_SIZE = 200 * 1024;
static uint8_t* image_download_buffer = nullptr;
static lv_img_dsc_t artwork_img_dsc;

// =========================================================================
// ASYNC LVGL CALLBACKS
// =========================================================================
static void update_artwork_cb(void* user_data) {
    Serial.println("[LVGL] Async call: Updating artwork.");
    lv_img_set_src(ui_album_art, &artwork_img_dsc);
}

// =========================================================================
// THE DOWNLOAD TASK - This runs on Core 0
// =========================================================================
void download_image_task(void* parameter) {
    MusicInfo info;

    while (true) {
        if (xQueueReceive(music_info_queue, &info, portMAX_DELAY)) {
            Serial.printf("[Task] Received new info. Downloading from %s\n", info.url);

            // --- Safely update text labels IMMEDIATELY ---
            lv_async_call([](void* data){
                MusicInfo* p_info = (MusicInfo*)data;
                lv_label_set_text(ui_track_label, p_info->track);
                lv_label_set_text(ui_artist_label, p_info->artist);
                free(data); // Free the memory allocated in the MQTT callback
            }, strdup(reinterpret_cast<const char*>(&info)));


            // --- Perform the BLOCKING download ---
            HTTPClient http;
            if (http.begin(info.url)) { // Pass the valid C-string
                int httpCode = http.GET();
                if (httpCode == HTTP_CODE_OK) {
                    int len = http.getSize();
                    if (len > 0 && len <= MAX_IMAGE_SIZE) {
                        WiFiClient* stream = http.getStreamPtr();
                        stream->readBytes(image_download_buffer, len);
                        Serial.printf("[Task] Image downloaded, %d bytes.\n", len);

                        artwork_img_dsc.data = image_download_buffer;
                        artwork_img_dsc.data_size = len;
                        lv_async_call(update_artwork_cb, NULL);

                    } else { Serial.printf("[Task] Image size invalid (%d) or too large.\n", len); }
                } else { Serial.printf("[Task] HTTP GET failed, code: %d\n", httpCode); }
                http.end();
            } else { Serial.printf("[Task] Unable to connect to %s\n", info.url); }
        }
    }
}

// =========================================================================
// MQTT CALLBACK - This runs on Core 1 (very fast!)
// =========================================================================
static void on_music_info_update(const char* url, const char* track, const char* artist) {
    Serial.println("MQTT callback received info, queuing for download task...");
    
    MusicInfo new_info = {0}; // Initialize struct to all zeros

    // Safely copy the strings into our struct's fixed-size buffers
    strncpy(new_info.url, url, sizeof(new_info.url) - 1);
    strncpy(new_info.track, track, sizeof(new_info.track) - 1);
    strncpy(new_info.artist, artist, sizeof(new_info.artist) - 1);

    xQueueSend(music_info_queue, &new_info, portMAX_DELAY);
}

// =========================================================================
// PUBLIC INIT FUNCTION
// =========================================================================
void music_player_init() {
    Serial.println("Music Player module initializing...");

    image_download_buffer = (uint8_t*) ps_malloc(MAX_IMAGE_SIZE);
    if (image_download_buffer == nullptr) {
        Serial.println("FATAL: Failed to allocate image buffer in PSRAM!");
        return;
    }
    Serial.printf("Successfully allocated %d KB image buffer in PSRAM.\n", MAX_IMAGE_SIZE / 1024);

    music_info_queue = xQueueCreate(5, sizeof(MusicInfo));

    xTaskCreatePinnedToCore(
        download_image_task, "ImageDownloader", 8192, NULL, 1, NULL, 0
    );

    register_music_info_update_callback(on_music_info_update);
}