#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAIN";

void app_main(void) {
  ESP_LOGI(TAG, "ESP32-S3 project initialized (ESP-IDF + PlatformIO)");
  while (1) {
    ESP_LOGI(TAG, "Running...");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
