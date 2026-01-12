#include "ui.h"

LV_FONT_DECLARE(delius20_numbers);

void ui_Screen1_screen_init(void);
void ui_Screen2_screen_init(void);

void publish_elapsed_time(uint16_t elapsed);

static void arc_event_cb(lv_event_t * e) {
    lv_obj_t * arc = (lv_obj_t *)lv_event_get_target(e);
    encoderValue = lv_arc_get_value(arc);
    lv_label_set_text_fmt(ui_value_label, "%d", (int)encoderValue);
}

static void switch_event_cb(lv_event_t * e) {
    lv_obj_t * sw = (lv_obj_t *)lv_event_get_target(e);
    ledsOn = lv_obj_has_state(sw, LV_STATE_CHECKED);
}

static void event_go_to_screen2(lv_event_t * e) {
    lv_screen_load(ui_Screen2);
}

static void event_go_to_screen1(lv_event_t * e) {
    lv_screen_load(ui_Screen1);
}

static void progress_bar_event_cb(lv_event_t * e) {
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    // Reagiere nur, wenn der Wert sich geändert hat
    if (code == LV_EVENT_VALUE_CHANGED) {
        int32_t value = lv_slider_get_value(slider);
        
        // Rufe hier deine MQTT-Funktion auf, um die neue Position zu senden
        publish_elapsed_time(value); 
    }
}


// --- SCREEN 1 INITIALIZATION (Display Screen) ---
// This screen shows information but has no main controls.
void ui_Screen1_screen_init(void) {
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(0x111111), LV_PART_MAIN);

    ui_album_art = lv_image_create(ui_Screen1);
    lv_obj_set_size(ui_album_art, 480, 320);
    lv_obj_align(ui_album_art, LV_ALIGN_CENTER, 0, 0);

    ui_length_label = lv_label_create(ui_Screen1);
    lv_obj_set_style_text_font(ui_length_label, &delius20_numbers, 0);
    lv_obj_set_style_text_color(ui_length_label, lv_color_hex(0xD2D2D2), 0);
    lv_obj_set_width(ui_length_label, 85);
    lv_obj_align(ui_length_label, LV_ALIGN_BOTTOM_RIGHT, -2, -25);
    lv_obj_set_style_text_align(ui_length_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui_length_label, "0:00");

    ui_position_label = lv_label_create(ui_Screen1);
    lv_obj_set_style_text_font(ui_position_label, &delius20_numbers, 0);
    lv_obj_set_style_text_color(ui_position_label, lv_color_hex(0xD2D2D2), 0);
    lv_obj_set_width(ui_position_label, 85);
    lv_obj_align(ui_position_label, LV_ALIGN_BOTTOM_LEFT, 2, -25);
    lv_obj_set_style_text_align(ui_position_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui_position_label, "0:00");

    ui_progress_bar = lv_slider_create(ui_Screen1);
    lv_obj_align(ui_progress_bar, LV_ALIGN_BOTTOM_MID, 0, -22);
    lv_obj_set_size(ui_progress_bar, 300, 20);
    lv_slider_set_range(ui_progress_bar, 0, 204); // Temporary max; will be updated via MQTT
    lv_slider_set_value(ui_progress_bar, 107, LV_ANIM_ON); // Temporary value; will be updated via MQTT
    // bg part
    lv_obj_set_style_bg_color(ui_progress_bar, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_progress_bar, 77, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_progress_bar, lv_color_hex(0x313131), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_progress_bar, 220, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_progress_bar, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    // indicator part
    lv_obj_set_style_bg_color(ui_progress_bar, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_progress_bar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    // Knob part
    lv_obj_set_style_bg_color(ui_progress_bar, lv_color_hex(0xE5E5E5), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_progress_bar, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_progress_bar, progress_bar_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Button to navigate to the Controls screen
    lv_obj_t * screen2_btn = lv_button_create(ui_Screen1);
    lv_obj_align(screen2_btn, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_add_event_cb(screen2_btn, event_go_to_screen2, LV_EVENT_CLICKED, NULL);
    lv_obj_t * btn_label = lv_label_create(screen2_btn);
    lv_label_set_text(btn_label, "Controls");
    lv_obj_center(btn_label);
}

// --- SCREEN 2 INITIALIZATION (Controls Screen) ---
// This screen has all the interactive controls.
void ui_Screen2_screen_init(void) {
    ui_Screen2 = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ui_Screen2, lv_color_hex(0x331133), LV_PART_MAIN);

    ui_arc = lv_arc_create(ui_Screen2);
    lv_obj_set_size(ui_arc, 220, 220);
    lv_obj_align(ui_arc, LV_ALIGN_CENTER, 0, 10);
    lv_arc_set_rotation(ui_arc, 135);
    lv_arc_set_bg_angles(ui_arc, 0, 270);
    lv_arc_set_value(ui_arc, encoderValue);
    lv_obj_add_event_cb(ui_arc, arc_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_group_add_obj(encoder_group, ui_arc);

    ui_value_label = lv_label_create(ui_Screen2);
    lv_label_set_text_fmt(ui_value_label, "%d", (int)encoderValue);
    lv_obj_set_style_text_font(ui_value_label, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(ui_value_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align_to(ui_value_label, ui_arc, LV_ALIGN_CENTER, 0, 0);

    ui_mode_label = lv_label_create(ui_Screen2);
    lv_label_set_text(ui_mode_label, modeNames[currentMode]);
    lv_obj_set_style_text_font(ui_mode_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(ui_mode_label, lv_color_hex(0xCCCCCC), 0);
    lv_obj_align(ui_mode_label, LV_ALIGN_TOP_MID, 0, 15);

    ui_power_switch = lv_switch_create(ui_Screen2);
    if (ledsOn) lv_obj_add_state(ui_power_switch, LV_STATE_CHECKED);
    lv_obj_align(ui_power_switch, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(ui_power_switch, switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_t* power_label = lv_label_create(ui_Screen2);
    lv_label_set_text(power_label, "LEDs On");
    lv_obj_align_to(power_label, ui_power_switch, LV_ALIGN_OUT_LEFT_MID, -10, 0);

    // Add a button to go back to Screen 1
    lv_obj_t * screen1_btn = lv_button_create(ui_Screen2);
    lv_obj_align(screen1_btn, LV_ALIGN_TOP_LEFT, 10, 10); // FIX: Moved button to top left to avoid overlap
    lv_obj_add_event_cb(screen1_btn, event_go_to_screen1, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_label = lv_label_create(screen1_btn);
    lv_label_set_text(btn_label, "Back");
    lv_obj_center(btn_label);
}


// --- MAIN UI INITIALIZATION ---
void ui_init() {
    ui_Screen1_screen_init();
    ui_Screen2_screen_init();
    lv_screen_load(ui_Screen1);
}

// --- UI STATE SYNC ---
void sync_ui_with_state() {
    // KEY CHANGE: Check which screen is active before updating widgets.

    if (lv_screen_active() == ui_Screen1) {
        // This is the "Display" screen.
        // You can add logic here to update the album art or time labels if needed.
        // For example: lv_label_set_text(ui_position_label, new_time_string);
    } 
    else if (lv_screen_active() == ui_Screen2) {
        // This is the "Controls" screen. Only update the controls when this screen is visible.
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
        if (currentMode == 3) lv_arc_set_range(ui_arc, 0, 100); // Volume 0-100%
    }
}