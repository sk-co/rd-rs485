/*!
 * \file      board.h
 *
 * \brief     Target board general functions implementation
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
#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdint.h>
#include "utilities.h"


namespace board {

/*!
 * Possible power sources
 */
enum BoardPowerSources {
  USB_POWER = 0,
  BATTERY_POWER,
};

/*!
 * \brief Initializes the mcu.
 */
void BoardInitMcu();
// Инициализаия платы под загрузчик.
void MinimalInitMcu();
/*!
 * \brief Resets the mcu.
 */
void BoardResetMcu();

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu();

// Initializes the mcu after low power mode.
void BoardReInitMcu();


///*!
// * \brief Get the current battery level
// *
// * \retval value  battery level [  0: USB,
// *                                 1: Min level,
// *                                 x: level
// *                               254: fully charged,
// *                               255: Error]
// */
//uint8_t BoardGetBatteryLevel(void);

/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed(void);

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId(uint8_t *id);

/*!
 * \brief Manages the entry into ARM cortex deep-sleep mode
 */
void BoardLowPowerHandler();

///*!
// * \brief Get the board power source
// *
// * \retval value  power source [0: USB_POWER, 1: BATTERY_POWER]
// */
//uint8_t GetBoardPowerSource(void);

void InitRadio();
void InitUart1();
void InitUart2();

void InitI2c1();
void InitI2c2();
void InitSpi1();
void InitSpi2();
void InitPwm(uint16_t period);
void InitRtc();
void InitAdc();
void DeInitAdc();
void PVD_On();
uint32_t GetSysTick();
void InitIWDG(uint32_t period_ms);
void RefreshIWDG();

void CalibrateSystemWakeupTimeFromStop();
// Выполнение действий в случае низкого уровня питания.
void LowPowerDetect();
// Переход на прошивку по заданному адресу.
void JumpToMainHex();

}


#endif // __BOARD_H__
