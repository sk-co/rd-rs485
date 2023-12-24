#ifndef __I2C_BOARD_H__
#define __I2C_BOARD_H__

#include <cstdint>
#include "i2c-def.h"

namespace board {

/*!
 * \brief Initializes the I2C object and MCU peripheral
 *
 * \param [IN] obj  I2C object
 * \param [IN] scl  I2C scl_pin pin name to be used
 * \param [IN] sda  I2C sda_pin pin name to be used
 */
void I2cInit(I2c_t *obj, I2cId_t i2cId, PinNames scl, PinNames sda);

/*!
 * \brief Initializes the I2C object and MCU peripheral
 *
 * \param [IN] obj              I2C object
 * \param [IN] dutyCycle        Signal duty cycle
 * \param [IN] ackAddrMode      7bit or 10 bit addressing
 * \param [IN] I2cFrequency     I2C bus clock frequency
 */
void I2cConfig(I2c_t *obj, uint32_t I2cFrequency,
               I2cDutyCycle dutyCycle, I2cAckAddrMode ackAddrMode, bool is_nostretch);

/*!
 * \brief DeInitializes the I2C object and MCU peripheral
 *
 * \param [IN] obj  I2C object
 */
void I2cDeInit(I2c_t *obj);

/*!
 * \brief Reset the I2C object and MCU peripheral
 *
 * \param [IN] obj  I2C object
 */
void I2cResetBus(I2c_t *obj);

/*!
 * \brief WriteVerify data buffer_ to the I2C device
 *
 * \param [IN] obj              I2C object
 * \param [IN] deviceAddr       device address
 * \param [IN] buffer           data buffer_ to write
 * \param [IN] size             number of data bytes to write
 */
bool I2cWrite(I2c_t *obj, uint8_t devAddr, uint8_t *buffer, uint16_t size,uint32_t timeout_ms);

/*!
 * \brief Read data buffer_ from the I2C device
 *
 * \param [IN] obj              I2C object
 * \param [IN] deviceAddr       device address
 * \param [IN] buffer           data buffer_ to read
 * \param [IN] size             number of data bytes to read
 */
bool I2cRead(I2c_t *obj, uint8_t devAddr, uint8_t *buffer, uint16_t size, uint32_t timeout_ms);

/*!
 * \brief WriteVerify data buffer_ starting at addr to the I2C device
 *
 * \param [IN] obj              I2C object
 * \param [IN] deviceAddr       device address
 * \param [IN] addr             data address
 * \param [IN] buffer           data buffer_ to write
 * \param [IN] size             number of data bytes to write
 */
bool I2cWriteMem(I2c_t *obj, uint8_t devAddr, uint16_t addr, uint8_t addrSize,
                    uint8_t *buffer, uint16_t size, uint32_t timeout_ms);

/*!
 * \brief Read data buffer_ starting at addr from the I2C device
 *
 * \param [IN] obj              I2C object
 * \param [IN] deviceAddr       device address
 * \param [IN] addr             data address
 * \param [IN] buffer           data buffer_ to read
 * \param [IN] size             number of data bytes to read
 */
bool I2cReadMem(I2c_t *obj, uint8_t devAddr, uint16_t addr, uint8_t addrSize,
                   uint8_t *buffer, uint16_t size, uint32_t timeout_ms);

}

#endif // __I2C_BOARD_H__
