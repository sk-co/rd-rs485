#ifndef __SPI_BOARD_H__
#define __SPI_BOARD_H__

#include "spi-def.h"

namespace board {

/*!
 * \brief Initializes the SPI object and MCU peripheral
 *
 * \remark When NSS pin is software controlled set the pin name to NC otherwise
 *         set the pin name to be used.
 *
 * \param [IN] obj  SPI object
 * \param [IN] mosi SPI MOSI pin name to be used
 * \param [IN] miso SPI MISO pin name to be used
 * \param [IN] sclk SPI SCLK pin name to be used
 * \param [IN] nss  SPI NSS pin name to be used
 */
void SpiInit( Spi_t *obj, SpiId_t spiId, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss );

/*!
 * \brief De-initializes the SPI object and MCU peripheral
 *
 * \param [IN] obj SPI object
 */
void SpiDeInit( Spi_t *obj );

/*!
 * \brief Configures the SPI peripheral
 *
 * \remark Slave mode isn't currently handled
 *
 * \param [IN] obj   SPI object
 * \param [IN] bits  Number of bits to be used. [8 or 16]
 * \param [IN] cpol  Clock polarity
 * \param [IN] cpha  Clock phase
 * \param [IN] slave When set the peripheral acts in slave mode
 */
void SpiConfig(Spi_t *obj, uint32_t hz, int32_t bits, int32_t cpol, int32_t cpha, int8_t slave );

/*!
 * \brief Sets the SPI speed
 *
 * \param [IN] obj SPI object
 * \param [IN] hz  SPI clock frequency in hz
 */
void SpiFrequency( Spi_t *obj, uint32_t hz );

/*!
 * \brief Sends outData and receives inData
 *
 * \param [IN] obj     SPI object
 * \param [IN] outData Byte to be sent
 * \retval inData      Received byte.
 */
uint16_t SpiInOut( Spi_t *obj, uint16_t outData );

int SpiInOut(Spi_t *obj, const uint8_t* tx_data, uint8_t* rx_data, uint32_t size);
//int SpiInOut(Spi_t *obj, const uint8_t* tx_data, uint8_t* rx_data, uint32_t size, uint32_t timeout);
int SpiIn(Spi_t *obj, uint8_t* rx_data, uint32_t size);
int SpiOut(Spi_t *obj, const uint8_t* tx_data, uint32_t size);
}

#endif // __SPI_BOARD_H__
