#include "display_manager.h"
#include "display_hal.h"
#include "esp_log.h"

static const char *TAG = "display_mgr";

static lv_display_t *lvgl_display = NULL;
static lv_indev_t *touch_indev = NULL;
static uint8_t current_brightness = 255;

esp_err_t display_manager_init(lv_display_t *display, lv_indev_t *indev) {
  if (display == NULL) {
    ESP_LOGE(TAG, "Display is NULL");
    return ESP_ERR_INVALID_ARG;
  }
  
  lvgl_display = display;
  touch_indev = indev;
  current_brightness = 255;
  
  if (indev == NULL) {
    ESP_LOGI(TAG, "Display manager initialized (display=%p, touch disabled)", display);
  } else {
    ESP_LOGI(TAG, "Display manager initialized (display=%p, indev=%p)", display, indev);
  }
  
  return ESP_OK;
}

esp_err_t display_manager_set_brightness(uint8_t level) {
  esp_err_t ret = display_hal_set_brightness(level);
  if (ret == ESP_OK) {
    current_brightness = level;
    ESP_LOGI(TAG, "Brightness set to %d", level);
  }
  return ret;
}

uint8_t display_manager_get_brightness(void) {
  return current_brightness;
}

esp_err_t display_manager_sleep(void) {
  ESP_LOGI(TAG, "Display sleep");
  return display_hal_sleep();
}

esp_err_t display_manager_wakeup(void) {
  ESP_LOGI(TAG, "Display wakeup");
  return display_hal_wakeup();
}

lv_display_t *display_manager_get_display(void) {
  return lvgl_display;
}

lv_indev_t *display_manager_get_touch(void) {
  return touch_indev;
}
