#include "lvgl_handler.h"

// LVGL Driver Callbacks
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    my_lcd.pushImage(area->x1, area->y1, w, h, (lgfx::rgb565_t*)px_map);
    lv_display_flush_ready(disp);
}
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
    uint16_t touchX, touchY;
    bool touched = my_lcd.getTouch(&touchX, &touchY);
    data->state = touched ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    if (touched) { data->point.x = touchX; data->point.y = touchY; }
}
void my_encoder_read(lv_indev_t *indev, lv_indev_data_t *data) {
    data->enc_diff = encoderValue - last_lvgl_encoder_val;
    last_lvgl_encoder_val = encoderValue;
    data->state = (TCA.read1(ENCODER_SW_PIN) == LOW) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}
uint32_t my_tick_get_cb(void) { return millis(); }

void lvgl_init() {
    lv_init();
    lv_tick_set_cb(my_tick_get_cb);

    // Display driver
    lv_display_t *disp = lv_display_create(screenWidth, screenHeight);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Touch driver
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read);

    // Encoder driver
    lv_indev_t *enc_indev = lv_indev_create();
    lv_indev_set_type(enc_indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(enc_indev, my_encoder_read);
    
    // Create a group for the encoder to control widgets
    encoder_group = lv_group_create();
    lv_group_set_default(encoder_group);
    lv_indev_set_group(enc_indev, encoder_group);
}