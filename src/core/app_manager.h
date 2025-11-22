#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include "esp_err.h"
#include "lvgl.h"

typedef enum {
  APP_TYPE_WATCHFACE,
  APP_TYPE_SYSTEM,
  APP_TYPE_USER
} app_type_t;

typedef enum {
  APP_WATCHFACE,
  APP_SYSTEM_LAUNCHER,
  APP_SYSTEM_QUICK_ACCESS,
  APP_SYSTEM_NOTIFICATIONS,
  APP_SYSTEM_CONTROL_CENTER,
  APP_USER_SYSTEM_INFO,
  APP_MAX
} app_id_t;

typedef struct {
  app_id_t id;
  app_type_t type;
  const char *name;
  const lv_img_dsc_t *icon;
  lv_obj_t *(*create)(void);
  void (*destroy)(lv_obj_t *screen);
  void (*on_show)(void);
  void (*on_hide)(void);
  void (*on_gesture)(lv_dir_t direction);
} app_descriptor_t;

esp_err_t app_manager_init(void);
esp_err_t app_manager_register(const app_descriptor_t *app);
esp_err_t app_manager_show(app_id_t app_id, lv_scr_load_anim_t anim);
app_id_t app_manager_get_current(void);
const app_descriptor_t **app_manager_get_user_apps(size_t *count);

#endif
