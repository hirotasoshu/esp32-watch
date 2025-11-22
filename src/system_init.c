#include "system_init.h"
#include "board_init.h"
#include "core/app_manager.h"
#include "core/display_manager.h"
#include "core/event_manager.h"
#include "core/navigation_manager.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "services/battery_service.h"
#include "services/steps_service.h"
#include "services/notification_service.h"
#include "services/time_service.h"
#include "ui/apps/system_info_app.h"
#include "ui/apps/launcher_app.h"
#include "ui/apps/control_center_app.h"
#include "ui/apps/notifications_app.h"
#include "ui/apps/quick_access_app.h"
#include "ui/apps/watchface_app.h"

static const char *TAG = "system_init";

esp_err_t system_init(lv_display_t *display) {
  if (display == NULL) {
    ESP_LOGE(TAG, "Display is NULL");
    return ESP_ERR_INVALID_ARG;
  }

  esp_err_t ret;

  ESP_LOGI(TAG, "=== System Initialization Started ===");

  // Step 1: Core Managers
  ESP_LOGI(TAG, "[1/4] Initializing core managers...");

  ret = event_manager_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init event manager");
    return ret;
  }

  lv_indev_t *touch_indev = board_get_touch_indev();
  if (touch_indev == NULL) {
    ESP_LOGI(TAG, "Touch input device is NULL - gesture support will be disabled");
  }
  
  ret = display_manager_init(display, touch_indev);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init display manager");
    return ret;
  }

  ret = navigation_manager_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init navigation manager");
    return ret;
  }

  ret = app_manager_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init app manager");
    return ret;
  }

  ESP_LOGI(TAG, "Core managers initialized");

  // Step 2: Services
  ESP_LOGI(TAG, "[2/4] Initializing services...");

  ret = time_service_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init time service");
    return ret;
  }

  ret = battery_service_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init battery service");
    return ret;
  }

  ret = steps_service_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init steps service");
    return ret;
  }

  ret = notification_service_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init notification service");
    return ret;
  }

  ESP_LOGI(TAG, "Services initialized");

  // Step 3: App Registration
  ESP_LOGI(TAG, "[3/4] Registering apps...");

  if (!lvgl_port_lock(pdMS_TO_TICKS(5000))) {
    ESP_LOGE(TAG, "Failed to acquire LVGL lock");
    return ESP_ERR_TIMEOUT;
  }

  ret = app_manager_register(watchface_app_get_descriptor());
  if (ret != ESP_OK) {
    lvgl_port_unlock();
    ESP_LOGE(TAG, "Failed to register watchface");
    return ret;
  }

  ret = app_manager_register(launcher_app_get_descriptor());
  if (ret != ESP_OK) {
    lvgl_port_unlock();
    ESP_LOGE(TAG, "Failed to register launcher");
    return ret;
  }

  ret = app_manager_register(quick_access_app_get_descriptor());
  if (ret != ESP_OK) {
    lvgl_port_unlock();
    ESP_LOGE(TAG, "Failed to register quick access");
    return ret;
  }

  ret = app_manager_register(notifications_app_get_descriptor());
  if (ret != ESP_OK) {
    lvgl_port_unlock();
    ESP_LOGE(TAG, "Failed to register notifications");
    return ret;
  }

  ret = app_manager_register(control_center_app_get_descriptor());
  if (ret != ESP_OK) {
    lvgl_port_unlock();
    ESP_LOGE(TAG, "Failed to register control center");
    return ret;
  }

  ret = app_manager_register(system_info_app_get_descriptor());
  if (ret != ESP_OK) {
    lvgl_port_unlock();
    ESP_LOGE(TAG, "Failed to register system info");
    return ret;
  }

  lvgl_port_unlock();

  ESP_LOGI(TAG, "All apps registered (6 total)");

  ESP_LOGI(TAG, "=== System Initialization Complete ===");
  return ESP_OK;
}
