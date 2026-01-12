#include "hardware.h"
#include "ui.h"

void hardware_init() {
    my_lcd.init();
    my_lcd.setRotation(1);
    my_lcd.setBrightness(255);
    my_lcd.setBacklightFreq(10000);

    Wire.begin(HW::I2C_SDA_PIN, HW::I2C_SCL_PIN);
    if (!TCA.begin()) {
        Serial.println("FATAL: TCA9535 not found.");
        while(1);
    }
    TCA.pinMode16(0xFFFF);

    FastLED.addLeds<LED_TYPE, HW::LED_DATA_PIN, COLOR_ORDER>(leds, HW::NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(100);

    // Prime the input readers
    uint16_t initialStates = TCA.read16();
    lastPinA_State = (initialStates >> HW::ENCODER_A_PIN) & 1;
    lastEncSwitchState = (initialStates >> HW::ENCODER_SW_PIN) & 1;
    lastButton1State = (initialStates >> HW::BUTTON_1_PIN) & 1;
    lastButton2State = (initialStates >> HW::BUTTON_2_PIN) & 1;
}

void handle_hardware_inputs() {
    uint16_t pinStates = TCA.read16();
    // --- Process Encoder Rotation ---
    uint8_t pinA = (pinStates >> HW::ENCODER_A_PIN) & 1;
    if (pinA == LOW && lastPinA_State == HIGH) {
        uint8_t pinB = (pinStates >> HW::ENCODER_B_PIN) & 1;
        if (pinB == HIGH) encoderValue++; else encoderValue--;
    }
    lastPinA_State = pinA;

    // --- Constrain value based on mode ---
    int minVal = 0, maxVal = 100;
    if (currentMode == 0) maxVal = 100;
    if (currentMode == 1) maxVal = 255;
    if (currentMode == 2) maxVal = HW::NUM_LEDS - 1;
    if (currentMode == 3) maxVal = 100; // Volume 0-100%
    encoderValue = constrain(encoderValue, minVal, maxVal);

    // --- Process Buttons ---
    uint8_t encSwitchState = (pinStates >> HW::ENCODER_SW_PIN) & 1;
    if (encSwitchState == LOW && lastEncSwitchState == HIGH) { ledsOn = !ledsOn; }
    lastEncSwitchState = encSwitchState;

    // --- Back Button ---
    uint8_t button1State = (pinStates >> HW::BUTTON_1_PIN) & 1;
    if (button1State == LOW && lastButton1State == HIGH) {
        lv_scr_load(ui_Screen1);
    }
    lastButton1State = button1State;

    // --- Switch Button ---
    uint8_t button2State = (pinStates >> HW::BUTTON_2_PIN) & 1;
    if (button2State == LOW && lastButton2State == HIGH) {
        currentMode = (currentMode - 1);
        if (currentMode < 0) currentMode = totalModes - 1;
        encoderValue = lv_arc_get_value(ui_arc);
    }
    lastButton2State = button2State;
}

void update_leds() {
    if (!ledsOn) {
        FastLED.clear();
        return;
    }
    switch (currentMode) {
        case 0:
            FastLED.setBrightness(map(encoderValue, 0, 100, 0, 255));
            fill_solid(leds, HW::NUM_LEDS, CRGB::Wheat);
            break;
        case 1:
            FastLED.setBrightness(200);
            fill_solid(leds, HW::NUM_LEDS, CHSV(encoderValue, 255, 255));
            break;
        case 2:
            FastLED.setBrightness(200);
            FastLED.clear();
            leds[encoderValue] = CRGB::Red;
            break;
        case 3:
            FastLED.setBrightness(0);
            fill_solid(leds, HW::NUM_LEDS, CRGB::Blue);
            break;
    }
}