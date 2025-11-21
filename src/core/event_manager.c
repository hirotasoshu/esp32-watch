#include "core/event_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>

static const char *TAG = "event_manager";

#define MAX_SUBSCRIBERS_PER_EVENT 10

typedef struct {
  event_callback_t callback;
  void *user_data;
  bool active;
} subscriber_t;

static subscriber_t subscribers[EVENT_MAX][MAX_SUBSCRIBERS_PER_EVENT];
static SemaphoreHandle_t event_mutex = NULL;

esp_err_t event_manager_init(void) {
  memset(subscribers, 0, sizeof(subscribers));

  event_mutex = xSemaphoreCreateMutex();
  if (event_mutex == NULL) {
    ESP_LOGE(TAG, "Failed to create event mutex");
    return ESP_ERR_NO_MEM;
  }

  ESP_LOGI(TAG, "Event manager initialized");
  return ESP_OK;
}

esp_err_t event_manager_subscribe(event_type_t event_type,
                                  event_callback_t callback, void *user_data) {
  if (event_type >= EVENT_MAX) {
    ESP_LOGE(TAG, "Invalid event type: %d", event_type);
    return ESP_ERR_INVALID_ARG;
  }

  if (callback == NULL) {
    ESP_LOGE(TAG, "Callback is NULL");
    return ESP_ERR_INVALID_ARG;
  }

  xSemaphoreTake(event_mutex, portMAX_DELAY);

  for (int i = 0; i < MAX_SUBSCRIBERS_PER_EVENT; i++) {
    if (!subscribers[event_type][i].active) {
      subscribers[event_type][i].callback = callback;
      subscribers[event_type][i].user_data = user_data;
      subscribers[event_type][i].active = true;

      xSemaphoreGive(event_mutex);
      ESP_LOGD(TAG, "Subscribed to event %d (slot %d)", event_type, i);
      return ESP_OK;
    }
  }

  xSemaphoreGive(event_mutex);
  ESP_LOGE(TAG, "No free slots for event %d", event_type);
  return ESP_ERR_NO_MEM;
}

esp_err_t event_manager_unsubscribe(event_type_t event_type,
                                    event_callback_t callback) {
  if (event_type >= EVENT_MAX || callback == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  xSemaphoreTake(event_mutex, portMAX_DELAY);

  bool found = false;
  for (int i = 0; i < MAX_SUBSCRIBERS_PER_EVENT; i++) {
    if (subscribers[event_type][i].active &&
        subscribers[event_type][i].callback == callback) {
      subscribers[event_type][i].active = false;
      subscribers[event_type][i].callback = NULL;
      subscribers[event_type][i].user_data = NULL;
      found = true;
      ESP_LOGD(TAG, "Unsubscribed from event %d", event_type);
      break;
    }
  }

  xSemaphoreGive(event_mutex);
  return found ? ESP_OK : ESP_ERR_NOT_FOUND;
}

esp_err_t event_manager_emit(event_type_t event_type, void *data,
                             uint32_t data_size) {
  if (event_type >= EVENT_MAX) {
    ESP_LOGE(TAG, "Invalid event type: %d", event_type);
    return ESP_ERR_INVALID_ARG;
  }

  xSemaphoreTake(event_mutex, portMAX_DELAY);

  event_t event = {.type = event_type, .data = data, .data_size = data_size};

  int callback_count = 0;
  for (int i = 0; i < MAX_SUBSCRIBERS_PER_EVENT; i++) {
    if (subscribers[event_type][i].active) {
      subscribers[event_type][i].callback(&event,
                                          subscribers[event_type][i].user_data);
      callback_count++;
    }
  }

  xSemaphoreGive(event_mutex);

  if (callback_count > 0) {
    ESP_LOGD(TAG, "Event %d emitted to %d subscribers", event_type,
             callback_count);
  }

  return ESP_OK;
}

esp_err_t event_manager_emit_simple(event_type_t event_type) {
  return event_manager_emit(event_type, NULL, 0);
}
