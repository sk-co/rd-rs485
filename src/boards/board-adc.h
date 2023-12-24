/*!
 * \file      adc-board.h
 *
 * \brief     Target board ADC driver implementation
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
#ifndef __ADC_BOARD_H__
#define __ADC_BOARD_H__

#include <cstdint>
#include "adc-def.h"

namespace board {

/*!
 * \brief Initializes the ADC object and MCU peripheral
 *
 * \param [IN] obj      ADC object
 * \param [IN] adcInput ADC input pin
 */
int AdcInit( Adc_t *obj);
void AdcDeinit( Adc_t *obj );

// Инициализация канала ацп.
int AdcConfigChannel( Adc_t *obj, AdcChannelId_t channel_id, PinNames pin, uint32_t channel,
                      ChannelConfig_t &config);
// Предустановленная инициализация канала АЦП.
int AdcConfigChannel(Adc_t *obj, AdcChannelId_t channel_id);
void AdcSleep( Adc_t *obj );
void AdcWakeup( Adc_t *obj );
/*!
 * \brief Reads the value of the given channel
 *
 * \param [IN] obj     ADC object
 */
uint32_t AdcStart(Adc_t *obj);
uint32_t AdcStartDma(Adc_t *obj, uint16_t *data, uint32_t len);
bool AdcWaitComplete(Adc_t *obj);
uint32_t AdcStop(Adc_t *obj);
uint32_t AdcRead( Adc_t *obj, AdcChannelId_t channel_id );
int16_t AdcCalcTempValue(Adc_t *obj, uint32_t temp);
uint16_t AdcReadWithTimeout(Adc_t *obj, uint32_t timeout_ms);
int DefineVRef();
float GetVRef();
void BatteryTest();
uint32_t AdcGetStartTime();


}

#endif // __ADC_BOARD_H__
