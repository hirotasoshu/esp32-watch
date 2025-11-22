#include "services/time_service.h"
#include "core/event_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "rtc_hal.h"
#include <string.h>

static const char *TAG = "time_service";

static struct tm current_time;
static SemaphoreHandle_t time_mutex = NULL;
static TaskHandle_t time_task_handle = NULL;

/**
 * @brief Convert rtc_time_t to struct tm
 */
static bool rtc_time_to_tm(const rtc_time_t *rtc, struct tm *tm) {
  // Validate RTC input
  if (rtc->month < 1 || rtc->month > 12) {
    ESP_LOGW(TAG, "Invalid RTC month: %d", rtc->month);
    return false;
  }
  if (rtc->day < 1 || rtc->day > 31) {
    ESP_LOGW(TAG, "Invalid RTC day: %d", rtc->day);
    return false;
  }
  if (rtc->hours > 23) {
    ESP_LOGW(TAG, "Invalid RTC hours: %d", rtc->hours);
    return false;
  }
  if (rtc->minutes > 59) {
    ESP_LOGW(TAG, "Invalid RTC minutes: %d", rtc->minutes);
    return false;
  }
  if (rtc->seconds > 59) {
    ESP_LOGW(TAG, "Invalid RTC seconds: %d", rtc->seconds);
    return false;
  }

  tm->tm_sec = rtc->seconds;
  tm->tm_min = rtc->minutes;
  tm->tm_hour = rtc->hours;
  tm->tm_mday = rtc->day;
  tm->tm_mon = rtc->month - 1;    // tm_mon is 0-11
  tm->tm_year = rtc->year - 1900; // tm_year is years since 1900
  tm->tm_wday = rtc->weekday;
  tm->tm_yday = 0;                // Not computed from RTC
  tm->tm_isdst = -1;              // DST information not available
  
  return true;
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
  int last_emitted_second = -1;

  while (1) {
    rtc_time_t rtc_time;
    esp_err_t ret = rtc_hal_get_time(&rtc_time);

    if (ret == ESP_OK) {
      struct tm new_time;
      
      // Convert and validate RTC time
      if (!rtc_time_to_tm(&rtc_time, &new_time)) {
        ESP_LOGW(TAG, "Invalid RTC data, skipping update");
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1000));
        continue;
      }

      // Lock mutex to update current_time
      if (xSemaphoreTake(time_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        memcpy(&current_time, &new_time, sizeof(struct tm));
        int current_second = current_time.tm_sec;
        xSemaphoreGive(time_mutex);

        // Only emit event if second changed
        if (current_second != last_emitted_second) {
          last_emitted_second = current_second;
          
          // Emit event with a copy of the time
          event_manager_emit(EVENT_TIME_UPDATED, &new_time, sizeof(struct tm));
        }
      } else {
        ESP_LOGW(TAG, "Failed to acquire time mutex for update");
      }
    } else {
      ESP_LOGW(TAG, "Failed to read RTC: %s", esp_err_to_name(ret));
    }

    // Wait 1 second
    vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1000));
  }
}

esp_err_t time_service_init(void) {
  // Create mutex for protecting current_time
  time_mutex = xSemaphoreCreateMutex();
  if (time_mutex == NULL) {
    ESP_LOGE(TAG, "Failed to create time mutex");
    return ESP_FAIL;
  }

  memset(&current_time, 0, sizeof(current_time));
  current_time.tm_isdst = -1;

  // Perform initial RTC read to populate current_time
  rtc_time_t rtc_time;
  esp_err_t ret = rtc_hal_get_time(&rtc_time);
  if (ret == ESP_OK) {
    if (rtc_time_to_tm(&rtc_time, &current_time)) {
      ESP_LOGI(TAG, "Initial time from RTC: %04d-%02d-%02d %02d:%02d:%02d",
               current_time.tm_year + 1900, current_time.tm_mon + 1,
               current_time.tm_mday, current_time.tm_hour,
               current_time.tm_min, current_time.tm_sec);
    } else {
      ESP_LOGW(TAG, "Invalid initial RTC data, using defaults");
    }
  } else {
    ESP_LOGW(TAG, "Failed to read initial RTC time: %s", esp_err_to_name(ret));
  }

  // Create time update task
  BaseType_t task_ret =
      xTaskCreate(time_update_task, "time_update", 3072, NULL, 5, &time_task_handle);
  if (task_ret != pdPASS) {
    ESP_LOGE(TAG, "Failed to create time update task");
    vSemaphoreDelete(time_mutex);
    time_mutex = NULL;
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "Time service initialized");
  return ESP_OK;
}

esp_err_t time_service_get_time(struct tm *out_time) {
  if (out_time == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  // Lock mutex to safely copy current_time
  if (xSemaphoreTake(time_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    memcpy(out_time, &current_time, sizeof(struct tm));
    xSemaphoreGive(time_mutex);
    return ESP_OK;
  } else {
    ESP_LOGE(TAG, "Failed to acquire time mutex for get");
    return ESP_ERR_TIMEOUT;
  }
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

  // Lock mutex to safely update current_time
  if (xSemaphoreTake(time_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    memcpy(&current_time, time, sizeof(struct tm));
    xSemaphoreGive(time_mutex);
  } else {
    ESP_LOGW(TAG, "Failed to acquire time mutex for set");
    // RTC is updated but cached time will sync on next task iteration
  }

  ESP_LOGI(TAG, "Time set to %04d-%02d-%02d %02d:%02d:%02d",
           time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour,
           time->tm_min, time->tm_sec);

  return ESP_OK;
}
