#include "screen_manager.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "screen_manager";

typedef struct {
  const screen_descriptor_t *descriptor;
  lv_obj_t *screen_obj; // Pre-created screen object
} screen_entry_t;

static lv_display_t *lvgl_display = NULL;
static lv_indev_t *touch_indev = NULL;
static screen_entry_t screens[SCREEN_MAX] = {0};
static screen_id_t current_screen_id = SCREEN_MAX; // No screen initially

// Forward declaration
static void gesture_event_handler(lv_event_t *e);

esp_err_t screen_manager_init(lv_display_t *display, lv_indev_t *indev) {
  if (display == NULL) {
    ESP_LOGE(TAG, "Display is NULL");
    return ESP_ERR_INVALID_ARG;
  }

  lvgl_display = display;
  touch_indev = indev; // Can be NULL, we'll handle it
  memset(screens, 0, sizeof(screens));

  ESP_LOGI(TAG, "Screen manager initialized (display=%p, indev=%p)", display, indev);
  return ESP_OK;
}

esp_err_t screen_manager_register(const screen_descriptor_t *descriptor) {
  if (descriptor == NULL) {
    ESP_LOGE(TAG, "Screen descriptor is NULL");
    return ESP_ERR_INVALID_ARG;
  }

  if (descriptor->id >= SCREEN_MAX) {
    ESP_LOGE(TAG, "Invalid screen ID: %d", descriptor->id);
    return ESP_ERR_INVALID_ARG;
  }

  if (descriptor->create == NULL) {
    ESP_LOGE(TAG, "Screen %s has no create function", descriptor->name);
    return ESP_ERR_INVALID_ARG;
  }

  // Create screen immediately
  lv_obj_t *screen_obj = descriptor->create();
  if (screen_obj == NULL) {
    ESP_LOGE(TAG, "Failed to create screen %s", descriptor->name);
    return ESP_FAIL;
  }

  // Store descriptor and screen object
  screens[descriptor->id].descriptor = descriptor;
  screens[descriptor->id].screen_obj = screen_obj;

  // Add gesture event handler if screen has on_gesture callback
  if (descriptor->on_gesture != NULL) {
    lv_obj_add_event_cb(screen_obj, gesture_event_handler, LV_EVENT_GESTURE, NULL);
    ESP_LOGI(TAG, "Gesture handler registered for screen: %s", descriptor->name);
  }

  ESP_LOGI(TAG, "Screen registered and created: %s (id=%d)", descriptor->name,
           descriptor->id);
  return ESP_OK;
}

esp_err_t screen_manager_show(screen_id_t screen_id, lv_scr_load_anim_t anim) {
  if (screen_id >= SCREEN_MAX) {
    ESP_LOGE(TAG, "Invalid screen ID: %d", screen_id);
    return ESP_ERR_INVALID_ARG;
  }

  if (screens[screen_id].screen_obj == NULL) {
    ESP_LOGE(TAG, "Screen %d not registered", screen_id);
    return ESP_ERR_INVALID_STATE;
  }

  // Don't do anything if already on this screen
  if (current_screen_id == screen_id) {
    ESP_LOGD(TAG, "Already on screen %d", screen_id);
    return ESP_OK;
  }

  // Call on_hide for current screen
  if (current_screen_id < SCREEN_MAX &&
      screens[current_screen_id].descriptor != NULL) {
    if (screens[current_screen_id].descriptor->on_hide != NULL) {
      screens[current_screen_id].descriptor->on_hide();
    }
  }

  // Load new screen (already created, just switch to it)
  lv_screen_load_anim(screens[screen_id].screen_obj, anim, 300, 0, false);

  // Update current screen
  current_screen_id = screen_id;

  // Call on_show for new screen
  if (screens[screen_id].descriptor->on_show != NULL) {
    screens[screen_id].descriptor->on_show();
  }

  ESP_LOGI(TAG, "Screen shown: %s", screens[screen_id].descriptor->name);
  return ESP_OK;
}

screen_id_t screen_manager_get_current(void) { return current_screen_id; }

static void gesture_event_handler(lv_event_t *e) {
  if (touch_indev == NULL) {
    ESP_LOGE(TAG, "touch_indev is NULL!");
    return;
  }

  lv_dir_t dir = lv_indev_get_gesture_dir(touch_indev);
  ESP_LOGI(TAG, "Gesture: dir=%d", dir);
  
  screen_manager_handle_gesture(dir);
}

void screen_manager_handle_gesture(lv_dir_t direction) {
  ESP_LOGI(TAG, "screen_manager_handle_gesture: dir=%d, current_screen=%d", 
           direction, current_screen_id);
  
  if (current_screen_id >= SCREEN_MAX) {
    ESP_LOGW(TAG, "Invalid current screen ID: %d", current_screen_id);
    return;
  }

  if (screens[current_screen_id].descriptor == NULL) {
    ESP_LOGW(TAG, "Current screen not registered");
    return;
  }

  const char *screen_name = screens[current_screen_id].descriptor->name;
  ESP_LOGI(TAG, "Current screen: %s", screen_name);

  // Let screen handle gesture
  if (screens[current_screen_id].descriptor->on_gesture != NULL) {
    ESP_LOGI(TAG, "Calling on_gesture for screen: %s", screen_name);
    screens[current_screen_id].descriptor->on_gesture(direction);
  } else {
    ESP_LOGW(TAG, "Screen %s has no on_gesture handler", screen_name);
  }
}
