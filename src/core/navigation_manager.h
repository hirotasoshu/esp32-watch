#ifndef NAVIGATION_MANAGER_H
#define NAVIGATION_MANAGER_H

#include "esp_err.h"
#include "lvgl.h"

/**
 * @brief Navigation context (where user is currently)
 */
typedef enum {
  NAV_CONTEXT_WATCHFACE,     // On watchface screen
  NAV_CONTEXT_SYSTEM_SCREEN, // On system screen (launcher, quick access, etc)
  NAV_CONTEXT_APP,           // Inside an application
} nav_context_t;

/**
 * @brief Initialize navigation manager
 */
esp_err_t navigation_manager_init(void);

/**
 * @brief Set current navigation context
 *
 * @param context New context
 */
void navigation_manager_set_context(nav_context_t context);

/**
 * @brief Get current navigation context
 *
 * @return nav_context_t Current context
 */
nav_context_t navigation_manager_get_context(void);

/**
 * @brief Handle gesture event
 *
 * This decides where to navigate based on current context and gesture direction
 *
 * @param direction Gesture direction from LVGL
 */
void navigation_manager_handle_gesture(lv_dir_t direction);

#endif // NAVIGATION_MANAGER_H
