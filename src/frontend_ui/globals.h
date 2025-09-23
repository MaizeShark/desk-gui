#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <lvgl.h>
#include "LGFX_ESP32_S3_MSP4031.hpp"
#include "FastLED.h"
#include "TCA9555.h"

// =========================================================================
// MQTT & WIFI CONFIGURATION
// =========================================================================
namespace Config {
    // --- Wi-Fi Credentials ---
    constexpr const char* ssid = "your-ssid";
    constexpr const char* pass = "your-password";

    // --- MQTT Broker Settings ---
    constexpr const char* broker_host = "broker.hivemq.com";
    constexpr uint16_t    broker_port = 1883;
    constexpr const char* broker_user = "";
    constexpr const char* broker_pass = "";
}

// =========================================================================
// HARDWARE CONFIGURATION
// =========================================================================

// --- Display & Touch ---
extern LGFX my_lcd;
static const uint32_t screenWidth  = 480;
static const uint32_t screenHeight = 320;
extern lv_color_t buf[screenWidth * 20];

// --- FastLED Ring ---
#define LED_DATA_PIN   39
#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB
#define NUM_LEDS       12
extern CRGB leds[NUM_LEDS];

// --- TCA9535 I/O Expander ---
#define I2C_SDA_PIN 41
#define I2C_SCL_PIN 40
extern TCA9535 TCA;

// --- Input Pin Assignments ---
#define ENCODER_A_PIN   0
#define ENCODER_B_PIN   1
#define ENCODER_SW_PIN  2
#define BUTTON_1_PIN    3 // Back Switch
#define BUTTON_2_PIN    4 // Mode Switch

// =========================================================================
// LVGL & UI GLOBALS
// =========================================================================
extern lv_obj_t *ui_arc;
extern lv_obj_t *ui_value_label;
extern lv_obj_t *ui_mode_label;
extern lv_obj_t *ui_power_switch;
extern lv_group_t *encoder_group;

// =========================================================================
// GLOBAL STATE VARIABLES
// =========================================================================
extern int currentMode;
extern const int totalModes;
extern const char *modeNames[];

extern long encoderValue;
extern bool ledsOn;

// For hardware input reading
extern uint8_t lastPinA_State;
extern uint8_t lastEncSwitchState;
extern uint8_t lastButton1State;
extern uint8_t lastButton2State;

// For LVGL encoder driver
extern long last_lvgl_encoder_val;

#endif // GLOBALS_H