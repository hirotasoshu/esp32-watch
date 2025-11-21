#ifndef BOARD_INIT_H
#define BOARD_INIT_H

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "lvgl.h"

/**
 * @brief Initialize all board hardware
 * 
 * Initializes in order:
 * - I2C bus
 * - Display HAL
 * - Touch HAL
 * - RTC HAL
 * - LVGL port
 * - LVGL touch input
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t board_init(void);

/**
 * @brief Get LVGL display handle
 * 
 * Must be called after board_init()
 * 
 * @return lv_display_t* LVGL display handle
 */
lv_display_t *board_get_display(void);

/**
 * @brief Get I2C bus handle
 * 
 * Must be called after board_init()
 * 
 * @return i2c_master_bus_handle_t I2C bus handle
 */
i2c_master_bus_handle_t board_get_i2c_bus(void);

/**
 * @brief Get touch input device
 * 
 * Must be called after board_init()
 * 
 * @return lv_indev_t* Touch input device
 */
lv_indev_t *board_get_touch_indev(void);

#endif // BOARD_INIT_H
