// src/main.cpp

#include "globals.h"
#include "lvgl_handler.h"
#include "ui.h"
#include "hardware.h"
#include "mqtt.h"
#include <WiFi.h>
#include "music_player.h"
#include "config.h"

// =========================================================================
// DEFINITIONS of Global Variables (declared as 'extern' in globals.h)
// =========================================================================

// --- Hardware Objects & Buffers ---
LGFX my_lcd;
TCA9535 TCA(HW::TCA_I2C_ADDR);
CRGB leds[HW::NUM_LEDS];
lv_color_t* buf = nullptr;
lv_obj_t *ui_Screen1 = nullptr, *ui_Screen2 = nullptr, *ui_arc = nullptr,
         *ui_value_label = nullptr, *ui_mode_label = nullptr, *ui_power_switch = nullptr,
         *ui_album_art = nullptr, *ui_length_label = nullptr, *ui_position_label = nullptr,
         *ui_progress_bar = nullptr;

lv_group_t *encoder_group = nullptr;
int currentMode = 0;
const char *modeNames[] = {"Brightness", "Color Hue", "Position"};
const int totalModes = sizeof(modeNames) / sizeof(modeNames[0]); // Calculate number of modes

long encoderValue = 50;
bool ledsOn = false;
uint8_t lastPinA_State = HIGH, lastEncSwitchState = HIGH, lastButton1State = HIGH, lastButton2State = HIGH;
long last_lvgl_encoder_val = 0;

// =========================================================================
// WIFI & NETWORK EVENT HANDLING
// =========================================================================

// Forward declarations for WiFi event handlers
void onWiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void onWiFiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info);

void setup_wifi_robust() {
    // Register event handlers BEFORE connecting
    WiFi.onEvent(onWiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(onWiFiDisconnect, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.mode(WIFI_STA);
    WiFi.begin(Config::ssid, Config::pass);
    Serial.printf("[Setup] Connecting to %s ", Config::ssid);
}

void onWiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.println("\n[Setup] WiFi connected!");
    Serial.print("[Setup] IP address: ");
    Serial.println(WiFi.localIP());

    mqtt_setup();
}

// This function is CALLED AUTOMATICALLY when WiFi disconnects
void onWiFiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.println("\n[Setup] WiFi lost connection. Reconnecting...");
    // The ESP32 WiFi library will attempt to reconnect automatically.
}

// =========================================================================
// MAIN SETUP & LOOP
// =========================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("[Setup] Booting up...");

    // STEP 1: Connect to the network first.
    setup_wifi_robust();

    // STEP 2: Initialize hardware, display, and UI.
    hardware_init();
    lvgl_init();
    ui_init();
    
    // STEP 3: Initialize other application logic.
    music_player_init();

    Serial.println("[Setup] Setup complete. Main loop is starting.");
}

void loop() {
    mqtt_loop();
    handle_hardware_inputs();
    sync_ui_with_state();
    update_leds();
    FastLED.show();
    
    lv_timer_handler();
    // A small delay is crucial to prevent starving the idle task,
    delay(5);
}