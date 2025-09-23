#include "globals.h"
#include "lvgl_handler.h"
#include "ui.h"
#include "hardware.h"
#include "mqtt.h"
#include <WiFi.h>

// =========================================================================
// DEFINITIONS of Global Variables (declared as 'extern' in globals.h)
// This is where the memory for them is actually allocated.
// =========================================================================

// --- Hardware Objects & Buffers ---
LGFX my_lcd;
TCA9535 TCA(HW::TCA_I2C_ADDR);
CRGB leds[HW::NUM_LEDS];
lv_color_t buf[HW::screenWidth * 20];

// --- LVGL UI Objects (initialized to nullptr for safety) ---
lv_obj_t *ui_arc          = nullptr;
lv_obj_t *ui_value_label  = nullptr;
lv_obj_t *ui_mode_label   = nullptr;
lv_obj_t *ui_power_switch = nullptr;
lv_group_t *encoder_group = nullptr;

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

void on_image_update(const byte* payload, unsigned int length) {
    Serial.printf("Image received in main.cpp! Size: %u bytes\n", length);

    // TODO: This is where you would decode the image (e.g., JPEG, PNG)
    // and update your LVGL image object.
    // For example, if it's a raw RGB565 image that fits a specific lv_img object:
    /*
    lv_obj_t* my_img_obj = ui_some_image; // Get your LVGL image object
    lv_img_dsc_t* img_dsc = (lv_img_dsc_t*)lv_img_get_src(my_img_obj);

    // This is a simplified example. In reality you would need to know the image
    // format and dimensions. A common approach is to use lv_img_decoder to
    // handle formats like JPG or PNG.
    
    // For now, we'll just print a message.
    lv_label_set_text(ui_value_label, "New Image!"); // Example of updating UI
    */
}

// WiFi setup
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(Config::ssid);
    WiFi.begin(Config::ssid, Config::pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
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

    hardware_init();  // Initialize physical devices
    lvgl_init();      // Initialize LVGL and its drivers
    ui_init();        // Create the user interface
    setup_wifi();     // Connect to WiFi
    mqtt_setup();     // Initialize MQTT client

    register_image_update_callback(on_image_update);

    Serial.println("Setup complete.");
}

void loop() {
    mqtt_loop();              // Handle incoming/outgoing MQTT messages
    handle_hardware_inputs(); // Read physical encoder and buttons
    sync_ui_with_state();     // Update the on-screen widgets to match our state
    update_leds();            // Update the physical LEDs to match our state
    FastLED.show();           // Send data to the LED strip
    
    lv_timer_handler();       // Let the GUI do its work
    delay(5);
}