#include "ui.h"

// UI Event Callbacks (local to this file)
static void arc_event_cb(lv_event_t * e) {
    lv_obj_t * arc = (lv_obj_t *)lv_event_get_target(e);
    encoderValue = lv_arc_get_value(arc);
    lv_label_set_text_fmt(ui_value_label, "%d", (int)encoderValue);
}

static void switch_event_cb(lv_event_t * e) {
    lv_obj_t * sw = (lv_obj_t *)lv_event_get_target(e);
    ledsOn = lv_obj_has_state(sw, LV_STATE_CHECKED);
}

// Main UI creation function
void ui_init() {
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x111111), LV_PART_MAIN);

    ui_album_art = lv_image_create(lv_screen_active());
    lv_obj_set_size(ui_album_art, 480, 320);
    lv_obj_align(ui_album_art, LV_ALIGN_CENTER, 0, 0);

    // Create a semi-transparent panel at the bottom for track info
    lv_obj_t* info_panel = lv_obj_create(lv_screen_active());
    lv_obj_set_size(info_panel, 480, 80);
    lv_obj_align(info_panel, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(info_panel, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(info_panel, LV_OPA_50, 0); // 50% transparent
    lv_obj_set_style_border_width(info_panel, 0, 0);

    // Create the track label inside the panel
    ui_track_label = lv_label_create(info_panel);
    lv_label_set_text(ui_track_label, "Track Name"); // Set placeholder text
    lv_obj_set_style_text_font(ui_track_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(ui_track_label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_long_mode(ui_track_label, LV_LABEL_LONG_SCROLL_CIRCULAR); // Cool scrolling effect for long titles
    lv_obj_set_width(ui_track_label, 440); // Set width to enable scrolling
    lv_obj_align(ui_track_label, LV_ALIGN_TOP_LEFT, 10, 10);

    // Create the artist label inside the panel
    ui_artist_label = lv_label_create(info_panel);
    lv_label_set_text(ui_artist_label, "Artist Name"); // Set placeholder text
    lv_obj_set_style_text_font(ui_artist_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(ui_artist_label, lv_color_hex(0xCCCCCC), 0);
    lv_obj_align(ui_artist_label, LV_ALIGN_BOTTOM_LEFT, 10, -10);

    ui_arc = lv_arc_create(lv_screen_active());
    lv_obj_set_size(ui_arc, 220, 220);
    lv_obj_align(ui_arc, LV_ALIGN_CENTER, 0, 10);
    lv_arc_set_rotation(ui_arc, 135);
    lv_arc_set_bg_angles(ui_arc, 0, 270);
    lv_arc_set_value(ui_arc, encoderValue);
    lv_obj_add_event_cb(ui_arc, arc_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_group_add_obj(encoder_group, ui_arc);

    ui_value_label = lv_label_create(lv_screen_active());
    lv_label_set_text_fmt(ui_value_label, "%d", (int)encoderValue);
    lv_obj_set_style_text_font(ui_value_label, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(ui_value_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align_to(ui_value_label, ui_arc, LV_ALIGN_CENTER, 0, 0);

    ui_mode_label = lv_label_create(lv_screen_active());
    lv_label_set_text(ui_mode_label, modeNames[currentMode]);
    lv_obj_set_style_text_font(ui_mode_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(ui_mode_label, lv_color_hex(0xCCCCCC), 0);
    lv_obj_align(ui_mode_label, LV_ALIGN_TOP_MID, 0, 15);

    ui_power_switch = lv_switch_create(lv_screen_active());
    if (ledsOn) lv_obj_add_state(ui_power_switch, LV_STATE_CHECKED);
    lv_obj_align(ui_power_switch, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(ui_power_switch, switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_t* power_label = lv_label_create(lv_screen_active());
    lv_label_set_text(power_label, "LEDs On");
    lv_obj_align_to(power_label, ui_power_switch, LV_ALIGN_OUT_LEFT_MID, -10, 0);
}

// Function to sync the UI with the current state
void sync_ui_with_state() {
    if (lv_arc_get_value(ui_arc) != encoderValue) {
        lv_arc_set_value(ui_arc, encoderValue);
        lv_label_set_text_fmt(ui_value_label, "%d", (int)encoderValue);
    }
    if (ledsOn && !lv_obj_has_state(ui_power_switch, LV_STATE_CHECKED)) {
        lv_obj_add_state(ui_power_switch, LV_STATE_CHECKED);
    } else if (!ledsOn && lv_obj_has_state(ui_power_switch, LV_STATE_CHECKED)) {
        lv_obj_clear_state(ui_power_switch, LV_STATE_CHECKED);
    }
    lv_label_set_text(ui_mode_label, modeNames[currentMode]);
    if (currentMode == 0) lv_arc_set_range(ui_arc, 0, 100);
    if (currentMode == 1) lv_arc_set_range(ui_arc, 0, 255);
    if (currentMode == 2) lv_arc_set_range(ui_arc, 0, HW::NUM_LEDS - 1);
}