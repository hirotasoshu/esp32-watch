#include "app_manager.h"
#include "esp_log.h"
#include "screen_manager.h"
#include <string.h>

static const char *TAG = "app_manager";

static const app_descriptor_t *apps[APP_MAX] = {0};
static app_id_t current_app_id = APP_MAX; // APP_MAX means no app running
static lv_obj_t *current_app_screen = NULL;
static size_t registered_app_count = 0;

esp_err_t app_manager_init(void) {
  memset(apps, 0, sizeof(apps));
  current_app_id = APP_MAX;
  registered_app_count = 0;

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

  if (apps[app->id] != NULL) {
    ESP_LOGW(TAG, "App %s already registered, overwriting", app->name);
  } else {
    registered_app_count++;
  }

  apps[app->id] = app;

  // Call init if available
  if (app->init != NULL) {
    esp_err_t ret = app->init();
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize app %s: %d", app->name, ret);
      apps[app->id] = NULL;
      registered_app_count--;
      return ret;
    }
  }

  ESP_LOGI(TAG, "App registered: %s (id=%d)", app->name, app->id);
  return ESP_OK;
}

esp_err_t app_manager_launch(app_id_t app_id) {
  if (app_id >= APP_MAX) {
    ESP_LOGE(TAG, "Invalid app ID: %d", app_id);
    return ESP_ERR_INVALID_ARG;
  }

  const app_descriptor_t *app = apps[app_id];
  if (app == NULL) {
    ESP_LOGE(TAG, "App %d not registered", app_id);
    return ESP_ERR_INVALID_STATE;
  }

  if (app->create_ui == NULL) {
    ESP_LOGE(TAG, "App %s has no create_ui function", app->name);
    return ESP_ERR_INVALID_STATE;
  }

  // Pause current app if any
  if (current_app_id < APP_MAX && apps[current_app_id] != NULL) {
    if (apps[current_app_id]->on_pause != NULL) {
      apps[current_app_id]->on_pause();
    }
  }

  // Create app UI
  lv_obj_t *app_screen = app->create_ui();
  if (app_screen == NULL) {
    ESP_LOGE(TAG, "Failed to create UI for app %s", app->name);
    return ESP_FAIL;
  }

  // Load app screen
  lv_screen_load_anim(app_screen, LV_SCR_LOAD_ANIM_MOVE_TOP, 300, 0, false);

  // Update state
  current_app_id = app_id;
  current_app_screen = app_screen;

  // Call on_launch
  if (app->on_launch != NULL) {
    app->on_launch();
  }

  ESP_LOGI(TAG, "App launched: %s", app->name);
  return ESP_OK;
}

esp_err_t app_manager_close_current(void) {
  if (current_app_id >= APP_MAX) {
    ESP_LOGW(TAG, "No app currently running");
    return ESP_ERR_INVALID_STATE;
  }

  const app_descriptor_t *app = apps[current_app_id];
  if (app == NULL) {
    ESP_LOGW(TAG, "Current app not found");
    current_app_id = APP_MAX;
    return ESP_ERR_INVALID_STATE;
  }

  // Call on_close
  if (app->on_close != NULL) {
    app->on_close();
  }

  // Destroy UI if handler exists
  if (app->destroy_ui != NULL && current_app_screen != NULL) {
    app->destroy_ui(current_app_screen);
  }

  ESP_LOGI(TAG, "App closed: %s", app->name);

  // Return to watchface
  current_app_id = APP_MAX;
  current_app_screen = NULL;

  return screen_manager_show(SCREEN_WATCHFACE, LV_SCR_LOAD_ANIM_MOVE_BOTTOM);
}

app_id_t app_manager_get_current(void) { return current_app_id; }

const app_descriptor_t **app_manager_get_all_apps(size_t *count) {
  if (count != NULL) {
    *count = registered_app_count;
  }
  return (const app_descriptor_t **)apps;
}
