#include "navigation_manager.h"
#include "app_manager.h"
#include "esp_log.h"

static const char *TAG = "nav_manager";

static nav_context_t current_context = NAV_CONTEXT_WATCHFACE;

esp_err_t navigation_manager_init(void) {
  current_context = NAV_CONTEXT_WATCHFACE;
  ESP_LOGI(TAG, "Navigation manager initialized");
  return ESP_OK;
}

void navigation_manager_set_context(nav_context_t context) {
  if (context != current_context) {
    ESP_LOGI(TAG, "Context changed: %d -> %d", current_context, context);
    current_context = context;
  }
}

nav_context_t navigation_manager_get_context(void) { return current_context; }

void navigation_manager_handle_gesture(lv_dir_t direction) {
  app_id_t current_app = app_manager_get_current();

  switch (current_context) {
  case NAV_CONTEXT_WATCHFACE:
    if (direction == LV_DIR_RIGHT) {
      app_manager_show(APP_SYSTEM_LAUNCHER, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    } else if (direction == LV_DIR_LEFT) {
      app_manager_show(APP_SYSTEM_QUICK_ACCESS, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    } else if (direction == LV_DIR_BOTTOM) {
      app_manager_show(APP_SYSTEM_NOTIFICATIONS, LV_SCR_LOAD_ANIM_MOVE_BOTTOM);
    } else if (direction == LV_DIR_TOP) {
      app_manager_show(APP_SYSTEM_CONTROL_CENTER, LV_SCR_LOAD_ANIM_MOVE_TOP);
    }
    break;

  case NAV_CONTEXT_SYSTEM_SCREEN:
    if (direction == LV_DIR_LEFT && current_app == APP_SYSTEM_LAUNCHER) {
      app_manager_show(APP_WATCHFACE, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    } else if (direction == LV_DIR_RIGHT && current_app == APP_SYSTEM_QUICK_ACCESS) {
      app_manager_show(APP_WATCHFACE, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    } else if (direction == LV_DIR_TOP && current_app == APP_SYSTEM_NOTIFICATIONS) {
      app_manager_show(APP_WATCHFACE, LV_SCR_LOAD_ANIM_MOVE_TOP);
    } else if (direction == LV_DIR_BOTTOM && current_app == APP_SYSTEM_CONTROL_CENTER) {
      app_manager_show(APP_WATCHFACE, LV_SCR_LOAD_ANIM_MOVE_BOTTOM);
    }
    break;

  case NAV_CONTEXT_APP:
    if (direction == LV_DIR_RIGHT) {
      ESP_LOGI(TAG, "Closing app and returning to launcher");
      app_manager_show(APP_SYSTEM_LAUNCHER, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    }
    break;

  default:
    ESP_LOGW(TAG, "Unknown navigation context: %d", current_context);
    break;
  }
}
