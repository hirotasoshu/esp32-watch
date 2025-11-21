#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "esp_err.h"
#include "lvgl.h"

/**
 * @brief Screen identifiers
 */
typedef enum {
  SCREEN_WATCHFACE,
  SCREEN_APP_LAUNCHER,
  SCREEN_QUICK_ACCESS,
  SCREEN_NOTIFICATIONS,
  SCREEN_CONTROL_CENTER,
  SCREEN_MAX
} screen_id_t;

/**
 * @brief Screen descriptor
 *
 * Screens are created once at startup and never destroyed
 */
typedef struct {
  screen_id_t id;
  const char *name;

  lv_obj_t *(*create)(void);              // Called ONCE at startup
  void (*on_show)(void);                  // Called when screen becomes visible
  void (*on_hide)(void);                  // Called when screen becomes hidden
  void (*on_gesture)(lv_dir_t direction); // Handle gestures on this screen
} screen_descriptor_t;

/**
 * @brief Initialize screen manager
 *
 * @param display LVGL display handle
 * @param indev LVGL touch input device (can be NULL)
 */
esp_err_t screen_manager_init(lv_display_t *display, lv_indev_t *indev);

/**
 * @brief Register and create a screen
 *
 * This creates the screen immediately and keeps it in memory
 */
esp_err_t screen_manager_register(const screen_descriptor_t *descriptor);

/**
 * @brief Show a screen (switch to it)
 */
esp_err_t screen_manager_show(screen_id_t screen_id, lv_scr_load_anim_t anim);

/**
 * @brief Get current screen ID
 */
screen_id_t screen_manager_get_current(void);

/**
 * @brief Handle gesture event
 */
void screen_manager_handle_gesture(lv_dir_t direction);

#endif // SCREEN_MANAGER_H
