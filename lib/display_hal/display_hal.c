#include "display_hal.h"
#include "esp_log.h"

static const char *TAG = "display_hal";
static const display_hal_interface_t *display_driver = NULL;

esp_err_t display_hal_register(const display_hal_interface_t *interface) {
  if (interface == NULL) {
    ESP_LOGE(TAG, "Interface is NULL");
    return ESP_ERR_INVALID_ARG;
  }

  if (display_driver != NULL) {
    ESP_LOGW(TAG, "Display driver already registered, overwriting");
  }

  display_driver = interface;
  ESP_LOGI(TAG, "Display driver registered: %s", interface->name);
  return ESP_OK;
}

esp_err_t display_hal_init(const void *config) {
  if (display_driver == NULL || display_driver->init == NULL) {
    ESP_LOGE(TAG, "Display driver not registered");
    return ESP_ERR_INVALID_STATE;
  }

  return display_driver->init(config);
}

esp_err_t display_hal_set_brightness(uint8_t level) {
  if (display_driver == NULL || display_driver->set_brightness == NULL) {
    ESP_LOGE(TAG,
             "Display driver not registered or doesn't support brightness");
    return ESP_ERR_NOT_SUPPORTED;
  }

  return display_driver->set_brightness(level);
}

esp_err_t display_hal_sleep(void) {
  if (display_driver == NULL || display_driver->sleep == NULL) {
    ESP_LOGE(TAG, "Display driver not registered or doesn't support sleep");
    return ESP_ERR_NOT_SUPPORTED;
  }

  return display_driver->sleep();
}

esp_err_t display_hal_wakeup(void) {
  if (display_driver == NULL || display_driver->wakeup == NULL) {
    ESP_LOGE(TAG, "Display driver not registered or doesn't support wakeup");
    return ESP_ERR_NOT_SUPPORTED;
  }

  return display_driver->wakeup();
}

lv_display_t *display_hal_get_lvgl_display(void) {
  if (display_driver == NULL || display_driver->get_lvgl_display == NULL) {
    ESP_LOGE(TAG, "Display driver not registered");
    return NULL;
  }

  return display_driver->get_lvgl_display();
}

const char *display_hal_get_driver_name(void) {
  if (display_driver == NULL) {
    return "none";
  }
  return display_driver->name;
}
