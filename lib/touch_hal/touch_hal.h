#ifndef TOUCH_HAL_H
#define TOUCH_HAL_H

#include "esp_err.h"
#include "esp_lcd_touch.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/**
 * @brief Touch HAL interface
 */
typedef struct {
  esp_err_t (*init)(const void *config);
  esp_err_t (*deinit)(void);
  esp_lcd_touch_handle_t (*get_handle)(void);
  SemaphoreHandle_t (*get_semaphore)(void); // Для ISR callback
  const char *name;
} touch_hal_interface_t;

/**
 * @brief Register touch driver
 */
esp_err_t touch_hal_register(const touch_hal_interface_t *interface);

/**
 * @brief Initialize touch controller
 */
esp_err_t touch_hal_init(const void *config);

/**
 * @brief Deinitialize touch controller
 */
esp_err_t touch_hal_deinit(void);

/**
 * @brief Get touch handle for LVGL
 */
esp_lcd_touch_handle_t touch_hal_get_handle(void);

/**
 * @brief Get touch semaphore (for ISR-based reading)
 */
SemaphoreHandle_t touch_hal_get_semaphore(void);

/**
 * @brief Get driver name
 */
const char *touch_hal_get_driver_name(void);

#endif // TOUCH_HAL_H
