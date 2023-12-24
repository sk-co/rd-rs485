/*!
 * \file      adc.h
 *
 * \brief     Generic ADC driver implementation
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
#ifndef __ADC_H__
#define __ADC_H__

#include <cstdint>
#include "gpio-def.h"

typedef enum {
  ADC_CN_1,
  ADC_CN_2,
} AdcChannelId_t;

typedef struct
{
  Gpio_t pin;
  uint32_t channel;
} AdcChannel_t;

/*!
 * ADC object type definition
 */
typedef struct
{
  AdcChannel_t channel_1;
  AdcChannel_t channel_2;
  uint32_t resolution;      // Разрядность ацп.
  uint32_t period_mks;      // Для запуска по таймеру, период запуска.
  void *hal_obj;
}Adc_t;

typedef struct {
  uint32_t sample_time;
}ChannelConfig_t;

#endif // __ADC_H__
