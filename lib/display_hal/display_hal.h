#ifndef DISPLAY_HAL_H
#define DISPLAY_HAL_H

#include "esp_err.h"
#include "lvgl.h"

/**
 * @brief Display HAL interface
 */
typedef struct {
  esp_err_t (*init)(const void *config); // Generic config pointer
  esp_err_t (*set_brightness)(uint8_t level);
  esp_err_t (*sleep)(void);
  esp_err_t (*wakeup)(void);
  lv_display_t *(*get_lvgl_display)(void);
  const char *name;
} display_hal_interface_t;

/**
 * @brief Register display driver
 */
esp_err_t display_hal_register(const display_hal_interface_t *interface);

/**
 * @brief Initialize display with driver-specific configuration
 * 
 * @param config Driver-specific configuration (driver knows the type)
 */
esp_err_t display_hal_init(const void *config);

/**
 * @brief Set display brightness (0-255)
 */
esp_err_t display_hal_set_brightness(uint8_t level);

/**
 * @brief Put display to sleep
 */
esp_err_t display_hal_sleep(void);

/**
 * @brief Wake up display
 */
esp_err_t display_hal_wakeup(void);

/**
 * @brief Get LVGL display handle
 */
lv_display_t *display_hal_get_lvgl_display(void);

/**
 * @brief Get driver name
 */
const char *display_hal_get_driver_name(void);

#endif // DISPLAY_HAL_H
