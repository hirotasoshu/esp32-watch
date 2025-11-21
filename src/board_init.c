#include "board_init.h"
#include "cst816s_driver.h"
#include "display_hal.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/spi_common.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "pcf85063_driver.h"
#include "rtc_hal.h"
#include "st7789_driver.h"
#include "touch_hal.h"

static const char *TAG = "board_init";

static i2c_master_bus_handle_t i2c_bus = NULL;
static lv_display_t *lvgl_display = NULL;
static lv_indev_t *touch_indev = NULL; // Store touch indev globally

/**
 * @brief LVGL touch read callback
 */
static void board_touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
  esp_lcd_touch_handle_t tp = (esp_lcd_touch_handle_t)lv_indev_get_user_data(indev);
  
  uint16_t x, y;
  uint8_t count = 0;
  
  // Get touch semaphore (signaled by interrupt)
  SemaphoreHandle_t touch_sem = touch_hal_get_semaphore();
  
  // Read touch data ONLY if interrupt occurred (don't poll continuously)
  if (touch_sem != NULL && xSemaphoreTake(touch_sem, 0) == pdTRUE) {
    esp_lcd_touch_read_data(tp);
  }
  
  bool pressed = esp_lcd_touch_get_coordinates(tp, &x, &y, NULL, &count, 1);
  
  if (pressed && count > 0) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

esp_err_t board_init(void) {
  esp_err_t ret;

  ESP_LOGI(TAG, "=== Board Initialization Started ===");

  // Step 1: Initialize I2C Bus
  ESP_LOGI(TAG, "[1/6] Initializing I2C bus...");
  const i2c_master_bus_config_t i2c_cfg = {
      .i2c_port = I2C_MASTER_NUM,
      .sda_io_num = I2C_MASTER_SDA,
      .scl_io_num = I2C_MASTER_SCL,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };

  ret = i2c_new_master_bus(&i2c_cfg, &i2c_bus);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init I2C: %s", esp_err_to_name(ret));
    return ret;
  }
  ESP_LOGI(TAG, "I2C initialized (port=%d)", I2C_MASTER_NUM);

  // Step 2: Initialize LVGL Port FIRST (before display driver!)
  ESP_LOGI(TAG, "[2/6] Initializing LVGL port...");
  const lvgl_port_cfg_t lvgl_cfg = {
      .task_priority = 4,
      .task_stack = 8192, // Increased from 4096 - more code = more stack needed
      .task_affinity = -1,
      .task_max_sleep_ms = 500,
      .timer_period_ms = 5,
  };

  ret = lvgl_port_init(&lvgl_cfg);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init LVGL port");
    return ret;
  }
  ESP_LOGI(TAG, "LVGL port initialized");

  // Step 3: Initialize Display (ST7789)
  ESP_LOGI(TAG, "[3/6] Initializing display...");
  ret = display_hal_register(st7789_get_interface());
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register display driver");
    return ret;
  }

  const st7789_config_t display_config = {
      .h_res = ST7789_H_RES,
      .v_res = ST7789_V_RES,
      .pin_sclk = ST7789_PIN_SCLK,
      .pin_mosi = ST7789_PIN_MOSI,
      .pin_rst = ST7789_PIN_RST,
      .pin_dc = ST7789_PIN_DC,
      .pin_cs = ST7789_PIN_CS,
      .pin_bl = ST7789_PIN_BL,
      .spi_host = SPI2_HOST,
  };

  ret = display_hal_init(&display_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init display");
    return ret;
  }

  lvgl_display = display_hal_get_lvgl_display();
  if (lvgl_display == NULL) {
    ESP_LOGE(TAG, "Failed to get LVGL display");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "Display initialized: %s", display_hal_get_driver_name());

  // Step 4: Initialize Touch (CST816S)
  ESP_LOGI(TAG, "[4/6] Initializing touch...");
  ret = touch_hal_register(cst816s_get_interface());
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register touch driver");
    return ret;
  }

  const cst816s_config_t touch_config = {
      .i2c_bus = i2c_bus,
      .pin_rst = CST816S_PIN_RST,
      .pin_int = CST816S_PIN_INT,
      .h_res = CST816S_H_RES,
      .v_res = CST816S_V_RES,
  };

  ret = touch_hal_init(&touch_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init touch");
    return ret;
  }
  ESP_LOGI(TAG, "Touch initialized: %s", touch_hal_get_driver_name());

  // Step 5: Create LVGL Input Device manually (for gesture support)
  ESP_LOGI(TAG, "[5/6] Creating LVGL touch input device...");
  lv_indev_t *indev = lv_indev_create();
  if (indev == NULL) {
    ESP_LOGE(TAG, "Failed to create LVGL input device");
    return ESP_FAIL;
  }
  
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, board_touch_read_cb);
  lv_indev_set_user_data(indev, touch_hal_get_handle());
  lv_indev_set_display(indev, lvgl_display);
  
  // Store globally
  touch_indev = indev;
  
  ESP_LOGI(TAG, "LVGL touch input device created (indev=%p)", indev);

  // Step 6: Initialize RTC (PCF85063)
  ESP_LOGI(TAG, "[6/6] Initializing RTC...");
  ret = rtc_hal_register(pcf85063_get_interface());
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register RTC driver");
    return ret;
  }

  const pcf85063_config_t rtc_config = {
      .i2c_bus = i2c_bus,
#ifdef PCF85063_SET_COMPILE_TIME
      .set_compile_time = (PCF85063_SET_COMPILE_TIME != 0),
#else
      .set_compile_time = false,
#endif
  };

  ret = rtc_hal_init(&rtc_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init RTC");
    return ret;
  }
  ESP_LOGI(TAG, "RTC initialized: %s", rtc_hal_get_driver_name());

  ESP_LOGI(TAG, "=== Board Initialization Complete ===");
  return ESP_OK;
}

lv_display_t *board_get_display(void) { return lvgl_display; }

i2c_master_bus_handle_t board_get_i2c_bus(void) { return i2c_bus; }

lv_indev_t *board_get_touch_indev(void) {
  return touch_indev; // Return stored touch indev
}
