/*
 * crc.h
 *
 *  Created on: 7 мая 2019 г.
 *      Author: SERG
 */

#ifndef CRYPTO_CRC16_H_
#define CRYPTO_CRC16_H_

#include <cstdint>
#include <cstdlib>

namespace crypt {

/*
  Name  : CRC-16 CCITT
  Poly  : 0x1021    x^16 + x^12 + x^5 + 1
  Init  : 0xFFFF
  Revert: false
  XorOut: 0x0000
  Check : 0x29B1 ("123456789")
  MaxLen: 4095 байт (32767 бит) - обнаружение
    одинарных, двойных, тройных и всех нечетных ошибок
*/
uint16_t Crc16(const uint8_t *data, size_t data_len);
uint16_t Crc16(const uint8_t *data, size_t data_len, uint16_t crc);

// Crc ModBus
uint16_t Crc16ModBus(const uint8_t *data, size_t data_len);
uint16_t Crc16ModBus(const uint8_t *data, size_t data_len, uint16_t crc);

}



#endif /* CRYPTO_CRC16_H_ */
