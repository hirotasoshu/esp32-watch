#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "esp_err.h"
#include "lvgl.h"

/**
 * @brief Initialize display manager
 */
esp_err_t display_manager_init(lv_display_t *display, lv_indev_t *indev);

/**
 * @brief Set display brightness (0-255)
 */
esp_err_t display_manager_set_brightness(uint8_t level);

/**
 * @brief Get current brightness
 */
uint8_t display_manager_get_brightness(void);

/**
 * @brief Put display to sleep
 */
esp_err_t display_manager_sleep(void);

/**
 * @brief Wake up display
 */
esp_err_t display_manager_wakeup(void);

/**
 * @brief Get LVGL display handle
 */
lv_display_t *display_manager_get_display(void);

/**
 * @brief Get touch input device handle
 */
lv_indev_t *display_manager_get_touch(void);

#endif // DISPLAY_MANAGER_H
