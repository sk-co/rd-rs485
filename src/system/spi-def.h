/*!
 * \file      spi-board.h
 *
 * \brief     SPI driver implementation
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
#ifndef __SPI_H__
#define __SPI_H__

#include "gpio-def.h"

/*!
 * SPI peripheral ID
 */
typedef enum {
  SPI_1,
  SPI_2,
  SPI_MANUAL,
}SpiId_t;

/*!
 * SPI object type definition
 */
typedef struct Spi_s {
  SpiId_t SpiId;
  Gpio_t Mosi;
  Gpio_t Miso;
  Gpio_t Sclk;
  Gpio_t Nss;
  void *hal_obj;
}Spi_t;



#endif // __SPI_H__
