#include "services/battery_service.h"
#include "core/event_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "battery_service";

static uint8_t battery_level = 75; // Simulated battery level
static bool is_charging = false;

/**
 * @brief Battery update task - simulates battery discharge
 */
static void battery_update_task(void *arg) {
  TickType_t last_wake = xTaskGetTickCount();

  while (1) {
    vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(5000)); // Every 5 seconds

    // Simulate battery discharge (demo)
    if (!is_charging && battery_level > 0) {
      battery_level = battery_level - 25;
      if (battery_level == 0) {
        battery_level = 100; // Reset for demo
      }
    }

    // Emit battery update event
    event_manager_emit(EVENT_BATTERY_UPDATED, &battery_level,
                       sizeof(battery_level));
  }
}

esp_err_t battery_service_init(void) {
  ESP_LOGI(TAG, "Battery service initialized (simulated)");

  // Create battery update task
  xTaskCreate(battery_update_task, "battery_update", 2048, NULL, 2, NULL);

  return ESP_OK;
}

uint8_t battery_service_get_level(void) { return battery_level; }

bool battery_service_is_charging(void) { return is_charging; }
