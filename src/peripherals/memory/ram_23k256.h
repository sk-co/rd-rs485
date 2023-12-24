#ifndef RD_RS485_SRC_PERIPHERALS_MEMORY_23K356_H_
#define RD_RS485_SRC_PERIPHERALS_MEMORY_23K356_H_

#include "spi-def.h"

namespace ext {

class Ram23k256 {
 public:
  static constexpr auto kSizeInBytes = 32*1024;
  Ram23k256(Spi_t *hspi, PinNames pin_cs);
  int Init();
  bool Write(uint32_t addr, const uint8_t *data, uint32_t len);
  bool Read(uint32_t addr, uint8_t *data, uint32_t len);
  bool Test();

 private:
  Spi_t *hspi_ = nullptr;
  Gpio_t pin_cs_ = {};
};

}

#endif //RD_RS485_SRC_PERIPHERALS_MEMORY_23K356_H_
