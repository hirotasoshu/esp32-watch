#ifndef ST7789_DRIVER_H
#define ST7789_DRIVER_H

#include "display_hal.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"

/**
 * @brief ST7789 configuration structure
 */
typedef struct {
  uint16_t h_res;
  uint16_t v_res;
  gpio_num_t pin_sclk;
  gpio_num_t pin_mosi;
  gpio_num_t pin_rst;
  gpio_num_t pin_dc;
  gpio_num_t pin_cs;
  gpio_num_t pin_bl;
  spi_host_device_t spi_host;
} st7789_config_t;

/**
 * @brief Get ST7789 display HAL interface
 */
const display_hal_interface_t *st7789_get_interface(void);

#endif // ST7789_DRIVER_H
