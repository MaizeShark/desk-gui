#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <lvgl.h>
#include "LGFX_ESP32_S3_MSP4031.hpp"
#include "FastLED.h"
#include "TCA9555.h"

// =========================================================================
// APPLICATION CONFIGURATION (Passwords, Keys, etc.)
// =========================================================================
namespace Config {
    // --- Wi-Fi Credentials ---
    constexpr const char* ssid = "Ironcloud9";
    constexpr const char* pass = "Nee25022010#*";

    // --- MQTT Broker Settings ---
    constexpr const char* broker_host = "192.168.178.15";
    constexpr uint16_t    broker_port = 1883;
    constexpr const char* broker_user = "mqtt";
    constexpr const char* broker_pass = "mqtt";

    // --- MQTT Topics ---
    constexpr const char* topic_status           = "esp-gui/status";
    constexpr const char* topic_command          = "esp-gui/command";
    constexpr const char* topic_brightness       = "esp-gui/brightness";
    constexpr const char* topic_image            = "music/image";
}

// =========================================================================
// HARDWARE CONSTANTS (Pins, Dimensions, etc.)
// =========================================================================
namespace HW {
    // --- Display ---
    constexpr uint32_t screenWidth  = 480;
    constexpr uint32_t screenHeight = 320;

    // --- FastLED Ring ---
    constexpr uint8_t LED_DATA_PIN = 39;
    constexpr int     NUM_LEDS     = 12;
    #define LED_TYPE    WS2812B   // These are symbols for the library, so #define is appropriate
    #define COLOR_ORDER GRB

    // --- I2C ---
    constexpr uint8_t I2C_SDA_PIN = 41;
    constexpr uint8_t I2C_SCL_PIN = 40;
    constexpr uint8_t TCA_I2C_ADDR = 0x20;

    // --- I/O Expander Pin Assignments ---
    constexpr uint8_t ENCODER_A_PIN   = 0;
    constexpr uint8_t ENCODER_B_PIN   = 1;
    constexpr uint8_t ENCODER_SW_PIN  = 2;
    constexpr uint8_t BUTTON_1_PIN    = 3; // Back Switch
    constexpr uint8_t BUTTON_2_PIN    = 4; // Mode Switch
}

// =========================================================================
// GLOBAL OBJECT & VARIABLE DECLARATIONS ('extern')
// These are just promises; the memory is allocated in main.cpp
// =========================================================================

// --- Global Hardware Objects & Buffers ---
extern LGFX my_lcd;
extern TCA9535 TCA;
extern CRGB leds[HW::NUM_LEDS];
extern lv_color_t* buf;

// --- LVGL UI Objects ---
// LED Control UI Elements
extern lv_obj_t *ui_arc;
extern lv_obj_t *ui_value_label;
extern lv_obj_t *ui_mode_label;
extern lv_obj_t *ui_power_switch;

// --- Music Info UI Elements ---
extern lv_obj_t *ui_album_art;
extern lv_obj_t *ui_track_label;
extern lv_obj_t *ui_artist_label;

extern lv_group_t *encoder_group;

// --- Global State Variables ---
extern int currentMode;
extern const int totalModes;
extern const char *modeNames[];

extern long encoderValue;
extern bool ledsOn;

// For tracking hardware input state
extern uint8_t lastPinA_State;
extern uint8_t lastEncSwitchState;
extern uint8_t lastButton1State;
extern uint8_t lastButton2State;

// For LVGL encoder driver
extern long last_lvgl_encoder_val;

#endif // GLOBALS_H