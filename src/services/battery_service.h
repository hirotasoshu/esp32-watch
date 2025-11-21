#ifndef BATTERY_SERVICE_H
#define BATTERY_SERVICE_H

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Initialize battery service
 */
esp_err_t battery_service_init(void);

/**
 * @brief Get current battery level (0-100%)
 */
uint8_t battery_service_get_level(void);

/**
 * @brief Check if charging
 */
bool battery_service_is_charging(void);

#endif // BATTERY_SERVICE_H
