#ifndef LVGL_HANDLER_H
#define LVGL_HANDLER_H

#include "globals.h"

void lvgl_init(); // A new function to contain all LVGL setup

// LVGL Driver Callbacks
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data);
void my_encoder_read(lv_indev_t *indev, lv_indev_data_t *data);
uint32_t my_tick_get_cb(void);

#endif // LVGL_HANDLER_H