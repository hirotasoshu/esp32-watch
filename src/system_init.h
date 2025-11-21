#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H

#include "esp_err.h"
#include "lvgl.h"

/**
 * @brief Initialize system-level components
 * 
 * This includes:
 * - Core managers (event, screen, navigation, app)
 * - Services (time, notification)
 * - Screen registration (watchface, launcher, etc.)
 * - App registration
 * 
 * @param display LVGL display handle (from board_init)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t system_init(lv_display_t *display);

#endif // SYSTEM_INIT_H
