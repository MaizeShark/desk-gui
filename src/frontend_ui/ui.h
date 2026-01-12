#ifndef UI_H
#define UI_H

#include "globals.h"
#include "config.h"

void ui_init();
void sync_ui_with_state();

static void event_go_to_screen2(lv_event_t * e);
static void event_go_to_screen1(lv_event_t * e);

#endif // UI_H