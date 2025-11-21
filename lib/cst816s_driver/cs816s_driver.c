#include "cst816s_driver.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch_cst816s.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "CST816S";

static esp_lcd_touch_handle_t touch_handle = NULL;
static SemaphoreHandle_t touch_semaphore = NULL;

// Store config locally
static cst816s_config_t driver_config;

static void cst816s_touch_callback(esp_lcd_touch_handle_t tp) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(touch_semaphore, &xHigherPriorityTaskWoken);

  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}

static esp_err_t cst816s_init_impl(const void *config) {
  if (config == NULL) {
    ESP_LOGE(TAG, "Config is NULL");
    return ESP_ERR_INVALID_ARG;
  }

  // Cast and store config
  const cst816s_config_t *cfg = (const cst816s_config_t *)config;
  driver_config = *cfg;

  if (driver_config.i2c_bus == NULL) {
    ESP_LOGE(TAG, "I2C bus not provided");
    return ESP_ERR_INVALID_ARG;
  }

  esp_err_t ret;

  // Create binary semaphore for ISR
  touch_semaphore = xSemaphoreCreateBinary();
  if (touch_semaphore == NULL) {
    ESP_LOGE(TAG, "Failed to create touch semaphore");
    return ESP_ERR_NO_MEM;
  }

  // Touch Panel IO config
  esp_lcd_panel_io_handle_t tp_io;
  esp_lcd_panel_io_i2c_config_t tp_io_cfg =
      ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
  tp_io_cfg.scl_speed_hz = 100000; // 100 kHz

  ret = esp_lcd_new_panel_io_i2c_v2(driver_config.i2c_bus, &tp_io_cfg, &tp_io);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create touch panel IO");
    vSemaphoreDelete(touch_semaphore);
    return ret;
  }

  // Touch controller config
  esp_lcd_touch_config_t tp_cfg = {
      .x_max = driver_config.h_res,
      .y_max = driver_config.v_res,
      .rst_gpio_num = driver_config.pin_rst,
      .int_gpio_num = driver_config.pin_int,
      .levels =
          {
              .reset = 0,
              .interrupt = 0,
          },
      .flags =
          {
              .swap_xy = 0,
              .mirror_x = 0,
              .mirror_y = 0,
          },
      .interrupt_callback = cst816s_touch_callback,
  };

  ret = esp_lcd_touch_new_i2c_cst816s(tp_io, &tp_cfg, &touch_handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create CST816S touch");
    vSemaphoreDelete(touch_semaphore);
    return ret;
  }

  ESP_LOGI(TAG, "CST816S touch initialized (%dx%d)", driver_config.h_res, driver_config.v_res);
  return ESP_OK;
}

static esp_err_t cst816s_deinit_impl(void) {
  if (touch_handle != NULL) {
    esp_lcd_touch_del(touch_handle);
    touch_handle = NULL;
  }

  if (touch_semaphore != NULL) {
    vSemaphoreDelete(touch_semaphore);
    touch_semaphore = NULL;
  }

  ESP_LOGI(TAG, "CST816S deinitialized");
  return ESP_OK;
}

static esp_lcd_touch_handle_t cst816s_get_handle_impl(void) {
  return touch_handle;
}

static SemaphoreHandle_t cst816s_get_semaphore_impl(void) {
  return touch_semaphore;
}

static const touch_hal_interface_t cst816s_interface = {
    .init = cst816s_init_impl,
    .deinit = cst816s_deinit_impl,
    .get_handle = cst816s_get_handle_impl,
    .get_semaphore = cst816s_get_semaphore_impl,
    .name = "CST816S"};

const touch_hal_interface_t *cst816s_get_interface(void) {
  return &cst816s_interface;
}
