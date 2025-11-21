#ifndef RTC_HAL_H
#define RTC_HAL_H

#include "esp_err.h"
#include <stdint.h>

/**
 * @brief RTC Time structure (hardware-agnostic)
 */
typedef struct {
  uint8_t seconds; // 0-59
  uint8_t minutes; // 0-59
  uint8_t hours;   // 0-23
  uint8_t day;     // 1-31
  uint8_t weekday; // 0-6 (0=Sunday)
  uint8_t month;   // 1-12
  uint8_t year;    // Year since 2000 (e.g., 24 for 2024)
} rtc_time_t;

/**
 * @brief RTC HAL Interface (vtable pattern)
 *
 * This structure contains function pointers for all RTC operations.
 * Different RTC drivers implement these functions.
 */
typedef struct rtc_hal_interface {
  /**
   * @brief Initialize the RTC hardware with driver-specific config
   */
  esp_err_t (*init)(const void *config); // Generic config pointer

  /**
   * @brief Get current time from RTC
   */
  esp_err_t (*get_time)(rtc_time_t *time);

  /**
   * @brief Set time on RTC
   */
  esp_err_t (*set_time)(const rtc_time_t *time);

  /**
   * @brief Deinitialize the RTC (optional)
   */
  esp_err_t (*deinit)(void);

  /**
   * @brief Driver name (for debugging)
   */
  const char *name;
} rtc_hal_interface_t;

/**
 * @brief Register RTC driver implementation
 *
 * @param interface Pointer to driver's interface implementation
 * @return esp_err_t ESP_OK on success
 */
esp_err_t rtc_hal_register(const rtc_hal_interface_t *interface);

/**
 * @brief Initialize RTC with driver-specific configuration
 * 
 * @param config Driver-specific configuration (driver knows the type)
 */
esp_err_t rtc_hal_init(const void *config);

/**
 * @brief Get current time (calls registered driver's get_time)
 */
esp_err_t rtc_hal_get_time(rtc_time_t *time);

/**
 * @brief Set time (calls registered driver's set_time)
 */
esp_err_t rtc_hal_set_time(const rtc_time_t *time);

/**
 * @brief Deinitialize RTC (calls registered driver's deinit)
 */
esp_err_t rtc_hal_deinit(void);

/**
 * @brief Get registered driver name
 */
const char *rtc_hal_get_driver_name(void);

#endif // RTC_HAL_H
