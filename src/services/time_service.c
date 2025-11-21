#include "services/time_service.h"
#include "core/event_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rtc_hal.h"
#include <string.h>

static const char *TAG = "time_service";

static struct tm current_time;

/**
 * @brief Convert rtc_time_t to struct tm
 */
static void rtc_time_to_tm(const rtc_time_t *rtc, struct tm *tm) {
  tm->tm_sec = rtc->seconds;
  tm->tm_min = rtc->minutes;
  tm->tm_hour = rtc->hours;
  tm->tm_mday = rtc->day;
  tm->tm_mon = rtc->month - 1;    // tm_mon is 0-11
  tm->tm_year = rtc->year - 1900; // tm_year is years since 1900
  tm->tm_wday = rtc->weekday;
}

/**
 * @brief Convert struct tm to rtc_time_t
 */
static void tm_to_rtc_time(const struct tm *tm, rtc_time_t *rtc) {
  rtc->seconds = tm->tm_sec;
  rtc->minutes = tm->tm_min;
  rtc->hours = tm->tm_hour;
  rtc->day = tm->tm_mday;
  rtc->month = tm->tm_mon + 1;    // tm_mon is 0-11
  rtc->year = tm->tm_year + 1900; // tm_year is years since 1900
  rtc->weekday = tm->tm_wday;
}

/**
 * @brief Time update task - reads RTC every second
 */
static void time_update_task(void *arg) {
  TickType_t last_wake = xTaskGetTickCount();

  while (1) {
    rtc_time_t rtc_time;
    esp_err_t ret = rtc_hal_get_time(&rtc_time);

    if (ret == ESP_OK) {
      // Convert to struct tm
      rtc_time_to_tm(&rtc_time, &current_time);

      // Emit event (pass pointer to current_time)
      event_manager_emit(EVENT_TIME_UPDATED, &current_time, sizeof(struct tm));
    } else {
      ESP_LOGW(TAG, "Failed to read RTC: %s", esp_err_to_name(ret));
    }

    // Wait 1 second
    vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1000));
  }
}

esp_err_t time_service_init(void) {
  memset(&current_time, 0, sizeof(current_time));

  // Create time update task
  BaseType_t ret =
      xTaskCreate(time_update_task, "time_update", 3072, NULL, 5, NULL);
  if (ret != pdPASS) {
    ESP_LOGE(TAG, "Failed to create time update task");
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "Time service initialized");
  return ESP_OK;
}

esp_err_t time_service_get_time(struct tm *out_time) {
  if (out_time == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  memcpy(out_time, &current_time, sizeof(struct tm));
  return ESP_OK;
}

esp_err_t time_service_set_time(const struct tm *time) {
  if (time == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  rtc_time_t rtc_time;
  tm_to_rtc_time(time, &rtc_time);

  esp_err_t ret = rtc_hal_set_time(&rtc_time);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set RTC time: %s", esp_err_to_name(ret));
    return ret;
  }

  memcpy(&current_time, time, sizeof(struct tm));

  ESP_LOGI(TAG, "Time set to %04d-%02d-%02d %02d:%02d:%02d",
           time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour,
           time->tm_min, time->tm_sec);

  return ESP_OK;
}
