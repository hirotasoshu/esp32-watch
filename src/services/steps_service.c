#include "services/steps_service.h"
#include "core/event_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "steps_service";

static uint32_t step_count = 3420; // Simulated steps
static uint32_t step_goal = 10000;

/**
 * @brief Steps update task - simulates step counting
 */
static void steps_update_task(void *arg) {
  TickType_t last_wake = xTaskGetTickCount();

  while (1) {
    vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(5000)); // Every 5 seconds

    // Simulate step increment (demo)
    step_count += 500;
    if (step_count > step_goal) {
      step_count = 0; // Reset for demo
    }

    // Emit steps update event
    event_manager_emit(EVENT_STEPS_UPDATED, &step_count, sizeof(step_count));
  }
}

esp_err_t steps_service_init(void) {
  ESP_LOGI(TAG, "Steps service initialized (simulated)");

  // Create steps update task
  xTaskCreate(steps_update_task, "steps_update", 2048, NULL, 2, NULL);

  return ESP_OK;
}

uint32_t steps_service_get_count(void) { return step_count; }

uint32_t steps_service_get_goal(void) { return step_goal; }
