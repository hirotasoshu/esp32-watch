#include "app_manager.h"
#include "display_manager.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "app_manager";

typedef struct {
  const app_descriptor_t *descriptor;
  lv_obj_t *screen_obj;
  bool is_created;
} app_entry_t;

static app_entry_t apps[APP_MAX] = {0};
static app_id_t current_app_id = APP_MAX;

static void gesture_event_handler(lv_event_t *e);

static app_id_t get_app_id_from_obj(lv_obj_t *obj) {
  if (obj == NULL) return APP_MAX;
  for (app_id_t id = 0; id < APP_MAX; id++) {
    if (apps[id].screen_obj == obj) return id;
  }
  return APP_MAX;
}

esp_err_t app_manager_init(void) {
  memset(apps, 0, sizeof(apps));
  current_app_id = APP_MAX;
  ESP_LOGI(TAG, "App manager initialized");
  return ESP_OK;
}

esp_err_t app_manager_register(const app_descriptor_t *app) {
  if (app == NULL) {
    ESP_LOGE(TAG, "App descriptor is NULL");
    return ESP_ERR_INVALID_ARG;
  }

  if (app->id >= APP_MAX) {
    ESP_LOGE(TAG, "Invalid app ID: %d", app->id);
    return ESP_ERR_INVALID_ARG;
  }

  if (apps[app->id].descriptor != NULL) {
    ESP_LOGE(TAG, "App ID %d already registered", app->id);
    return ESP_ERR_INVALID_STATE;
  }

  if (app->create == NULL) {
    ESP_LOGE(TAG, "App %s has no create function", app->name);
    return ESP_ERR_INVALID_ARG;
  }

  apps[app->id].descriptor = app;

  // For WATCHFACE and SYSTEM apps: create immediately
  if (app->type == APP_TYPE_WATCHFACE || app->type == APP_TYPE_SYSTEM) {
    lv_obj_t *screen_obj = app->create();
    if (screen_obj == NULL) {
      ESP_LOGE(TAG, "Failed to create app %s", app->name);
      return ESP_FAIL;
    }

    apps[app->id].screen_obj = screen_obj;
    apps[app->id].is_created = true;

    // Add gesture handler
    if (app->on_gesture != NULL) {
      lv_indev_t *touch = display_manager_get_touch();
      if (touch == NULL) {
        ESP_LOGW(TAG, "Touch disabled - no gesture for app %s", app->name);
      } else {
        lv_obj_add_event_cb(screen_obj, gesture_event_handler, LV_EVENT_GESTURE, NULL);
        ESP_LOGI(TAG, "Gesture registered for app: %s", app->name);
      }
    }

    ESP_LOGI(TAG, "App registered (persistent): %s (id=%d, type=%d)", 
             app->name, app->id, app->type);
  } else {
    // USER apps: don't create yet
    apps[app->id].is_created = false;
    ESP_LOGI(TAG, "App registered (lazy): %s (id=%d)", app->name, app->id);
  }

  return ESP_OK;
}

esp_err_t app_manager_show(app_id_t app_id, lv_scr_load_anim_t anim) {
  if (app_id >= APP_MAX) {
    ESP_LOGE(TAG, "Invalid app ID: %d", app_id);
    return ESP_ERR_INVALID_ARG;
  }

  if (apps[app_id].descriptor == NULL) {
    ESP_LOGE(TAG, "App %d not registered", app_id);
    return ESP_ERR_INVALID_STATE;
  }

  if (current_app_id == app_id) {
    ESP_LOGD(TAG, "Already showing app %d", app_id);
    return ESP_OK;
  }

  const app_descriptor_t *app = apps[app_id].descriptor;

  // Create USER app if not created
  if (!apps[app_id].is_created) {
    ESP_LOGI(TAG, "Creating user app: %s", app->name);
    lv_obj_t *screen_obj = app->create();
    if (screen_obj == NULL) {
      ESP_LOGE(TAG, "Failed to create app %s", app->name);
      return ESP_FAIL;
    }

    apps[app_id].screen_obj = screen_obj;
    apps[app_id].is_created = true;

    // Add gesture handler
    if (app->on_gesture != NULL) {
      lv_indev_t *touch = display_manager_get_touch();
      if (touch != NULL) {
        lv_obj_add_event_cb(screen_obj, gesture_event_handler, LV_EVENT_GESTURE, NULL);
      }
    }
  }

  // Call on_hide for current app
  if (current_app_id < APP_MAX && apps[current_app_id].descriptor != NULL) {
    if (apps[current_app_id].descriptor->on_hide != NULL) {
      apps[current_app_id].descriptor->on_hide();
    }
  }

  // Switch screen
  lv_screen_load_anim(apps[app_id].screen_obj, anim, 300, 0, false);

  // Update current
  current_app_id = app_id;

  // Call on_show
  if (app->on_show != NULL) {
    app->on_show();
  }

  ESP_LOGI(TAG, "App shown: %s", app->name);
  return ESP_OK;
}

app_id_t app_manager_get_current(void) {
  return current_app_id;
}

const app_descriptor_t **app_manager_get_user_apps(size_t *count) {
  static const app_descriptor_t *user_apps[APP_MAX];
  size_t n = 0;

  for (app_id_t id = 0; id < APP_MAX; id++) {
    if (apps[id].descriptor != NULL && apps[id].descriptor->type == APP_TYPE_USER) {
      user_apps[n++] = apps[id].descriptor;
    }
  }

  *count = n;
  return user_apps;
}

static void gesture_event_handler(lv_event_t *e) {
  lv_obj_t *target = lv_event_get_target(e);
  if (target == NULL) {
    ESP_LOGE(TAG, "Event target is NULL");
    return;
  }

  app_id_t app_id = get_app_id_from_obj(target);
  if (app_id >= APP_MAX) {
    ESP_LOGW(TAG, "Could not identify app for gesture");
    return;
  }

  lv_indev_t *touch = display_manager_get_touch();
  if (touch == NULL) {
    ESP_LOGE(TAG, "Touch device is NULL");
    return;
  }

  lv_dir_t dir = lv_indev_get_gesture_dir(touch);
  ESP_LOGI(TAG, "Gesture: dir=%d on app=%d", dir, app_id);

  if (apps[app_id].descriptor == NULL) {
    ESP_LOGW(TAG, "App descriptor is NULL");
    return;
  }

  if (apps[app_id].descriptor->on_gesture != NULL) {
    apps[app_id].descriptor->on_gesture(dir);
  }
}
