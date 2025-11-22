#include "ui/apps/launcher_app.h"
#include "core/app_manager.h"
#include "core/navigation_manager.h"
#include "esp_log.h"
#include "lvgl.h"
#include "ui/theme.h"

static const char *TAG = "launcher_app";

/**
 * @brief App icon click callback
 */
static void app_icon_clicked(lv_event_t *e) {
  app_id_t app_id = (app_id_t)(intptr_t)lv_event_get_user_data(e);
  ESP_LOGI(TAG, "App clicked: %d", app_id);

  // Show app
  app_manager_show(app_id, LV_SCR_LOAD_ANIM_MOVE_LEFT);
}

/**
 * @brief Create app launcher screen
 */
static lv_obj_t *launcher_app_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen, lv_color_hex(THEME_COLOR_BG), 0);
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

  // Title
  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(title, "Apps");
  lv_obj_set_style_text_font(title, THEME_FONT_MEDIUM, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(THEME_COLOR_BLACK), 0);
  lv_obj_set_pos(title, 20, 20);

  // App grid container
  lv_obj_t *grid = lv_obj_create(screen);
  lv_obj_set_size(grid, 220, 180);
  lv_obj_set_pos(grid, 10, 60);
  lv_obj_set_style_bg_color(grid, lv_color_hex(THEME_COLOR_BG), 0);
  lv_obj_set_style_border_width(grid, 0, 0);
  lv_obj_set_style_pad_all(grid, 10, 0);
  lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(grid, 15, 0);
  lv_obj_set_style_pad_column(grid, 15, 0);
  lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

  // Create app icon for System Info
  lv_obj_t *btn = lv_btn_create(grid);
  lv_obj_set_size(btn, 80, 80);
  lv_obj_set_style_bg_color(btn, lv_color_hex(THEME_COLOR_ORANGE), 0);
  lv_obj_set_style_radius(btn, 10, 0);
  lv_obj_add_event_cb(btn, app_icon_clicked, LV_EVENT_CLICKED,
                      (void *)(intptr_t)APP_USER_SYSTEM_INFO);

  // App label
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, "System\nInfo");
  lv_obj_set_style_text_font(label, THEME_FONT_SMALL, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(THEME_COLOR_WHITE), 0);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(label);

  ESP_LOGI(TAG, "App launcher screen created");
  return screen;
}

static void launcher_app_on_show(void) {
  ESP_LOGI(TAG, "App launcher screen shown");
  navigation_manager_set_context(NAV_CONTEXT_SYSTEM_SCREEN);
}

static void launcher_app_on_hide(void) {
  ESP_LOGI(TAG, "App launcher screen hidden");
}

static void launcher_app_on_gesture(lv_dir_t direction) {
  navigation_manager_handle_gesture(direction);
}

static const app_descriptor_t app_launcher_descriptor = {
    .id = APP_SYSTEM_LAUNCHER,
    .type = APP_TYPE_SYSTEM,
    .name = "App Launcher",
    .icon = NULL,
    .create = launcher_app_create,
    .destroy = NULL,
    .on_show = launcher_app_on_show,
    .on_hide = launcher_app_on_hide,
    .on_gesture = launcher_app_on_gesture,
};

const app_descriptor_t *launcher_app_get_descriptor(void) {
  return &app_launcher_descriptor;
}
