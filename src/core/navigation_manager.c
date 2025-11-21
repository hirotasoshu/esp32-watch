#include "navigation_manager.h"
#include "app_manager.h"
#include "esp_log.h"
#include "screen_manager.h"

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
  screen_id_t current_screen = screen_manager_get_current();

  // Handle gestures based on current context
  switch (current_context) {
  case NAV_CONTEXT_WATCHFACE:
    // On watchface - navigate to system screens
    if (direction == LV_DIR_RIGHT) {
      // Swipe right -> App Launcher (reveal from left)
      screen_manager_show(SCREEN_APP_LAUNCHER, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    } else if (direction == LV_DIR_LEFT) {
      // Swipe left -> Quick Access (reveal from right)
      screen_manager_show(SCREEN_QUICK_ACCESS, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    } else if (direction == LV_DIR_BOTTOM) {
      // Swipe down -> Notifications (reveal from top)
      screen_manager_show(SCREEN_NOTIFICATIONS, LV_SCR_LOAD_ANIM_MOVE_BOTTOM);
    } else if (direction == LV_DIR_TOP) {
      // Swipe up -> Control Center (reveal from bottom)
      screen_manager_show(SCREEN_CONTROL_CENTER, LV_SCR_LOAD_ANIM_MOVE_TOP);
    }
    break;

  case NAV_CONTEXT_SYSTEM_SCREEN:
    // On system screen - return to watchface
    if (direction == LV_DIR_LEFT && current_screen == SCREEN_APP_LAUNCHER) {
      // App Launcher -> Watchface (swipe back left)
      screen_manager_show(SCREEN_WATCHFACE, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    } else if (direction == LV_DIR_RIGHT &&
               current_screen == SCREEN_QUICK_ACCESS) {
      // Quick Access -> Watchface (swipe back right)
      screen_manager_show(SCREEN_WATCHFACE, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    } else if (direction == LV_DIR_TOP &&
               current_screen == SCREEN_NOTIFICATIONS) {
      // Notifications -> Watchface (swipe back up)
      screen_manager_show(SCREEN_WATCHFACE, LV_SCR_LOAD_ANIM_MOVE_TOP);
    } else if (direction == LV_DIR_BOTTOM &&
               current_screen == SCREEN_CONTROL_CENTER) {
      // Control Center -> Watchface (swipe back down)
      screen_manager_show(SCREEN_WATCHFACE, LV_SCR_LOAD_ANIM_MOVE_BOTTOM);
    }
    break;

  case NAV_CONTEXT_APP:
    // Inside app - swipe RIGHT to close (back gesture)
    if (direction == LV_DIR_RIGHT) {
      ESP_LOGI(TAG, "Closing app and returning to app launcher");
      app_manager_close_current();
      screen_manager_show(SCREEN_APP_LAUNCHER, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
      // Context will be set in app_launcher_screen_on_show()
    }
    break;

  default:
    ESP_LOGW(TAG, "Unknown navigation context: %d", current_context);
    break;
  }
}
