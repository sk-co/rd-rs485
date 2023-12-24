/*!
 * \file      gpio.h
 *
 * \brief     GPIO driver implementation
 *
 * \remark: Relies on the specific board GPIO implementation as well as on
 *          IO expander driver implementation if one is available on the target
 *          board.
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdint.h>
#include "pinName-board.h"

/*!
 * Board GPIO pin names
 */
typedef enum {
  MCU_PINS,
  // Not connected
  NC = (int) 0xFFFFFFFF
} PinNames;

/*!
 * Operation Mode for the GPIO
 */
typedef enum {
  PIN_INPUT = 0,
  PIN_OUTPUT,
  PIN_ALTERNATE_FCT,
  PIN_ANALOGIC,
  PIN_ADC,
} PinModes;

/*!
 * Add a pull-up, a pull-down or nothing on the GPIO line
 */
typedef enum {
  PIN_NO_PULL = 0,
  PIN_PULL_UP,
  PIN_PULL_DOWN
} PinTypes;

/*!
 * Define the GPIO as Push-pull type or Open Drain
 */
typedef enum {
  PIN_PUSH_PULL = 0,
  PIN_OPEN_DRAIN
} PinConfigs;

/*!
 * Define the GPIO IRQ on a rising, falling or both edges
 */
typedef enum {
  NO_IRQ = 0,
  IRQ_RISING_EDGE,
  IRQ_FALLING_EDGE,
  IRQ_RISING_FALLING_EDGE
} IrqModes;

/*!
 * Define the IRQ priority on the GPIO
 */
typedef enum {
  IRQ_VERY_LOW_PRIORITY = 0,
  IRQ_LOW_PRIORITY,
  IRQ_MEDIUM_PRIORITY,
  IRQ_HIGH_PRIORITY,
  IRQ_VERY_HIGH_PRIORITY
} IrqPriorities;

/*!
 * GPIO IRQ handler function prototype
 */
typedef void( GpioIrqHandler )(void *context);

/*!
 * Structure for the GPIO
 */
typedef struct {
  PinNames pin = NC;
  uint16_t pinIndex {};
  void *port = nullptr;
  uint16_t portIndex {};
  PinTypes pull {};
  void *Context = nullptr;
  GpioIrqHandler *IrqHandler = nullptr;
} Gpio_t;

#endif // __GPIO_H__
