#ifndef NOTIFICATION_SERVICE_H
#define NOTIFICATION_SERVICE_H

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Notification type
 */
typedef enum {
  NOTIF_TYPE_APP,         // App notification (WhatsApp, Instagram, Gmail, etc.)
  NOTIF_TYPE_MISSED_CALL, // Missed call
  NOTIF_TYPE_SYSTEM,      // System notification (low battery, etc.)
} notification_type_t;

/**
 * @brief Notification data structure
 */
typedef struct {
  uint32_t id;
  notification_type_t type;
  char title[64];     // e.g. "John Doe", "WhatsApp", "System"
  char message[256];  // e.g. "Hey, what's up?", "Missed call at 14:35"
  char app_name[32];  // e.g. "WhatsApp", "Phone", "Instagram"
  uint32_t timestamp; // Unix timestamp
} notification_t;

/**
 * @brief Initialize notification service
 *
 * This subscribes to EVENT_NOTIFICATION_NEW to save notifications to history
 */
esp_err_t notification_service_init(void);

/**
 * @brief Get notification count
 *
 * @return uint32_t Number of notifications in history
 */
uint32_t notification_service_get_count(void);

/**
 * @brief Get all notifications
 *
 * Returns pointer to internal notification array (read-only).
 * Array is ordered: most recent notification at the END (index = count - 1)
 *
 * @param out_count Output: number of notifications
 * @return const notification_t* Pointer to notification array (read-only, do
 * not modify!)
 */
const notification_t *notification_service_get_all(uint32_t *out_count);

/**
 * @brief Clear all notifications
 *
 * Emits EVENT_NOTIFICATION_CLEAR
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t notification_service_clear_all(void);

/**
 * @brief Clear specific notification by ID
 *
 * @param id Notification ID
 * @return esp_err_t ESP_OK on success, ESP_ERR_NOT_FOUND if not found
 */
esp_err_t notification_service_clear(uint32_t id);

#endif // NOTIFICATION_SERVICE_H
