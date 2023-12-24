/*
 * crc32.h
 *
 *  Created on: 6 июн. 2019 г.
 *      Author: SERG
 */

#ifndef CRYPTO_CRC32_H_
#define CRYPTO_CRC32_H_

#include <cstdint>
#include <vector>

namespace crypt {

/*
  Name  : CRC-32
  Poly  : 0x04C11DB7    x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11
                       + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
  Init  : 0xFFFFFFFF
  Revert: true
  XorOut: 0xFFFFFFFF
  Check : 0xCBF43926 ("123456789")
  MaxLen: 268 435 455 байт (2 147 483 647 бит) - обнаружение
   одинарных, двойных, пакетных и всех нечетных ошибок
*/
uint32_t Crc32(const std::vector<uint8_t> &data);
uint32_t Crc32(const std::vector<uint8_t> &data, uint32_t crc);
uint32_t Crc32(const uint8_t *data, int data_len);
uint32_t Crc32(const uint8_t *data, int data_len, uint32_t crc);
// Меньший размер кода, таблица в оперативке.
void Crc32BuildTable();
// Для получения crc от второй и четвертой функции надо вызывать
// Crc32_Init сначала, а Crc32_Final в конце.
constexpr uint32_t Crc32_Init() { return 0xFFFFFFFF; }
uint32_t Crc32_Final(uint32_t crc);

}


#endif /* CRYPTO_CRC32_H_ */
