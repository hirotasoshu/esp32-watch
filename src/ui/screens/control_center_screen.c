#include "ui/screens/control_center_screen.h"
#include "core/navigation_manager.h"
#include "esp_log.h"
#include "lvgl.h"
#include "ui/theme.h"

static const char *TAG = "control_center_screen";

static lv_obj_t *control_center_screen_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen, lv_color_hex(THEME_COLOR_BLACK), 0);

  lv_obj_t *label = lv_label_create(screen);
  lv_label_set_text(label, "Control Center\n(Coming Soon)");
  lv_obj_set_style_text_color(label, lv_color_hex(THEME_COLOR_WHITE), 0);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(label);

  ESP_LOGI(TAG, "Control center screen created");
  return screen;
}

static void control_center_screen_on_show(void) {
  ESP_LOGI(TAG, "Control center screen shown");
  navigation_manager_set_context(NAV_CONTEXT_SYSTEM_SCREEN);
}

static void control_center_screen_on_hide(void) {
  ESP_LOGI(TAG, "Control center screen hidden");
}

static void control_center_screen_on_gesture(lv_dir_t direction) {
  navigation_manager_handle_gesture(direction);
}

static const screen_descriptor_t control_center_descriptor = {
    .id = SCREEN_CONTROL_CENTER,
    .name = "Control Center",
    .create = control_center_screen_create,
    .on_show = control_center_screen_on_show,
    .on_hide = control_center_screen_on_hide,
    .on_gesture = control_center_screen_on_gesture,
};

const screen_descriptor_t *control_center_screen_get_descriptor(void) {
  return &control_center_descriptor;
}
