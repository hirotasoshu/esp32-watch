#include "ui/screens/quick_access_screen.h"
#include "core/navigation_manager.h"
#include "esp_log.h"
#include "lvgl.h"
#include "ui/theme.h"

static const char *TAG = "quick_access_screen";

static lv_obj_t *quick_access_screen_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen, lv_color_hex(THEME_COLOR_BLACK), 0);

  lv_obj_t *label = lv_label_create(screen);
  lv_label_set_text(label, "Quick Access\n(Coming Soon)");
  lv_obj_set_style_text_color(label, lv_color_hex(THEME_COLOR_WHITE), 0);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(label);

  ESP_LOGI(TAG, "Quick access screen created");
  return screen;
}

static void quick_access_screen_on_show(void) {
  ESP_LOGI(TAG, "Quick access screen shown");
  navigation_manager_set_context(NAV_CONTEXT_SYSTEM_SCREEN);
}

static void quick_access_screen_on_hide(void) {
  ESP_LOGI(TAG, "Quick access screen hidden");
}

static void quick_access_screen_on_gesture(lv_dir_t direction) {
  navigation_manager_handle_gesture(direction);
}

static const screen_descriptor_t quick_access_descriptor = {
    .id = SCREEN_QUICK_ACCESS,
    .name = "Quick Access",
    .create = quick_access_screen_create,
    .on_show = quick_access_screen_on_show,
    .on_hide = quick_access_screen_on_hide,
    .on_gesture = quick_access_screen_on_gesture,
};

const screen_descriptor_t *quick_access_screen_get_descriptor(void) {
  return &quick_access_descriptor;
}
