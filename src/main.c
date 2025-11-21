#include "board_init.h"
#include "core/screen_manager.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "system_init.h"

static const char *TAG = "main";

void app_main(void) {
  ESP_LOGI(TAG, "========================================");
  ESP_LOGI(TAG, "    ESP32 Watch Firmware Starting");
  ESP_LOGI(TAG, "========================================");

  // Initialize board hardware (I2C, Display, Touch, RTC, LVGL)
  ESP_ERROR_CHECK(board_init());

  // Get display handle from board
  lv_display_t *display = board_get_display();

  // Initialize system (managers, services, screens, apps)
  ESP_ERROR_CHECK(system_init(display));

  // Load watchface screen
  ESP_LOGI(TAG, "Loading watchface...");
  lvgl_port_lock(-1);
  ESP_ERROR_CHECK(screen_manager_show(SCREEN_WATCHFACE, LV_SCR_LOAD_ANIM_NONE));
  lvgl_port_unlock();

  ESP_LOGI(TAG, "========================================");
  ESP_LOGI(TAG, "    ESP32 Watch Ready!");
  ESP_LOGI(TAG, "========================================");
}
