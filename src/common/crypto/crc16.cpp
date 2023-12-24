/*
 * crc.cpp
 *
 *  Created on: 7 мая 2019 г.
 *      Author: SERG
 */

#include <crc16.h>

namespace crypt {

uint16_t Crc16(const uint8_t *data, size_t data_len) {
  uint16_t crc = 0xFFFF;
  return Crc16(data, data_len, crc);
}

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
uint16_t Crc16(const uint8_t *data, size_t data_len, uint16_t crc) {
  for (size_t pos = 0; pos < data_len; pos++) {
    crc ^= uint16_t(data[pos]) << 8;
    for (int i = 0; i < 8; i++)
        crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
  }
  return crc;
}

uint16_t Crc16ModBus(const uint8_t *data, size_t data_len) {
  uint16_t crc = 0xFFFF;
  return Crc16ModBus(data, data_len, crc);
}
uint16_t Crc16ModBus(const uint8_t *data, size_t data_len, uint16_t crc) {
  for (uint32_t pos = 0; pos < data_len; pos++) {
    crc ^= *(data + pos);
    for (size_t i = 8; i != 0; i--) {
      if ((crc & 0x0001u) != 0) {
        crc >>= 1u;
        crc ^= 0xA001u;
      } else
        crc >>= 1u;
    }
  }
  return crc;
}
}
