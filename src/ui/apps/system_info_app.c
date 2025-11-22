#include "ui/apps/system_info_app.h"
#include "core/navigation_manager.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_system.h"
#include "lvgl.h"
#include "ui/theme.h"
#include <stdio.h>

static const char *TAG = "system_info_app";

static lv_obj_t *app_screen = NULL;

static esp_err_t system_info_init(void) {
  ESP_LOGI(TAG, "System Info app initialized");
  return ESP_OK;
}

static void system_info_deinit(void) {
  ESP_LOGI(TAG, "System Info app deinitialized");
}

/**
 * @brief Gesture handler for app screen
 */
static void system_info_on_gesture(lv_dir_t direction) {
  ESP_LOGI(TAG, "System info gesture: dir=%d", direction);
  navigation_manager_handle_gesture(direction);
}

static lv_obj_t *system_info_create_ui(void) {
  app_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(app_screen, lv_color_hex(THEME_COLOR_BG), 0);
  lv_obj_clear_flag(app_screen, LV_OBJ_FLAG_SCROLLABLE);

  // Title
  lv_obj_t *title = lv_label_create(app_screen);
  lv_label_set_text(title, "System Info");
  lv_obj_set_style_text_font(title, THEME_FONT_MEDIUM, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(THEME_COLOR_BLACK), 0);
  lv_obj_set_pos(title, 20, 10);

  // Info container
  lv_obj_t *info_container = lv_obj_create(app_screen);
  lv_obj_set_size(info_container, 220, 230);
  lv_obj_set_pos(info_container, 10, 50);
  lv_obj_set_style_bg_color(info_container, lv_color_hex(THEME_COLOR_WHITE), 0);
  lv_obj_set_style_border_width(info_container, 0, 0);
  lv_obj_set_style_radius(info_container, 10, 0);
  lv_obj_set_style_pad_all(info_container, 15, 0);
  lv_obj_set_scroll_dir(info_container, LV_DIR_VER);

  // Info text
  lv_obj_t *info_label = lv_label_create(info_container);
  lv_obj_set_width(info_label, 190);
  lv_label_set_long_mode(info_label, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_font(info_label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(info_label, lv_color_hex(THEME_COLOR_BLACK), 0);

  // Build system info string
  char info_buf[512];
  snprintf(info_buf, sizeof(info_buf),
           "Project: esp32-watch\n\n"
           "Build Date: %s\n"
           "Build Time: %s\n\n"
           "ESP-IDF: %s\n\n"
           "Chip: %s\n"
           "Cores: %d\n\n"
           "Free Heap: %lu KB\n"
           "Min Free Heap: %lu KB",
           __DATE__, __TIME__, esp_get_idf_version(), CONFIG_IDF_TARGET,
           portNUM_PROCESSORS, esp_get_free_heap_size() / 1024,
           esp_get_minimum_free_heap_size() / 1024);

  lv_label_set_text(info_label, info_buf);

  ESP_LOGI(TAG, "System Info UI created");
  return app_screen;
}

static void system_info_destroy_ui(lv_obj_t *screen) {
  if (screen != NULL) {
    lv_obj_del(screen);
    app_screen = NULL;
  }
}

static void system_info_on_launch(void) {
  ESP_LOGI(TAG, "System Info app launched");
  navigation_manager_set_context(NAV_CONTEXT_APP);
}

static void system_info_on_close(void) {
  ESP_LOGI(TAG, "System Info app closed");
}

static const app_descriptor_t system_info_descriptor = {
    .id = APP_USER_SYSTEM_INFO,
    .type = APP_TYPE_USER,
    .name = "System Info",
    .icon = NULL,
    .create = system_info_create_ui,
    .destroy = system_info_destroy_ui,
    .on_show = system_info_on_launch,
    .on_hide = system_info_on_close,
    .on_gesture = system_info_on_gesture,
};

const app_descriptor_t *system_info_app_get_descriptor(void) {
  return &system_info_descriptor;
}
