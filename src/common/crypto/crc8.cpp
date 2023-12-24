/*
 * crc8.cpp
 *
 *  Created on: 26 апр. 2019 г.
 *      Author: SERG
 */
#include "crc8.h"

namespace crypt {

unsigned char Crc8(const std::vector<uint8_t> &data) {
  unsigned char crc = 0xFF;
  return Crc8(data, crc);
}

uint8_t Crc8(const std::vector<uint8_t> &data, uint8_t crc) {
  return Crc8(data.data(), data.size(), crc);
}

uint8_t Crc8(const uint8_t *data, int data_len) {
  unsigned char crc = 0xFF;
  return Crc8(data, data_len, crc);
}

uint8_t Crc8(const uint8_t *data, int data_len, uint8_t crc) {
  for (int pos = 0; pos < data_len; pos++) {
    crc ^= data[pos];
    for (int i = 0; i < 8; i++)
      crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
  }
  return crc;
}

}


