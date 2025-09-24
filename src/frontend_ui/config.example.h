#ifndef CONFIG_H
#define CONFIG_H
// #define DEBUG_MQTT // Uncomment to enable verbose MQTT debug output

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
    constexpr const char* ssid = "your_ssid";
    constexpr const char* pass = "your_password";

    // --- MQTT Broker Settings ---
    constexpr const char* broker_host = "broker.emqx.io"; // Public broker for testing, please use your own
    constexpr uint16_t    broker_port = 1883;
    constexpr const char* broker_user = ""; // add your username if needed
    constexpr const char* broker_pass = ""; // add your password if needed

    // --- MQTT Topics ---
    constexpr const char* topic_status           = "esp-gui/status";
    constexpr const char* topic_command          = "esp-gui/command";
    constexpr const char* topic_brightness       = "esp-gui/brightness";
    constexpr const char* topic_image            = "music/image";

    // --- Spotify Credentials ---
    // You need to create an app in the Spotify Developer Dashboard
    // and run ../../get_spotify_token.py to get a refresh token. 
    constexpr const char* spotify_client_id     = "spotify_client_id_here";
    constexpr const char* spotify_client_secret = "spotify_client_secret_here";
    constexpr const char* spotify_refresh_token  = "spotify_refresh_token_here";
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
    #define LED_TYPE    WS2812B
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

#endif // CONFIG_H