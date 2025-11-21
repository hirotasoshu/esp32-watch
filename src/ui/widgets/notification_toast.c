#include "ui/widgets/notification_toast.h"
#include "core/event_manager.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "ui/theme.h"

static const char *TAG = "notification_toast";

static lv_obj_t *toast_container = NULL;
static lv_timer_t *dismiss_timer = NULL;

/**
 * @brief Timer callback to auto-dismiss toast
 */
static void toast_auto_dismiss_cb(lv_timer_t *timer) {
  notification_toast_dismiss();
  dismiss_timer = NULL;
}

/**
 * @brief Gesture callback (swipe right to dismiss)
 */
static void toast_gesture_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());

  if (code == LV_EVENT_GESTURE && dir == LV_DIR_RIGHT) {
    ESP_LOGI(TAG, "Toast swiped right, dismissing");
    notification_toast_dismiss();
  }
}

/**
 * @brief Event callback for new notifications
 */
static void toast_event_callback(const event_t *event, void *user_data) {
  if (event->type == EVENT_NOTIFICATION_NEW && event->data != NULL) {
    // Use notification data from event directly
    notification_t *notif = (notification_t *)event->data;
    notification_toast_show(notif);
  }
}

esp_err_t notification_toast_init(void) {
  // Subscribe to notification events
  esp_err_t ret = event_manager_subscribe(EVENT_NOTIFICATION_NEW,
                                          toast_event_callback, NULL);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to subscribe to events: %s", esp_err_to_name(ret));
    return ret;
  }

  ESP_LOGI(TAG, "Notification toast initialized");
  return ESP_OK;
}

void notification_toast_show(const notification_t *notification) {
  if (notification == NULL) {
    ESP_LOGE(TAG, "Notification is NULL");
    return;
  }

  // Dismiss existing toast if any
  if (toast_container != NULL) {
    notification_toast_dismiss();
  }

  lvgl_port_lock(-1);

  // Get current active screen
  lv_obj_t *active_screen = lv_scr_act();

  // Create toast container
  toast_container = lv_obj_create(active_screen);
  lv_obj_set_size(toast_container, 220, 70);
  lv_obj_align(toast_container, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_set_style_bg_color(toast_container, lv_color_hex(THEME_COLOR_BLACK),
                            0);
  lv_obj_set_style_bg_opa(toast_container, LV_OPA_90, 0);
  lv_obj_set_style_border_width(toast_container, 2, 0);
  lv_obj_set_style_border_color(toast_container,
                                lv_color_hex(THEME_COLOR_ORANGE), 0);
  lv_obj_set_style_radius(toast_container, 10, 0);
  lv_obj_set_style_pad_all(toast_container, 10, 0);
  lv_obj_clear_flag(toast_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(toast_container, LV_OBJ_FLAG_GESTURE_BUBBLE);
  lv_obj_move_foreground(toast_container);

  // Add gesture detection (swipe right to dismiss)
  lv_obj_add_event_cb(toast_container, toast_gesture_cb, LV_EVENT_GESTURE,
                      NULL);

  // Title label
  lv_obj_t *title = lv_label_create(toast_container);
  lv_label_set_text(title, notification->title);
  lv_obj_set_style_text_font(title, THEME_FONT_NORMAL, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(THEME_COLOR_WHITE), 0);
  lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

  // Message label
  lv_obj_t *message = lv_label_create(toast_container);
  lv_label_set_text(message, notification->message);
  lv_obj_set_style_text_font(message, THEME_FONT_SMALL, 0);
  lv_obj_set_style_text_color(message, lv_color_hex(THEME_COLOR_GRAY), 0);
  lv_obj_set_width(message, 200);
  lv_label_set_long_mode(message, LV_LABEL_LONG_DOT);
  lv_obj_align(message, LV_ALIGN_TOP_LEFT, 0, 22);

  // Slide in animation (from top)
  lv_obj_set_y(toast_container, -70);
  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_var(&anim, toast_container);
  lv_anim_set_values(&anim, -70, 10);
  lv_anim_set_time(&anim, 300);
  lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
  lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
  lv_anim_start(&anim);

  lvgl_port_unlock();

  // Auto-dismiss after 3 seconds
  dismiss_timer = lv_timer_create(toast_auto_dismiss_cb, 3000, NULL);
  lv_timer_set_repeat_count(dismiss_timer, 1);

  ESP_LOGI(TAG, "Toast shown: '%s' - '%s'", notification->title,
           notification->message);
}

void notification_toast_dismiss(void) {
  if (toast_container == NULL) {
    return;
  }

  // Cancel auto-dismiss timer
  if (dismiss_timer != NULL) {
    lv_timer_del(dismiss_timer);
    dismiss_timer = NULL;
  }

  lvgl_port_lock(-1);

  // Slide out animation (to top)
  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_var(&anim, toast_container);
  lv_anim_set_values(&anim, lv_obj_get_y(toast_container), -70);
  lv_anim_set_time(&anim, 200);
  lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
  lv_anim_set_path_cb(&anim, lv_anim_path_ease_in);
  lv_anim_set_deleted_cb(&anim, (lv_anim_deleted_cb_t)lv_obj_del);
  lv_anim_start(&anim);

  lvgl_port_unlock();

  toast_container = NULL;

  ESP_LOGI(TAG, "Toast dismissed");
}
