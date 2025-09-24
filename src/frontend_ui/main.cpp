#include "globals.h"
#include "lvgl_handler.h"
#include "ui.h"
#include "hardware.h"
#include "mqtt.h"
#include <WiFi.h>
#include "music_player.h"
#include "spotify.h"
#include "config.h"

// =========================================================================
// DEFINITIONS of Global Variables (declared as 'extern' in globals.h)
// This is where the memory for them is actually allocated.
// =========================================================================

// --- Hardware Objects & Buffers ---
LGFX my_lcd;
TCA9535 TCA(HW::TCA_I2C_ADDR);
CRGB leds[HW::NUM_LEDS];
lv_color_t* buf = nullptr;

// --- LVGL UI Objects (initialized to nullptr for safety) ---
lv_obj_t *ui_arc          = nullptr;
lv_obj_t *ui_value_label  = nullptr;
lv_obj_t *ui_mode_label   = nullptr;
lv_obj_t *ui_power_switch = nullptr;
lv_group_t *encoder_group = nullptr;

// Music Info UI Elements
lv_obj_t *ui_album_art;
lv_obj_t *ui_track_label;
lv_obj_t *ui_artist_label;

// --- Global State Variables ---
int currentMode = 0;
const char *modeNames[] = {"Brightness", "Color Hue", "Position"};
// This automatically calculates the number of modes. Robust!
const int totalModes = sizeof(modeNames) / sizeof(modeNames[0]);

long encoderValue = 50;
bool ledsOn = true;

// Initial state for input polling
uint8_t lastPinA_State     = HIGH;
uint8_t lastEncSwitchState = HIGH;
uint8_t lastButton1State   = HIGH;
uint8_t lastButton2State   = HIGH;

long last_lvgl_encoder_val = 0;

// WiFi setup
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(Config::ssid);
    WiFi.begin(Config::ssid, Config::pass);
    int wifi_attempts = 0;
    while (WiFi.status() != WL_CONNECTED && wifi_attempts < 20) {
        delay(500);
        Serial.print(".");
        wifi_attempts++;
    }
    if (WiFi.status() != WL_CONNECTED && wifi_attempts >= 20) {
        Serial.println("\nFailed to connect to WiFi.");
        Serial.println("Please check your SSID and password in Config.h");
        Serial.println("Restarting in 2 seconds to reset...");
        delay(2000);
        ESP.restart();
    }
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

// =========================================================================
// MAIN SETUP & LOOP
// =========================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("Starting up...");

    hardware_init();
    lvgl_init();
    ui_init();
    setup_wifi();
    
    spotify_setup();
    test_spotify();

    mqtt_setup();
    music_player_init();

    Serial.println("Setup complete.");
}

void loop() {
    mqtt_loop();
    handle_hardware_inputs();
    sync_ui_with_state();
    update_leds();
    FastLED.show();
    
    lv_timer_handler();
    delay(5);
}