#ifndef CST816S_DRIVER_H
#define CST816S_DRIVER_H

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "touch_hal.h"

/**
 * @brief CST816S configuration structure
 */
typedef struct {
  i2c_master_bus_handle_t i2c_bus;
  gpio_num_t pin_rst;
  gpio_num_t pin_int;
  uint16_t h_res;
  uint16_t v_res;
} cst816s_config_t;

/**
 * @brief Get CST816S touch HAL interface
 */
const touch_hal_interface_t *cst816s_get_interface(void);

#endif // CST816S_DRIVER_H
