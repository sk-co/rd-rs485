/*
 * crc8.h
 *
 *  Created on: 26 апр. 2019 г.
 *      Author: SERG
 */

#ifndef LAMP_CRC8_H_
#define LAMP_CRC8_H_

#include <cstdint>
#include <vector>

namespace crypt {

/*
  Name  : CRC-8
  Poly  : 0x31    x^8 + x^5 + x^4 + 1
  Init  : 0xFF
  Revert: false
  XorOut: 0x00
  Check : 0xF7 ("123456789")
  MaxLen: 15 байт(127 бит) - обнаружение
    одинарных, двойных, тройных и всех нечетных ошибок
*/
uint8_t Crc8(const std::vector<uint8_t> &data);
uint8_t Crc8(const std::vector<uint8_t> &data, uint8_t crc);
uint8_t Crc8(const uint8_t *data, int data_len);
uint8_t Crc8(const uint8_t *data, int data_len, uint8_t crc);

}

#endif /* LAMP_CRC8_H_ */
