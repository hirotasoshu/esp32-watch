#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include "esp_err.h"
#include <stdint.h>
#include <time.h>

/**
 * @brief Initialize time service
 *
 * Starts a task that reads RTC every second and emits EVENT_TIME_UPDATED
 */
esp_err_t time_service_init(void);

/**
 * @brief Get current time
 *
 * @param out_time Output time structure
 * @return esp_err_t ESP_OK on success
 */
esp_err_t time_service_get_time(struct tm *out_time);

/**
 * @brief Set current time
 *
 * @param time Time to set
 * @return esp_err_t ESP_OK on success
 */
esp_err_t time_service_set_time(const struct tm *time);

#endif // TIME_SERVICE_H
