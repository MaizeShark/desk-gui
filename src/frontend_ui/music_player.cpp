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

// --- PNG Header for verification ---
static const uint8_t PNG_HEADER[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
const size_t PNG_HEADER_SIZE = sizeof(PNG_HEADER);

// =========================================================================
// ASYNC LVGL CALLBACKS
// =========================================================================
static void update_artwork_cb(void* user_data) {
    Serial.println("[LVGL] Async call: Updating artwork.");
    // Pass the pointer to the image descriptor. LVGL will use the data pointer
    // and size from this struct to decode and display the image.
    lv_img_set_src(ui_album_art, &artwork_img_dsc);
}

// =========================================================================
// THE DOWNLOAD TASK
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

            bool download_success = false;

            // --- THE RETRY LOOP (without the mutex) ---
            for (int i = 0; i < MAX_DOWNLOAD_RETRIES; i++) {
                if (i >= 1) { Serial.printf("[Task] Download attempt %d/%d...\n", i + 1, MAX_DOWNLOAD_RETRIES);};
                
                HTTPClient http;
                http.setTimeout(10000);
                if (http.begin(info.url)) {
                    http.setUserAgent("ESP32-Downloader/1.0");

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
                                }
                                vTaskDelay(pdMS_TO_TICKS(1)); // Yield to other tasks
                            }
                            // --- END OF CHUNKED READING ---

                            // Important: Only update LVGL if the download was complete
                            if(bytes_read == len) {
                                // --- PNG Header Verification ---
                                if (memcmp(image_download_buffer, PNG_HEADER, PNG_HEADER_SIZE) == 0) {

                                    Serial.printf("[Task] Image successfully downloaded, %d bytes.\n", bytes_read);
                                    // --- Descriptor Update ---
                                    // Clear the struct to avoid any garbage data in the header
                                    memset(&artwork_img_dsc, 0, sizeof(lv_img_dsc_t));
                                    artwork_img_dsc.data = image_download_buffer;
                                    artwork_img_dsc.data_size = len;

                                    lv_image_cache_drop(&artwork_img_dsc); // Clear any cached version of this image
                                    
                                    lv_async_call(update_artwork_cb, NULL);
                                    download_success = true;
                                } else {
                                    Serial.println("[Task] ERROR: Downloaded file is not a valid PNG (header mismatch).");
                                    Serial.print("[Task]   Expected Header: ");
                                    for(size_t i = 0; i < PNG_HEADER_SIZE; ++i) { Serial.printf("0x%02X ", PNG_HEADER[i]); }
                                    Serial.println();

                                    Serial.print("[Task]   Received Header: ");
                                    for(size_t i = 0; i < PNG_HEADER_SIZE; ++i) { Serial.printf("0x%02X ", image_download_buffer[i]); }
                                    Serial.println();
                                    download_success = false; // Ensure we report failure
                                }
                            } else {
                                Serial.printf("[Task] Download incomplete! Expected %d, got %d.\n", len, bytes_read);
                            }

                        } else { Serial.printf("[Task] Image size invalid (%d) or too large.\n", len); }
                    } else { Serial.printf("[Task] HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str()); }
                    http.end();
                } else { Serial.printf("[Task] Unable to connect to %s\n", info.url); }

                if (download_success) {
                    break; 
                }

                vTaskDelay(pdMS_TO_TICKS(50)); // Make sure Server has time.

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
// MQTT CALLBACK
// =========================================================================
static void on_music_info_update(const char* url, const char* track, const char* artist) {    
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