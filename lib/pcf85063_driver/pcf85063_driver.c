#include "pcf85063_driver.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "PCF85063";

#define PCF85063_ADDR 0x51
#define PCF85063_REG_CONTROL_1 0x00
#define PCF85063_REG_TIME_START 0x04
#define I2C_TIMEOUT_MS 1000

static i2c_master_dev_handle_t pcf85063_handle = NULL;
static pcf85063_config_t driver_config;

static uint8_t bcd_to_dec(uint8_t val) {
  return (val >> 4) * 10 + (val & 0x0F);
}

static uint8_t dec_to_bcd(uint8_t val) {
  return ((val / 10) << 4) | (val % 10);
}

static esp_err_t pcf85063_read_reg(uint8_t reg_addr, uint8_t *data, size_t len) {
  if (pcf85063_handle == NULL) {
    return ESP_ERR_INVALID_STATE;
  }
  return i2c_master_transmit_receive(pcf85063_handle, &reg_addr, 1, data, len, I2C_TIMEOUT_MS);
}

static esp_err_t pcf85063_write_reg(uint8_t reg_addr, uint8_t *data, size_t len) {
  if (pcf85063_handle == NULL) {
    return ESP_ERR_INVALID_STATE;
  }

  uint8_t *write_buf = malloc(len + 1);
  if (write_buf == NULL) {
    return ESP_ERR_NO_MEM;
  }

  write_buf[0] = reg_addr;
  memcpy(&write_buf[1], data, len);

  esp_err_t ret = i2c_master_transmit(pcf85063_handle, write_buf, len + 1, I2C_TIMEOUT_MS);
  free(write_buf);
  return ret;
}

static void pcf85063_get_compile_time(rtc_time_t *time) {
  const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  char month_str[4];
  int day, year, hour, min, sec;

  sscanf(__DATE__, "%s %d %d", month_str, &day, &year);
  sscanf(__TIME__, "%d:%d:%d", &hour, &min, &sec);

  time->month = 1;
  for (int i = 0; i < 12; i++) {
    if (strcmp(month_str, months[i]) == 0) {
      time->month = i + 1;
      break;
    }
  }

  time->day = day;
  time->year = year - 2000;
  time->hours = hour;
  time->minutes = min;
  time->seconds = sec;
  time->weekday = 0; // Unknown from compile time
}

static esp_err_t pcf85063_init_impl(const void *config) {
  if (config == NULL) {
    ESP_LOGE(TAG, "Config is NULL");
    return ESP_ERR_INVALID_ARG;
  }

  const pcf85063_config_t *cfg = (const pcf85063_config_t *)config;
  driver_config = *cfg;

  if (driver_config.i2c_bus == NULL) {
    ESP_LOGE(TAG, "I2C bus not provided");
    return ESP_ERR_INVALID_ARG;
  }

  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = PCF85063_ADDR,
      .scl_speed_hz = 100000,
  };

  esp_err_t ret = i2c_master_bus_add_device(driver_config.i2c_bus, &dev_cfg, &pcf85063_handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add I2C device");
    return ret;
  }

  uint8_t ctrl = 0x00;
  ret = pcf85063_write_reg(PCF85063_REG_CONTROL_1, &ctrl, 1);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure PCF85063");
    return ret;
  }

  if (driver_config.set_compile_time) {
    rtc_time_t compile_time;
    pcf85063_get_compile_time(&compile_time);
    
    uint8_t time_regs[7];
    time_regs[0] = dec_to_bcd(compile_time.seconds);
    time_regs[1] = dec_to_bcd(compile_time.minutes);
    time_regs[2] = dec_to_bcd(compile_time.hours);
    time_regs[3] = dec_to_bcd(compile_time.day);
    time_regs[4] = 0; // weekday
    time_regs[5] = dec_to_bcd(compile_time.month);
    time_regs[6] = dec_to_bcd(compile_time.year);

    ret = pcf85063_write_reg(PCF85063_REG_TIME_START, time_regs, 7);
    if (ret == ESP_OK) {
      ESP_LOGI(TAG, "Set time to compile time: %04d-%02d-%02d %02d:%02d:%02d",
               compile_time.year + 2000, compile_time.month, compile_time.day,
               compile_time.hours, compile_time.minutes, compile_time.seconds);
    }
  }

  ESP_LOGI(TAG, "PCF85063 RTC initialized");
  return ESP_OK;
}

static esp_err_t pcf85063_get_time_impl(rtc_time_t *time) {
  if (time == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t time_regs[7];
  esp_err_t ret = pcf85063_read_reg(PCF85063_REG_TIME_START, time_regs, 7);
  if (ret != ESP_OK) {
    return ret;
  }

  time->seconds = bcd_to_dec(time_regs[0] & 0x7F);
  time->minutes = bcd_to_dec(time_regs[1] & 0x7F);
  time->hours = bcd_to_dec(time_regs[2] & 0x3F);
  time->day = bcd_to_dec(time_regs[3] & 0x3F);
  time->weekday = time_regs[4] & 0x07;
  time->month = bcd_to_dec(time_regs[5] & 0x1F);
  time->year = bcd_to_dec(time_regs[6]);

  return ESP_OK;
}

static esp_err_t pcf85063_set_time_impl(const rtc_time_t *time) {
  if (time == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t time_regs[7];
  time_regs[0] = dec_to_bcd(time->seconds);
  time_regs[1] = dec_to_bcd(time->minutes);
  time_regs[2] = dec_to_bcd(time->hours);
  time_regs[3] = dec_to_bcd(time->day);
  time_regs[4] = time->weekday & 0x07;
  time_regs[5] = dec_to_bcd(time->month);
  time_regs[6] = dec_to_bcd(time->year);

  return pcf85063_write_reg(PCF85063_REG_TIME_START, time_regs, 7);
}

static esp_err_t pcf85063_deinit_impl(void) {
  if (pcf85063_handle != NULL) {
    i2c_master_bus_rm_device(pcf85063_handle);
    pcf85063_handle = NULL;
  }
  ESP_LOGI(TAG, "PCF85063 deinitialized");
  return ESP_OK;
}

static const rtc_hal_interface_t pcf85063_interface = {
    .init = pcf85063_init_impl,
    .get_time = pcf85063_get_time_impl,
    .set_time = pcf85063_set_time_impl,
    .deinit = pcf85063_deinit_impl,
    .name = "PCF85063"};

const rtc_hal_interface_t *pcf85063_get_interface(void) {
  return &pcf85063_interface;
}
