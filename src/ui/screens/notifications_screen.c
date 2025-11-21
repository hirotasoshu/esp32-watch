#include "ui/screens/notifications_screen.h"
#include "core/navigation_manager.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "services/notification_service.h"
#include "ui/theme.h"

static const char *TAG = "notifications_screen";

static lv_obj_t *notification_list = NULL;

/**
 * @brief Create a single notification item widget
 */
static void create_notification_item(lv_obj_t *parent,
                                     const notification_t *notif) {
  // Item container
  lv_obj_t *item = lv_obj_create(parent);
  lv_obj_set_width(item, 200);
  lv_obj_set_height(item, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(item, lv_color_hex(THEME_COLOR_WHITE), 0);
  lv_obj_set_style_radius(item, 10, 0);
  lv_obj_set_style_pad_all(item, 10, 0);
  lv_obj_set_style_border_width(item, 0, 0);
  lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

  // Title
  lv_obj_t *title = lv_label_create(item);
  lv_label_set_text(title, notif->title);
  lv_obj_set_style_text_font(title, THEME_FONT_NORMAL, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(THEME_COLOR_BLACK), 0);
  lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

  // Message
  lv_obj_t *message = lv_label_create(item);
  lv_label_set_text(message, notif->message);
  lv_obj_set_style_text_font(message, THEME_FONT_SMALL, 0);
  lv_obj_set_style_text_color(message, lv_color_hex(THEME_COLOR_GRAY), 0);
  lv_obj_set_width(message, 180);
  lv_label_set_long_mode(message, LV_LABEL_LONG_WRAP);
  lv_obj_align(message, LV_ALIGN_TOP_LEFT, 0, 22);

  // App name
  lv_obj_t *app_name = lv_label_create(item);
  lv_label_set_text(app_name, notif->app_name);
  lv_obj_set_style_text_font(app_name, THEME_FONT_SMALL, 0);
  lv_obj_set_style_text_color(app_name, lv_color_hex(THEME_COLOR_ORANGE), 0);
  lv_obj_align(app_name, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
}

/**
 * @brief Refresh notification list
 */
static void refresh_notification_list(void) {
  if (notification_list == NULL) {
    ESP_LOGE(TAG, "Notification list is NULL");
    return;
  }

  // NOTE: No lvgl_port_lock needed - called from on_show which is already in LVGL context

  // Clear existing list
  lv_obj_clean(notification_list);

  // Get all notifications
  uint32_t count;
  const notification_t *notifications = notification_service_get_all(&count);

  if (count == 0) {
    // Show "No notifications" message
    lv_obj_t *label = lv_label_create(notification_list);
    lv_label_set_text(label, "No notifications");
    lv_obj_set_style_text_font(label, THEME_FONT_NORMAL, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(THEME_COLOR_GRAY), 0);
    lv_obj_center(label);
  } else {
    // Render notifications (index 0 = newest)
    for (uint32_t i = 0; i < count && i < 10; i++) {
      create_notification_item(notification_list, &notifications[i]);
    }
  }

  ESP_LOGI(TAG, "Notification list refreshed: %d items", count);
}

/**
 * @brief Create notifications screen
 */
static lv_obj_t *notifications_screen_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen, lv_color_hex(THEME_COLOR_BG), 0);
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

  // Title
  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(title, "Notifications");
  lv_obj_set_style_text_font(title, THEME_FONT_MEDIUM, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(THEME_COLOR_BLACK), 0);
  lv_obj_set_pos(title, 20, 20);

  // Notification list container (scrollable)
  notification_list = lv_obj_create(screen);
  lv_obj_set_size(notification_list, 220, 190);
  lv_obj_set_pos(notification_list, 10, 50);
  lv_obj_set_style_bg_color(notification_list, lv_color_hex(THEME_COLOR_BG), 0);
  lv_obj_set_style_border_width(notification_list, 0, 0);
  lv_obj_set_style_pad_all(notification_list, 0, 0);
  lv_obj_set_flex_flow(notification_list, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(notification_list, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(notification_list, 10, 0);
  lv_obj_set_scroll_dir(notification_list, LV_DIR_VER);
  
  // Allow gestures to propagate when at scroll boundaries
  lv_obj_add_flag(notification_list, LV_OBJ_FLAG_GESTURE_BUBBLE);

  ESP_LOGI(TAG, "Notifications screen created");
  return screen;
}

static void notifications_screen_on_show(void) {
  ESP_LOGI(TAG, "Notifications screen shown");
  navigation_manager_set_context(NAV_CONTEXT_SYSTEM_SCREEN);

  // Refresh list when shown
  refresh_notification_list();
}

static void notifications_screen_on_hide(void) {
  ESP_LOGI(TAG, "Notifications screen hidden");
}

static void notifications_screen_on_gesture(lv_dir_t direction) {
  navigation_manager_handle_gesture(direction);
}

static const screen_descriptor_t notifications_descriptor = {
    .id = SCREEN_NOTIFICATIONS,
    .name = "Notifications",
    .create = notifications_screen_create,
    .on_show = notifications_screen_on_show,
    .on_hide = notifications_screen_on_hide,
    .on_gesture = notifications_screen_on_gesture,
};

const screen_descriptor_t *notifications_screen_get_descriptor(void) {
  return &notifications_descriptor;
}
