#ifndef STEPS_SERVICE_H
#define STEPS_SERVICE_H

#include "esp_err.h"
#include <stdint.h>

/**
 * @brief Initialize steps service
 */
esp_err_t steps_service_init(void);

/**
 * @brief Get current step count
 */
uint32_t steps_service_get_count(void);

/**
 * @brief Get step goal
 */
uint32_t steps_service_get_goal(void);

#endif // STEPS_SERVICE_H
