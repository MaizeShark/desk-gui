#include "music_player.h"
#include "globals.h"
#include "mqtt.h"
#include <HTTPClient.h>
#include <string.h> // For strncpy

// --- Task Communication ---
struct MusicInfo {
    char url[256];
    char track[128];
    char artist[128];
};

static QueueHandle_t music_info_queue;

// --- Private Data ---
constexpr size_t MAX_IMAGE_SIZE = 200 * 1024;
static uint8_t* image_download_buffer = nullptr;
static lv_img_dsc_t artwork_img_dsc;
// A static copy of the latest info for LVGL async callbacks to safely access
static MusicInfo static_info_for_lvgl;

// =========================================================================
// ASYNC LVGL CALLBACKS
// =========================================================================
static void update_text_labels_cb(void* user_data) {
    lv_label_set_text(ui_track_label, static_info_for_lvgl.track);
    lv_label_set_text(ui_artist_label, static_info_for_lvgl.artist);
}

static void update_artwork_cb(void* user_data) {
    Serial.println("[LVGL] Async call: Updating artwork.");
    // Pass the pointer to the image descriptor. LVGL will use the data pointer
    // and size from this struct to decode and display the image.
    lv_img_set_src(ui_album_art, &artwork_img_dsc);
}

// =========================================================================
// THE DOWNLOAD TASK - This runs on Core 0
// =========================================================================
void download_image_task(void* parameter) {
    // --- Retry Parameters ---
    constexpr int MAX_DOWNLOAD_RETRIES = 3;
    constexpr int RETRY_DELAY_MS = 1000;

    MusicInfo info;

    while (true) {
        if (xQueueReceive(music_info_queue, &info, portMAX_DELAY)) {
            Serial.printf("[Task] Received new info. Downloading from %s\n", info.url);

            static_info_for_lvgl = info;
            lv_async_call(update_text_labels_cb, NULL);

            bool download_success = false;

            // --- THE RETRY LOOP (without the mutex) ---
            for (int i = 0; i < MAX_DOWNLOAD_RETRIES; i++) {
                Serial.printf("[Task] Download attempt %d/%d...\n", i + 1, MAX_DOWNLOAD_RETRIES);
                
                HTTPClient http;
                http.setTimeout(10000);
                if (http.begin(info.url)) {
                    vTaskDelay(pdMS_TO_TICKS(5)); // Small delay to let the connection stabilize
                    int httpCode = http.GET();
                    if (httpCode == HTTP_CODE_OK) {
                        int len = http.getSize(); // Total size of the image

                        if (len > 0 && len <= MAX_IMAGE_SIZE) {
                            WiFiClient* stream = http.getStreamPtr();
                            
                            // --- ROBUST CHUNKED READING ---
                            int bytes_read = 0;
                            while (bytes_read < len) {
                                // Wait for data to be available or timeout
                                if (stream->available()) {
                                    // Read a chunk of data. The read() function is non-blocking here.
                                    // We read into the buffer at the correct offset.
                                    int bytes_to_read = stream->read(image_download_buffer + bytes_read, len - bytes_read);
                                    if (bytes_to_read > 0) {
                                        bytes_read += bytes_to_read;
                                    }
                                } else {
                                    // A small delay to prevent a tight loop from starving other tasks
                                    vTaskDelay(pdMS_TO_TICKS(10));
                                }
                            }
                            // --- END OF CHUNKED READING ---

                            Serial.printf("[Task] Image downloaded, %d bytes.\n", bytes_read);

                            // Important: Only update LVGL if the download was complete
                            if(bytes_read == len) {
                                // --- PNG Header Verification ---
                                const uint8_t png_header[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
                                if (memcmp(image_download_buffer, png_header, sizeof(png_header)) == 0) {
                                    // --- Descriptor Update ---
                                    // Clear the struct to avoid any garbage data in the header
                                    memset(&artwork_img_dsc, 0, sizeof(lv_img_dsc_t));
                                    artwork_img_dsc.data = image_download_buffer;
                                    artwork_img_dsc.data_size = len;
                                    
                                    lv_async_call(update_artwork_cb, NULL);
                                    download_success = true;
                                } else {
                                    Serial.println("[Task] ERROR: Downloaded file is not a valid PNG.");
                                    download_success = false; // Ensure we report failure
                                }
                            } else {
                                Serial.printf("[Task] Download incomplete! Expected %d, got %d.\n", len, bytes_read);
                            }

                        } else { Serial.printf("[Task] Image size invalid (%d) or too large.\n", len); }
                    } else { Serial.printf("[Task] HTTP GET failed, code: %d\n", httpCode); }
                    http.end();
                } else { Serial.printf("[Task] Unable to connect to %s\n", info.url); }

                if (download_success) {
                    break; 
                }

                if (i < MAX_DOWNLOAD_RETRIES - 1) {
                    Serial.printf("...waiting %dms before retry.\n", RETRY_DELAY_MS);
                    vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
                }
            }

            if (!download_success) {
                Serial.printf("[Task] Failed to download image after %d attempts.\n", MAX_DOWNLOAD_RETRIES);
            }
        }
    }
}


// =========================================================================
// MQTT CALLBACK - This runs on Core 1 (very fast!)
// =========================================================================
static void on_music_info_update(const char* url, const char* track, const char* artist) {
    Serial.println("MQTT callback received info, queuing for download task...");
    
    MusicInfo new_info = {0};

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