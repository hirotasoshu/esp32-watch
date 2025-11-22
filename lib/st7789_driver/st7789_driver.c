#include "st7789_driver.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"

static const char *TAG = "ST7789";

// LCD settings (constants)
#define LCD_PIXEL_CLK_HZ (40 * 1000 * 1000)
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_RGB
#define LCD_BITS_PER_PIXEL 16
#define LCD_DRAW_BUFF_DOUBLE 1
#define LCD_DRAW_BUFF_HEIGHT 50

// PWM settings for backlight
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY          (5000)

static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;
static lv_display_t *lvgl_disp = NULL;
static bool pwm_initialized = false;

// Store config locally
static st7789_config_t driver_config;

static esp_err_t st7789_init_impl(const void *config) {
  if (config == NULL) {
    ESP_LOGE(TAG, "Config is NULL");
    return ESP_ERR_INVALID_ARG;
  }

  // Cast and store config
  const st7789_config_t *cfg = (const st7789_config_t *)config;
  driver_config = *cfg;

  esp_err_t ret;

  // Backlight GPIO
  gpio_config_t bk_gpio_config = {
      .mode = GPIO_MODE_OUTPUT,
      .pin_bit_mask = 1ULL << driver_config.pin_bl
  };
  ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

  // SPI Bus
  spi_bus_config_t buscfg = {
      .sclk_io_num = driver_config.pin_sclk,
      .mosi_io_num = driver_config.pin_mosi,
      .miso_io_num = GPIO_NUM_NC,
      .quadwp_io_num = GPIO_NUM_NC,
      .quadhd_io_num = GPIO_NUM_NC,
      .max_transfer_sz = driver_config.h_res * LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t),
  };
  ret = spi_bus_initialize(driver_config.spi_host, &buscfg, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "SPI bus init failed");
    return ret;
  }

  // Panel IO
  esp_lcd_panel_io_spi_config_t io_config = {
      .dc_gpio_num = driver_config.pin_dc,
      .cs_gpio_num = driver_config.pin_cs,
      .pclk_hz = LCD_PIXEL_CLK_HZ,
      .lcd_cmd_bits = LCD_CMD_BITS,
      .lcd_param_bits = LCD_PARAM_BITS,
      .spi_mode = 0,
      .trans_queue_depth = 10,
  };
  ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)driver_config.spi_host,
                                 &io_config, &lcd_io);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Panel IO init failed");
    return ret;
  }

  // Panel
  esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = driver_config.pin_rst,
      .color_space = LCD_COLOR_SPACE,
      .bits_per_pixel = LCD_BITS_PER_PIXEL,
  };
  ret = esp_lcd_new_panel_st7789(lcd_io, &panel_config, &lcd_panel);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Panel init failed");
    return ret;
  }

  esp_lcd_panel_reset(lcd_panel);
  esp_lcd_panel_init(lcd_panel);
  esp_lcd_panel_invert_color(lcd_panel, true);
  esp_lcd_panel_mirror(lcd_panel, true, true);
  esp_lcd_panel_disp_on_off(lcd_panel, true);

  // Backlight on
  gpio_set_level(driver_config.pin_bl, 1);

  esp_lcd_panel_set_gap(lcd_panel, 0, 20);

  // LVGL display
  const lvgl_port_display_cfg_t disp_cfg = {
      .io_handle = lcd_io,
      .panel_handle = lcd_panel,
      .buffer_size = driver_config.h_res * LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t),
      .double_buffer = LCD_DRAW_BUFF_DOUBLE,
      .hres = driver_config.h_res,
      .vres = driver_config.v_res,
      .monochrome = false,
      .rotation =
          {
              .swap_xy = false,
              .mirror_x = false,
              .mirror_y = false,
          },
      .flags = {.buff_dma = true},
  };

  lvgl_disp = lvgl_port_add_disp(&disp_cfg);
  if (lvgl_disp == NULL) {
    ESP_LOGE(TAG, "Failed to add LVGL display");
    return ESP_FAIL;
  }

  // Initialize PWM for backlight
  ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_MODE,
    .timer_num        = LEDC_TIMER,
    .duty_resolution  = LEDC_DUTY_RES,
    .freq_hz          = LEDC_FREQUENCY,
    .clk_cfg          = LEDC_AUTO_CLK
  };
  ret = ledc_timer_config(&ledc_timer);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure LEDC timer");
    return ret;
  }

  ledc_channel_config_t ledc_channel = {
    .speed_mode     = LEDC_MODE,
    .channel        = LEDC_CHANNEL,
    .timer_sel      = LEDC_TIMER,
    .intr_type      = LEDC_INTR_DISABLE,
    .gpio_num       = driver_config.pin_bl,
    .duty           = 255, // Start at full brightness
    .hpoint         = 0
  };
  ret = ledc_channel_config(&ledc_channel);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure LEDC channel");
    return ret;
  }
  
  pwm_initialized = true;

  ESP_LOGI(TAG, "ST7789 display initialized (%dx%d)", driver_config.h_res, driver_config.v_res);
  return ESP_OK;
}

static esp_err_t st7789_set_brightness_impl(uint8_t level) {
  if (!pwm_initialized) {
    ESP_LOGW(TAG, "PWM not initialized");
    return ESP_ERR_INVALID_STATE;
  }
  
  esp_err_t ret = ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, level);
  if (ret != ESP_OK) return ret;
  
  return ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

static esp_err_t st7789_sleep_impl(void) {
  if (pwm_initialized) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
  }
  return esp_lcd_panel_disp_on_off(lcd_panel, false);
}

static esp_err_t st7789_wakeup_impl(void) {
  esp_err_t ret = esp_lcd_panel_disp_on_off(lcd_panel, true);
  if (pwm_initialized) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 255);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
  }
  return ret;
}

static lv_display_t *st7789_get_lvgl_display_impl(void) { return lvgl_disp; }

static const display_hal_interface_t st7789_interface = {
    .init = st7789_init_impl,
    .set_brightness = st7789_set_brightness_impl,
    .sleep = st7789_sleep_impl,
    .wakeup = st7789_wakeup_impl,
    .get_lvgl_display = st7789_get_lvgl_display_impl,
    .name = "ST7789"};

const display_hal_interface_t *st7789_get_interface(void) {
  return &st7789_interface;
}
