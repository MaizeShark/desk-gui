#ifndef GLOBALS_H
#define GLOBALS_H
// #define DEBUG_MQTT // Uncomment to enable verbose MQTT debug output

#include <Arduino.h>
#include <lvgl.h>
#include "LGFX_ESP32_S3_MSP4031.hpp"
#include "FastLED.h"
#include "TCA9555.h"
#include "config.h"

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

// Top-level Screen Objects
extern lv_obj_t *ui_Screen1;
extern lv_obj_t *ui_Screen2;

// --- Screen 1 UI Elements ---
extern lv_obj_t *ui_album_art;
extern lv_obj_t *ui_length_label;
extern lv_obj_t *ui_position_label;
extern lv_obj_t *ui_progress_bar;

// --- Screen 2 UI Elements ---
extern lv_obj_t *ui_arc;
extern lv_obj_t *ui_value_label;
extern lv_obj_t *ui_mode_label;
extern lv_obj_t *ui_power_switch;

extern lv_group_t *encoder_group;

// --- Global State Variables ---
extern int currentMode;
extern const int totalModes;
extern const char *modeNames[];

extern long encoderValue;
extern bool ledsOn;

// For tracking hardware input state
extern uint8_t lastPinA_State;
extern uint8_t lastPinB_State;
extern uint8_t lastEncSwitchState;
extern uint8_t lastButton1State;
extern uint8_t lastButton2State;

// For LVGL encoder driver
extern long last_lvgl_encoder_val;

#endif // GLOBALS_H