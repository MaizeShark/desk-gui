#include <Arduino.h>
#include "globals.h"
#include "lvgl_handler.h"
#include "ui.h"
#include "hardware.h"

// =========================================================================
// DEFINITIONS of Global Variables (declared as 'extern' in globals.h)
// This is where the memory for them is actually allocated.
// =========================================================================
LGFX my_lcd;
lv_color_t buf[screenWidth * 20];
CRGB leds[NUM_LEDS];
TCA9535 TCA(0x20);

lv_obj_t *ui_arc;
lv_obj_t *ui_value_label;
lv_obj_t *ui_mode_label;
lv_obj_t *ui_power_switch;
lv_group_t *encoder_group;

int currentMode = 0;
const int totalModes = 3;
const char *modeNames[] = {"Brightness", "Color Hue", "Position"};

long encoderValue = 50;
bool ledsOn = true;

uint8_t lastPinA_State = HIGH;
uint8_t lastEncSwitchState = HIGH;
uint8_t lastButton1State = HIGH;
uint8_t lastButton2State = HIGH;

long last_lvgl_encoder_val = 0;

// =========================================================================
// MAIN SETUP & LOOP
// =========================================================================

void setup() {
    Serial.begin(115200);
    Serial.println("Starting up...");

    hardware_init();  // Initialize physical devices
    lvgl_init();      // Initialize LVGL and its drivers
    ui_init();        // Create the user interface

    Serial.println("Setup complete.");
}

void loop() {
    handle_hardware_inputs(); // Read physical encoder and buttons
    sync_ui_with_state();     // Update the on-screen widgets to match our state
    update_leds();            // Update the physical LEDs to match our state
    FastLED.show();           // Send data to the LED strip
    
    lv_timer_handler();       // Let the GUI do its work
    delay(5);
}