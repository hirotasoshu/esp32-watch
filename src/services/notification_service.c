#include "services/notification_service.h"
#include "core/event_manager.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "notification_service";

#define MAX_NOTIFICATIONS 50

/**
 * @brief Notification storage (STACK - newest at index 0)
 */
static notification_t notifications[MAX_NOTIFICATIONS];
static uint32_t notification_count = 0;

/**
 * @brief Event callback - saves notifications to history
 */
static void notification_event_callback(const event_t *event, void *user_data) {
  if (event->type == EVENT_NOTIFICATION_NEW && event->data != NULL) {
    notification_t *notif = (notification_t *)event->data;

    // Store in history (STACK - newest at index 0)
    if (notification_count < MAX_NOTIFICATIONS) {
      // Shift all notifications right (make room at index 0)
      memmove(&notifications[1], &notifications[0],
              sizeof(notification_t) * notification_count);

      // Insert new notification at index 0
      memcpy(&notifications[0], notif, sizeof(notification_t));
      notification_count++;
    } else {
      // Array full - drop oldest (at index MAX_NOTIFICATIONS - 1)
      // Shift all left (dropping last element)
      memmove(&notifications[1], &notifications[0],
              sizeof(notification_t) * (MAX_NOTIFICATIONS - 1));

      // Insert new notification at index 0
      memcpy(&notifications[0], notif, sizeof(notification_t));
    }

    ESP_LOGI(TAG, "Notification saved: '%s' from '%s' (total: %d)",
             notif->title, notif->app_name, notification_count);
  }
}

esp_err_t notification_service_init(void) {
  memset(notifications, 0, sizeof(notifications));
  notification_count = 0;

  // Subscribe to notification events (to save to history)
  esp_err_t ret = event_manager_subscribe(EVENT_NOTIFICATION_NEW,
                                          notification_event_callback, NULL);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to subscribe to events: %s", esp_err_to_name(ret));
    return ret;
  }

  // Add fake notifications for demo
  strncpy(notifications[0].app_name, "GitHub", sizeof(notifications[0].app_name) - 1);
  strncpy(notifications[0].title, "New pull request", sizeof(notifications[0].title) - 1);
  strncpy(notifications[0].message, "John opened PR #42", sizeof(notifications[0].message) - 1);
  notifications[0].id = 0;
  
  strncpy(notifications[1].app_name, "Messages", sizeof(notifications[1].app_name) - 1);
  strncpy(notifications[1].title, "Alice", sizeof(notifications[1].title) - 1);
  strncpy(notifications[1].message, "Hey! Are you free today?", sizeof(notifications[1].message) - 1);
  notifications[1].id = 1;
  
  strncpy(notifications[2].app_name, "Calendar", sizeof(notifications[2].app_name) - 1);
  strncpy(notifications[2].title, "Meeting", sizeof(notifications[2].title) - 1);
  strncpy(notifications[2].message, "Team sync in 15 minutes", sizeof(notifications[2].message) - 1);
  notifications[2].id = 2;
  
  notification_count = 3;

  ESP_LOGI(TAG, "Notification service initialized with %d demo notifications", notification_count);
  return ESP_OK;
}

uint32_t notification_service_get_count(void) { return notification_count; }

const notification_t *notification_service_get_all(uint32_t *out_count) {
  if (out_count == NULL) {
    ESP_LOGE(TAG, "out_count is NULL");
    return NULL;
  }

  *out_count = notification_count;

  // Return pointer to array (index 0 = newest, index count-1 = oldest)
  return notifications;
}

esp_err_t notification_service_clear_all(void) {
  notification_count = 0;
  memset(notifications, 0, sizeof(notifications));

  ESP_LOGI(TAG, "All notifications cleared");

  // Emit event
  event_manager_emit_simple(EVENT_NOTIFICATION_CLEAR);

  return ESP_OK;
}

esp_err_t notification_service_clear(uint32_t id) {
  for (uint32_t i = 0; i < notification_count; i++) {
    if (notifications[i].id == id) {
      // Shift array left to remove this notification
      if (i < notification_count - 1) {
        memmove(&notifications[i], &notifications[i + 1],
                sizeof(notification_t) * (notification_count - i - 1));
      }
      notification_count--;

      ESP_LOGI(TAG, "Notification %d cleared", id);
      event_manager_emit_simple(EVENT_NOTIFICATION_CLEAR);
      return ESP_OK;
    }
  }

  ESP_LOGW(TAG, "Notification %d not found", id);
  return ESP_ERR_NOT_FOUND;
}
