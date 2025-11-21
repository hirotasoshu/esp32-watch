#ifndef PCF85063_DRIVER_H
#define PCF85063_DRIVER_H

#include "driver/i2c_master.h"
#include "rtc_hal.h"
#include <stdbool.h>

/**
 * @brief PCF85063 configuration structure
 */
typedef struct {
  i2c_master_bus_handle_t i2c_bus;
  bool set_compile_time; // Set time to __DATE__ __TIME__ on init
} pcf85063_config_t;

/**
 * @brief Get PCF85063 driver interface
 *
 * @return const rtc_hal_interface_t* Pointer to PCF85063 implementation
 */
const rtc_hal_interface_t *pcf85063_get_interface(void);

#endif // PCF85063_DRIVER_H
