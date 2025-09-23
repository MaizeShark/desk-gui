#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// This configuration is for the MSP4031 display with an ESP32-S3.
// Display: 4.0" 320x480 ST7796S (SPI)
// Touch:   Capacitive FT6336U (I2C)

class LGFX : public lgfx::LGFX_Device
{
  // Provide instances for panel, bus, light, and touch.
  lgfx::Panel_ST7796  _panel_instance;
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance;
  lgfx::Touch_FT5x06  _touch_instance; // FT6336 is compatible with FT5x06 driver

public:
  LGFX(void)
  {
    // Configure the bus control settings (SPI).
    {
      auto cfg = _bus_instance.config();
      
      // ESP32-S3 can use SPI2_HOST or SPI3_HOST.
      // Let's use SPI3_HOST as it's often free.
      cfg.spi_host = SPI3_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 80000000; // 80MHz from your User_Setup.h
      cfg.freq_read  = 20000000; // 20MHz from your User_Setup.h
      cfg.pin_sclk = 13;         // from your User_Setup.h TFT_SCLK
      cfg.pin_mosi = 12;         // from your User_Setup.h TFT_MOSI
      cfg.pin_miso = 11;         // from your User_Setup.h TFT_MISO
      cfg.pin_dc   = 17;         // from your User_Setup.h TFT_DC
      
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    // Configure the display panel control settings.
    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs           = 9;  // from your User_Setup.h TFT_CS
      cfg.pin_rst          = 10; // from your User_Setup.h TFT_RST
      cfg.pin_busy         = -1; // Not used

      // Set the resolution from the MSP4031 datasheet.
      cfg.panel_width      = 320;
      cfg.panel_height     = 480;
      cfg.memory_width     = 320;
      cfg.memory_height    = 480;
      cfg.offset_x         = 0;
      cfg.offset_y         = 0;
      cfg.offset_rotation  = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable         = true;
      cfg.invert           = false;
      cfg.rgb_order        = false; // false=RGB, true=BGR
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = true;

      _panel_instance.config(cfg);
    }

    // Configure the backlight control settings.
    {
      auto cfg = _light_instance.config();

      cfg.pin_bl = 14;           // from your User_Setup.h TFT_BL
      cfg.invert = false;        // TFT_BACKLIGHT_ON HIGH means not inverted
      cfg.freq   = 44100;        // PWM frequency
      cfg.pwm_channel = 7;       // PWM channel number (0-7)

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    // Configure the touch control settings (I2C).
    {
      auto cfg = _touch_instance.config();

      cfg.i2c_port    = 0;          // I2C port 0 or 1
      cfg.i2c_addr    = 0x38;       // FT6336U default address
      cfg.pin_sda     = 41;         // from your touch.h
      cfg.pin_scl     = 40;         // from your touch.h
      cfg.pin_int     = 16;         // from your touch.h
      cfg.pin_rst     = 15;         // from your touch.h
      cfg.freq        = 400000;     // I2C frequency
      cfg.x_min       = 0;
      cfg.x_max       = 319;        // Match panel width - 1
      cfg.y_min       = 0;
      cfg.y_max       = 479;        // Match panel height - 1
      cfg.bus_shared  = false;      // I2C bus is separate from SPI

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }
    
    setPanel(&_panel_instance);
  }
  void setBacklightFreq(uint32_t freq)
  {
    auto cfg = _light_instance.config();
    cfg.freq = freq;
    _light_instance.config(cfg);
    // Re-initialize the light with the current brightness
    _light_instance.init(getBrightness());
  }
};