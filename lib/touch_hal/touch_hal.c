#include "touch_hal.h"
#include "esp_log.h"

static const char *TAG = "touch_hal";
static const touch_hal_interface_t *touch_driver = NULL;

esp_err_t touch_hal_register(const touch_hal_interface_t *interface) {
  if (interface == NULL) {
    ESP_LOGE(TAG, "Interface is NULL");
    return ESP_ERR_INVALID_ARG;
  }

  if (touch_driver != NULL) {
    ESP_LOGW(TAG, "Touch driver already registered, overwriting");
  }

  touch_driver = interface;
  ESP_LOGI(TAG, "Touch driver registered: %s", interface->name);
  return ESP_OK;
}

esp_err_t touch_hal_init(const void *config) {
  if (touch_driver == NULL || touch_driver->init == NULL) {
    ESP_LOGE(TAG, "Touch driver not registered");
    return ESP_ERR_INVALID_STATE;
  }

  return touch_driver->init(config);
}

esp_err_t touch_hal_deinit(void) {
  if (touch_driver == NULL || touch_driver->deinit == NULL) {
    ESP_LOGE(TAG, "Touch driver not registered");
    return ESP_ERR_INVALID_STATE;
  }

  return touch_driver->deinit();
}

esp_lcd_touch_handle_t touch_hal_get_handle(void) {
  if (touch_driver == NULL || touch_driver->get_handle == NULL) {
    ESP_LOGE(TAG, "Touch driver not registered");
    return NULL;
  }

  return touch_driver->get_handle();
}

SemaphoreHandle_t touch_hal_get_semaphore(void) {
  if (touch_driver == NULL || touch_driver->get_semaphore == NULL) {
    ESP_LOGE(TAG, "Touch driver not registered or doesn't support semaphore");
    return NULL;
  }

  return touch_driver->get_semaphore();
}

const char *touch_hal_get_driver_name(void) {
  if (touch_driver == NULL) {
    return "none";
  }
  return touch_driver->name;
}
