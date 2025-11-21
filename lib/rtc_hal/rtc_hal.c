#include "rtc_hal.h"
#include "esp_log.h"

static const char *TAG = "RTC_HAL";

// Global pointer to registered RTC driver
static const rtc_hal_interface_t *g_rtc_driver = NULL;

// Register RTC driver
esp_err_t rtc_hal_register(const rtc_hal_interface_t *interface) {
  if (interface == NULL) {
    ESP_LOGE(TAG, "Cannot register NULL interface");
    return ESP_ERR_INVALID_ARG;
  }

  if (interface->init == NULL || interface->get_time == NULL ||
      interface->set_time == NULL) {
    ESP_LOGE(TAG, "Interface has NULL function pointers");
    return ESP_ERR_INVALID_ARG;
  }

  g_rtc_driver = interface;
  ESP_LOGI(TAG, "Registered RTC driver: %s",
           interface->name ? interface->name : "Unknown");
  return ESP_OK;
}

// Initialize RTC
esp_err_t rtc_hal_init(const void *config) {
  if (g_rtc_driver == NULL) {
    ESP_LOGE(TAG, "No RTC driver registered");
    return ESP_ERR_INVALID_STATE;
  }
  return g_rtc_driver->init(config);
}

// Get time
esp_err_t rtc_hal_get_time(rtc_time_t *time) {
  if (g_rtc_driver == NULL) {
    ESP_LOGE(TAG, "No RTC driver registered");
    return ESP_ERR_INVALID_STATE;
  }
  return g_rtc_driver->get_time(time);
}

// Set time
esp_err_t rtc_hal_set_time(const rtc_time_t *time) {
  if (g_rtc_driver == NULL) {
    ESP_LOGE(TAG, "No RTC driver registered");
    return ESP_ERR_INVALID_STATE;
  }
  return g_rtc_driver->set_time(time);
}

// Deinitialize
esp_err_t rtc_hal_deinit(void) {
  if (g_rtc_driver == NULL) {
    ESP_LOGE(TAG, "No RTC driver registered");
    return ESP_ERR_INVALID_STATE;
  }

  if (g_rtc_driver->deinit) {
    return g_rtc_driver->deinit();
  }

  return ESP_OK;
}

// Get driver name
const char *rtc_hal_get_driver_name(void) {
  if (g_rtc_driver == NULL || g_rtc_driver->name == NULL) {
    return "None";
  }
  return g_rtc_driver->name;
}
