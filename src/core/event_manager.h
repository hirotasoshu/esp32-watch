#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "esp_err.h"
#include <stdint.h>

/**
 * @brief System event types
 */
typedef enum {
  // Time events
  EVENT_TIME_UPDATED, // data: struct tm*

  // Battery events
  EVENT_BATTERY_UPDATED, // data: uint8_t* (0-100%)
  EVENT_BATTERY_LEVEL,
  EVENT_BATTERY_CHARGING,
  EVENT_BATTERY_LOW,

  // Health events
  EVENT_STEPS_UPDATED, // data: uint32_t* (step count)
  EVENT_HEALTH_STEPS,
  EVENT_HEALTH_HEART_RATE,
  EVENT_HEALTH_GOAL_REACHED,

  // Notification events
  EVENT_NOTIFICATION_NEW,   // New notification arrived
  EVENT_NOTIFICATION_CLEAR, // Notification cleared

  // System events
  EVENT_SYSTEM_SLEEP,
  EVENT_SYSTEM_WAKEUP,

  EVENT_MAX
} event_type_t;

/**
 * @brief Event data structure
 */
typedef struct {
  event_type_t type;
  void *data;
  uint32_t data_size;
} event_t;

/**
 * @brief Event callback function type
 */
typedef void (*event_callback_t)(const event_t *event, void *user_data);

/**
 * @brief Initialize event manager
 */
esp_err_t event_manager_init(void);

/**
 * @brief Subscribe to an event
 */
esp_err_t event_manager_subscribe(event_type_t event_type,
                                  event_callback_t callback, void *user_data);

/**
 * @brief Unsubscribe from an event
 */
esp_err_t event_manager_unsubscribe(event_type_t event_type,
                                    event_callback_t callback);

/**
 * @brief Emit an event
 */
esp_err_t event_manager_emit(event_type_t event_type, void *data,
                             uint32_t data_size);

/**
 * @brief Emit an event without data
 */
esp_err_t event_manager_emit_simple(event_type_t event_type);

#endif // EVENT_MANAGER_H
