#ifndef __I2C_H__
#define __I2C_H__

#include "gpio-def.h"

/*!
 * I2C peripheral ID
 */
typedef enum {
  I2C_1,
  I2C_2,
} I2cId_t;

/*!
 * I2C object type definition
 */
typedef struct {
  I2cId_t I2cId;
  Gpio_t scl_pin;
  Gpio_t sda_pin;
  void *hal_obj;
} I2c_t;

/*!
 * I2C signal duty cycle
 */
typedef enum {
  I2C_DUTY_CYCLE_2 = 0,
  I2C_DUTY_CYCLE_16_9
} I2cDutyCycle;

/*!
 * I2C select if the acknowledge in after the 7th or 10th bit
 */
typedef enum {
  I2C_ACK_ADD_7_BIT = 0,
  I2C_ACK_ADD_10_BIT
} I2cAckAddrMode;

/*!
 * Internal device address size
 */
typedef enum {
  I2C_ADDR_SIZE_8 = 0,
  I2C_ADDR_SIZE_16,
} I2cAddrSize;

#endif // __I2C_H__
