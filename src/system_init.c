#include "system_init.h"
#include "board_init.h"
#include "core/app_manager.h"
#include "core/event_manager.h"
#include "core/navigation_manager.h"
#include "core/screen_manager.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "services/battery_service.h"
#include "services/steps_service.h"
#include "services/notification_service.h"
#include "services/time_service.h"
#include "ui/apps/system_info_app.h"
#include "ui/screens/app_launcher_screen.h"
#include "ui/screens/control_center_screen.h"
#include "ui/screens/notifications_screen.h"
#include "ui/screens/quick_access_screen.h"
#include "ui/screens/watchface_screen.h"

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
  
  ret = screen_manager_init(display, touch_indev);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init screen manager");
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

  // Step 3: Screen Registration
  ESP_LOGI(TAG, "[3/4] Registering screens...");

  lvgl_port_lock(-1);

  ret = screen_manager_register(watchface_screen_get_descriptor());
  if (ret != ESP_OK) {
    lvgl_port_unlock();
    ESP_LOGE(TAG, "Failed to register watchface");
    return ret;
  }

  ret = screen_manager_register(app_launcher_screen_get_descriptor());
  if (ret != ESP_OK) {
    lvgl_port_unlock();
    ESP_LOGE(TAG, "Failed to register app launcher");
    return ret;
  }

  ret = screen_manager_register(quick_access_screen_get_descriptor());
  if (ret != ESP_OK) {
    lvgl_port_unlock();
    ESP_LOGE(TAG, "Failed to register quick access");
    return ret;
  }

  ret = screen_manager_register(notifications_screen_get_descriptor());
  if (ret != ESP_OK) {
    lvgl_port_unlock();
    ESP_LOGE(TAG, "Failed to register notifications");
    return ret;
  }

  ret = screen_manager_register(control_center_screen_get_descriptor());
  if (ret != ESP_OK) {
    lvgl_port_unlock();
    ESP_LOGE(TAG, "Failed to register control center");
    return ret;
  }

  lvgl_port_unlock();

  ESP_LOGI(TAG, "All screens registered (5 total)");

  // Step 4: App Registration
  ESP_LOGI(TAG, "[4/4] Registering apps...");

  ret = app_manager_register(system_info_app_get_descriptor());
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register system info app");
    return ret;
  }

  ESP_LOGI(TAG, "All apps registered (1 total)");

  ESP_LOGI(TAG, "=== System Initialization Complete ===");
  return ESP_OK;
}
