#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include "esp_err.h"
#include "lvgl.h"

/**
 * @brief Application identifiers
 */
typedef enum {
  APP_SYSTEM_INFO,
  APP_MAX // Keep this last
} app_id_t;

/**
 * @brief Application descriptor
 */
typedef struct {
  app_id_t id;
  const char *name;
  const lv_img_dsc_t *icon; // Icon for app launcher (can be NULL)

  esp_err_t (*init)(void); // Initialize app
  void (*deinit)(void);    // Deinitialize app

  lv_obj_t *(*create_ui)(void);         // Create app UI
  void (*destroy_ui)(lv_obj_t *screen); // Destroy app UI

  void (*on_launch)(void); // Called when app is launched
  void (*on_close)(void);  // Called when app is closed
  void (*on_pause)(void);  // Called when app is paused
  void (*on_resume)(void); // Called when app is resumed
} app_descriptor_t;

/**
 * @brief Initialize app manager
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t app_manager_init(void);

/**
 * @brief Register an application
 *
 * @param app Application descriptor
 * @return esp_err_t ESP_OK on success
 */
esp_err_t app_manager_register(const app_descriptor_t *app);

/**
 * @brief Launch an application
 *
 * @param app_id Application ID
 * @return esp_err_t ESP_OK on success
 */
esp_err_t app_manager_launch(app_id_t app_id);

/**
 * @brief Close current application
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t app_manager_close_current(void);

/**
 * @brief Get current application ID
 *
 * @return app_id_t Current app ID (APP_MAX if none)
 */
app_id_t app_manager_get_current(void);

/**
 * @brief Get all registered applications
 *
 * @param count Output: number of apps
 * @return const app_descriptor_t** Array of app descriptors
 */
const app_descriptor_t **app_manager_get_all_apps(size_t *count);

#endif // APP_MANAGER_H
