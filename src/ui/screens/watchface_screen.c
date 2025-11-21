#include "ui/screens/watchface_screen.h"
#include "core/event_manager.h"
#include "core/navigation_manager.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "services/battery_service.h"
#include "services/steps_service.h"
#include "services/time_service.h"
#include "ui/theme.h"
#include <time.h>

static const char *TAG = "watchface_screen";

// UI elements
static lv_obj_t *lbl_time = NULL;
static lv_obj_t *lbl_date = NULL;
static lv_obj_t *battery_dots[5];
static lv_obj_t *progress_dots[15];
static lv_obj_t *runner_icon = NULL;

// Current values from services
static uint8_t current_battery = 0;
static uint32_t current_steps = 0;

/**
 * @brief Create a dot
 */
static lv_obj_t *create_dot(lv_obj_t *parent, int x, int y, int size,
                            lv_color_t color) {
  lv_obj_t *dot = lv_obj_create(parent);
  lv_obj_set_size(dot, size, size);
  lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(dot, color, 0);
  lv_obj_set_style_border_width(dot, 0, 0);
  lv_obj_set_pos(dot, x, y);
  lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  return dot;
}

/**
 * @brief Create a bar (rectangle)
 */
static lv_obj_t *create_bar(lv_obj_t *parent, int x, int y, int w, int h,
                            lv_color_t color) {
  lv_obj_t *bar = lv_obj_create(parent);
  lv_obj_set_size(bar, w, h);
  lv_obj_set_style_radius(bar, 5, 0);
  lv_obj_set_style_bg_color(bar, color, 0);
  lv_obj_set_style_border_width(bar, 0, 0);
  lv_obj_set_pos(bar, x, y);
  lv_obj_clear_flag(bar, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  return bar;
}

/**
 * @brief Update time display
 */
static void update_time_display(const struct tm *time) {
  if (lbl_time == NULL || lbl_date == NULL) {
    return;
  }

  char time_str[16];
  char date_str[32];

  // Format time: "14:35"
  strftime(time_str, sizeof(time_str), "%H:%M", time);
  lv_label_set_text(lbl_time, time_str);

  // Format date: "Tue 19.12"
  strftime(date_str, sizeof(date_str), "%a %d.%m", time);
  lv_label_set_text(lbl_date, date_str);
}

/**
 * @brief Update battery display (simulated for now)
 */
static void update_battery_display(void) {
  int filled = (current_battery * 5) / 100;

  for (int i = 0; i < 5; i++) {
    lv_obj_set_style_bg_color(battery_dots[i],
                              (i >= 5 - filled)
                                  ? lv_color_hex(THEME_COLOR_ORANGE)
                                  : lv_color_hex(THEME_COLOR_WHITE),
                              0);
  }
}

/**
 * @brief Update progress display (simulated steps for now)
 */
static void update_progress_display(void) {
  int morse_x_start = 20;
  int battery_x = 218;
  int dot_start_x = morse_x_start;
  int dot_end_x = battery_x;
  int num_dots = 15;
  int total_width = dot_end_x - dot_start_x;
  int dot_spacing = total_width / (num_dots - 1);

  // Calculate progress based on steps (0-goal = 0-15 dots)
  uint32_t goal = steps_service_get_goal();
  int progress = (current_steps * num_dots) / goal;
  if (progress > num_dots) progress = num_dots;

  for (int i = 0; i < num_dots; i++) {
    if (i < progress) {
      lv_obj_set_style_bg_color(progress_dots[i],
                                lv_color_hex(THEME_COLOR_ORANGE), 0);
    } else {
      lv_obj_set_style_bg_color(progress_dots[i],
                                lv_color_hex(THEME_COLOR_WHITE), 0);
    }
  }

  int runner_x = dot_start_x + progress * dot_spacing - 2;
  lv_obj_set_pos(runner_icon, runner_x, 218);
}

/**
 * @brief Event callback for time updates
 */
static void time_event_callback(const event_t *event, void *user_data) {
  if (event->type == EVENT_TIME_UPDATED && event->data != NULL) {
    struct tm *time = (struct tm *)event->data;
    
    lvgl_port_lock(-1);
    update_time_display(time);
    lvgl_port_unlock();
  }
}

static void battery_event_callback(const event_t *event, void *user_data) {
  if (event->type == EVENT_BATTERY_UPDATED && event->data != NULL) {
    current_battery = *(uint8_t *)event->data;
    
    lvgl_port_lock(-1);
    update_battery_display();
    lvgl_port_unlock();
  }
}

static void steps_event_callback(const event_t *event, void *user_data) {
  if (event->type == EVENT_STEPS_UPDATED && event->data != NULL) {
    current_steps = *(uint32_t *)event->data;
    
    lvgl_port_lock(-1);
    update_progress_display();
    lvgl_port_unlock();
  }
}

/**
 * @brief Test button callback
 */
static void test_button_clicked(lv_event_t *e) {
  ESP_LOGI("TOUCH_TEST", "=== BUTTON CLICKED! TOUCH WORKS! ===");
}

/**
 * @brief Create watchface screen
 */
static lv_obj_t *watchface_screen_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen, lv_color_hex(THEME_COLOR_BG), 0);
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

  // Top icon (top-left corner)
  lv_obj_t *top_icon = lv_obj_create(screen);
  lv_obj_set_size(top_icon, 8, 8);
  lv_obj_set_pos(top_icon, 12, 12);
  lv_obj_set_style_bg_color(top_icon, lv_color_hex(THEME_COLOR_WHITE), 0);
  lv_obj_set_style_border_width(top_icon, 0, 0);
  lv_obj_set_style_radius(top_icon, 2, 0);
  lv_obj_clear_flag(top_icon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

  // Notification dot (orange, next to top icon)
  lv_obj_t *notif_dot = lv_obj_create(screen);
  lv_obj_set_size(notif_dot, 4, 4);
  lv_obj_set_pos(notif_dot, 23, 12);
  lv_obj_set_style_bg_color(notif_dot, lv_color_hex(THEME_COLOR_ORANGE), 0);
  lv_obj_set_style_border_width(notif_dot, 0, 0);
  lv_obj_set_style_radius(notif_dot, LV_RADIUS_CIRCLE, 0);
  lv_obj_clear_flag(notif_dot, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

  // Time label (large, centered-ish)
  lbl_time = lv_label_create(screen);
  lv_label_set_text(lbl_time, "12:00");
  lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_42, 0);
  lv_obj_set_style_text_color(lbl_time, lv_color_hex(THEME_COLOR_BLACK), 0);
  lv_obj_set_pos(lbl_time, 20, 70);

  // Date label (below time)
  lbl_date = lv_label_create(screen);
  lv_label_set_text(lbl_date, "Tue 19.12");
  lv_obj_set_style_text_font(lbl_date, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(lbl_date, lv_color_hex(THEME_COLOR_BLACK), 0);
  lv_obj_set_pos(lbl_date, 20, 118);

  // Battery display (vertical dots on the right)
  int battery_x = 218;
  int morse_line2_y = 155 + 16;
  int battery_dot_spacing = 14;
  int battery_y_end = morse_line2_y;
  int battery_y_start = battery_y_end - 4 * battery_dot_spacing;

  // Charge icon (above battery dots)
  lv_obj_t *charge_icon = lv_label_create(screen);
  lv_label_set_text(charge_icon, LV_SYMBOL_CHARGE);
  lv_obj_set_style_text_font(charge_icon, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(charge_icon, lv_color_hex(THEME_COLOR_WHITE), 0);
  lv_obj_set_pos(charge_icon, battery_x, battery_y_start - 20);

  // Battery dots (5 vertical dots)
  for (int i = 0; i < 5; i++) {
    battery_dots[i] =
        create_dot(screen, battery_x, battery_y_start + i * battery_dot_spacing,
                   8, lv_color_hex(THEME_COLOR_WHITE));
  }

  // MORSE code visualization
  int morse_y = 155;
  int morse_x_start = 20;

  // Line 1: - · · · ·
  create_bar(screen, morse_x_start, morse_y, 25, 10,
             lv_color_hex(THEME_COLOR_BLACK));
  create_dot(screen, morse_x_start + 33, morse_y, 8,
             lv_color_hex(THEME_COLOR_ORANGE));
  create_dot(screen, morse_x_start + 48, morse_y, 8,
             lv_color_hex(THEME_COLOR_ORANGE));
  create_dot(screen, morse_x_start + 63, morse_y, 8,
             lv_color_hex(THEME_COLOR_ORANGE));
  create_dot(screen, morse_x_start + 78, morse_y, 8,
             lv_color_hex(THEME_COLOR_ORANGE));

  // Line 2: · · · - -
  morse_y += 16;
  create_dot(screen, morse_x_start, morse_y, 8,
             lv_color_hex(THEME_COLOR_ORANGE));
  create_dot(screen, morse_x_start + 15, morse_y, 8,
             lv_color_hex(THEME_COLOR_ORANGE));
  create_dot(screen, morse_x_start + 30, morse_y, 8,
             lv_color_hex(THEME_COLOR_ORANGE));
  create_bar(screen, morse_x_start + 45, morse_y - 1, 25, 10,
             lv_color_hex(THEME_COLOR_BLACK));
  create_bar(screen, morse_x_start + 78, morse_y - 1, 25, 10,
             lv_color_hex(THEME_COLOR_BLACK));

  // Line 3: - - - - -
  morse_y += 16;
  create_bar(screen, morse_x_start, morse_y, 25, 10,
             lv_color_hex(THEME_COLOR_BLACK));
  create_bar(screen, morse_x_start + 33, morse_y, 25, 10,
             lv_color_hex(THEME_COLOR_BLACK));
  create_bar(screen, morse_x_start + 66, morse_y, 25, 10,
             lv_color_hex(THEME_COLOR_BLACK));
  create_bar(screen, morse_x_start + 99, morse_y, 25, 10,
             lv_color_hex(THEME_COLOR_BLACK));
  create_bar(screen, morse_x_start + 132, morse_y, 25, 10,
             lv_color_hex(THEME_COLOR_BLACK));

  // Progress dots (15 dots at the bottom, representing steps/goal)
  int progress_y = 220;
  int dot_start_x = morse_x_start;
  int dot_end_x = battery_x;
  int total_width = dot_end_x - dot_start_x;
  int num_dots = 15;
  int dot_spacing = total_width / (num_dots - 1);

  for (int i = 0; i < num_dots; i++) {
    progress_dots[i] =
        create_dot(screen, dot_start_x + i * dot_spacing, progress_y, 6,
                   lv_color_hex(THEME_COLOR_WHITE));
  }

  // Runner icon (moves along the progress dots)
  runner_icon = lv_obj_create(screen);
  lv_obj_set_size(runner_icon, 10, 10);
  lv_obj_set_pos(runner_icon, dot_start_x + 3 * dot_spacing - 2,
                 progress_y - 2);
  lv_obj_set_style_bg_color(runner_icon, lv_color_hex(THEME_COLOR_BLACK), 0);
  lv_obj_set_style_border_width(runner_icon, 0, 0);
  lv_obj_set_style_radius(runner_icon, 2, 0);
  lv_obj_clear_flag(runner_icon,
                    LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

  ESP_LOGI(TAG, "Watchface screen created with full graphics");
  return screen;
}

static void watchface_screen_on_show(void) {
  ESP_LOGI(TAG, "Watchface screen shown");
  navigation_manager_set_context(NAV_CONTEXT_WATCHFACE);

  // Get FRESH values from services
  struct tm time;
  time_service_get_time(&time);
  current_battery = battery_service_get_level();
  current_steps = steps_service_get_count();
  
  // Update UI with current values
  update_time_display(&time);
  update_battery_display();
  update_progress_display();

  // Subscribe to events (for updates while screen is shown)
  event_manager_subscribe(EVENT_TIME_UPDATED, time_event_callback, NULL);
  event_manager_subscribe(EVENT_BATTERY_UPDATED, battery_event_callback, NULL);
  event_manager_subscribe(EVENT_STEPS_UPDATED, steps_event_callback, NULL);
}

static void watchface_screen_on_hide(void) {
  ESP_LOGI(TAG, "Watchface screen hidden");

  // Unsubscribe from all events
  event_manager_unsubscribe(EVENT_TIME_UPDATED, time_event_callback);
  event_manager_unsubscribe(EVENT_BATTERY_UPDATED, battery_event_callback);
  event_manager_unsubscribe(EVENT_STEPS_UPDATED, steps_event_callback);
}

static void watchface_screen_on_gesture(lv_dir_t direction) {
  navigation_manager_handle_gesture(direction);
}

static const screen_descriptor_t watchface_descriptor = {
    .id = SCREEN_WATCHFACE,
    .name = "Watchface",
    .create = watchface_screen_create,
    .on_show = watchface_screen_on_show,
    .on_hide = watchface_screen_on_hide,
    .on_gesture = watchface_screen_on_gesture,
};

const screen_descriptor_t *watchface_screen_get_descriptor(void) {
  return &watchface_descriptor;
}
