# Desk GUI for ESP32-S3 with Spotify Integration

## Project Overview

This project implements a sophisticated graphical user interface (GUI) for a desk setup, leveraging an ESP32-S3 microcontroller. It features seamless Spotify playback control, dynamic ambient lighting, and intuitive user interaction through an encoder and physical buttons. Designed for a rich visual experience, it integrates a high-resolution display and various peripherals to create a functional and aesthetically pleasing control hub for your workspace.

## Features

*   **Spotify Playback Control:** Display current track information (title, artist, album art) and control playback (play/pause, next/previous track).
*   **Ambient Lighting:** Control 12 WS2812B individually addressable LEDs for dynamic lighting effects, synchronized with music or user preferences.
*   **Intuitive User Interface:** A responsive GUI built with LVGL and LovyanGFX on a 4.0" 320x480 display.
*   **Physical Controls:** User input via a rotary encoder and two buttons, managed by a TCA9535 I/O expander.
*   **Modular Architecture:** Separated concerns for GUI/HID and main control logic, allowing for flexible development and expansion.

## Hardware Requirements

*   **ESP32-S3 N16R8:** Microcontroller with 16MB Flash and 8MB PSRAM.
*   **MSP4031 Display:** 4.0" 320x480 TFT display (ST7796S controller, SPI interface).
*   **FT6336U Touch Controller:** Capacitive touch panel (I2C interface).
*   **WS2812B LEDs:** 12 individually addressable RGB LEDs.
*   **TCA9535 I/O Expander:** For managing encoder and button inputs.
*   **Rotary Encoder:** For navigation and input.
*   **Buttons:** Two physical buttons.

## Software Requirements

*   **PlatformIO IDE:** For building and uploading the firmware.
*   **Arduino Framework for ESP32.**
*   **Libraries:**
    *   LovyanGFX (`lovyan03/LovyanGFX`)
    *   LVGL (`lvgl/lvgl`)
    *   TCA9555 (`robtillaart/TCA9555`)
    *   FastLED (`fastled/FastLED`)
    *   PubSubClient (`knolleary/PubSubClient`)
    *   ArduinoJson (`bblanchon/ArduinoJson`)
    *   SpotifyEsp32 (`finianlandes/SpotifyEsp32`)

## Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/MaizeShark/desk-gui.git
cd desk-gui
cp src/frontend_ui/globals.example.h src/frontend_ui/globals.h
```

### 2. Install PlatformIO

If you don't have PlatformIO installed, follow the instructions on the [PlatformIO website](https://platformio.org/).

### 3. Configure Wi-Fi and Spotify API Credentials

Before building, you need to provide your Wi-Fi network credentials and Spotify API keys.

*   **Spotify API:**
    *   Follow the instructions in the `get_spotify_token.py` script to obtain your Spotify Client ID, Client Secret, and Refresh Token.
    *   Update the `spotify_refresh_token` in `src/frontend_ui/globals.h` with your refresh token.
    *   Update `spotify_client_id` and `spotify_client_secret` in `src/frontend_ui/globals.h` with your Spotify application credentials.
*   **Wi-Fi Credentials:**
    *   Update `ssid` and `pass` in `src/frontend_ui/globals.h` with your Wi-Fi network details.

### 4. Build and Upload

Open the project in PlatformIO IDE and build/upload to your ESP32-S3 board. Ensure you select the `frontend_s3` environment for the GUI/HID ESP32.

```bash
pio run -e frontend_s3 -t upload
```

## Project Structure

*   `get_spotify_token.py`: Python script to assist in obtaining Spotify API tokens.
*   `src/frontend_ui/`: Contains all source code for the GUI, HID, and UI logic running on the ESP32.
    *   `globals.h`: Global configuration settings for the GUI ESP32 (Wi-Fi, Spotify credentials, pin definitions).
    *   `hardware.cpp`/`hardware.h`: Hardware initialization and control (display, touch, LEDs, encoder).
    *   `lvgl_handler.cpp`/`lvgl_handler.h`: LVGL initialization and task handling.
    *   `main.cpp`: Main application entry point for the GUI ESP32.
    *   `mqtt.cpp`/`mqtt.h`: MQTT communication for inter-ESP32 communication or external control.
    *   `music_player.cpp`/`music_player.h`: Logic for Spotify integration and music display.
    *   `ui.cpp`/`ui.h`: LVGL UI creation and management.
*   `src/main_controller/`: (Placeholder/Separate project) Intended for the main control/audio ESP32.

## Usage

Once uploaded, the ESP32 will connect to Wi-Fi and authenticate with Spotify. The display will show the current track information and album art.

*   **Rotary Encoder:**
    *   Rotate: Navigate through menus or adjust settings (e.g., volume, brightness).
    *   Press: Select an item or confirm an action.
*   **Buttons:**
    *   Button 1: Play/Pause Spotify.
    *   Button 2: Skip to next track. (Example functionality, can be customized).

## Contributing

Feel free to fork the repository, make improvements, and submit pull requests.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
