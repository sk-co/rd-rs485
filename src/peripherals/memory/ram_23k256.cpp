#include "ram_23k256.h"
#include <cstring>
#include "board-critical-section.h"
#include "board-spi.h"
#include "board-gpio.h"

namespace {

constexpr uint8_t kCmdRead = 0x03;
constexpr uint8_t kCmdWrite = 0x02;
constexpr uint8_t kCmdReadSR = 0x05;
constexpr uint8_t kCmdWriteSR = 0x01;
constexpr uint8_t kStatusSeq = 0xC0;


}

namespace ext {

Ram23k256::Ram23k256(Spi_t *hspi, PinNames pin_cs)
  : hspi_(hspi) {
  board::GpioInit(&pin_cs_,pin_cs,PIN_OUTPUT,PIN_PUSH_PULL,PIN_PULL_UP,1);
}
int Ram23k256::Init() {
  // SPI считаем, что уже сконфигурирована.
  // Переводим в режим последовательного чтения/записи.
  const uint8_t tx_wr[] = {kCmdWriteSR, kStatusSeq};
  bool is_good = false;
  {
    CRITICAL_SECTION_BEGIN();
    board::GpioWrite(&pin_cs_, 0);
    is_good = board::SpiOut(hspi_, tx_wr, sizeof(tx_wr));
    board::GpioWrite(&pin_cs_, 1);
    CRITICAL_SECTION_END();
  }
  if(!is_good)
    return 1;
  const uint8_t tx_rd[] = {kCmdReadSR, 0};
  uint8_t rx[] = {0, 0};
  {
    CRITICAL_SECTION_BEGIN();
    board::GpioWrite(&pin_cs_, 0);
    is_good = board::SpiInOut(hspi_, tx_rd, rx, sizeof(tx_rd));
    board::GpioWrite(&pin_cs_, 1);
    CRITICAL_SECTION_END();
  }
  if(!is_good)
    return 2;
  if(rx[1] != (kStatusSeq & 0xFE))
    return 3;
  return 0;
}
bool Ram23k256::Write(uint32_t addr, const uint8_t *data, uint32_t len) {
  const uint8_t data_tx[] = {
    kCmdWrite, uint8_t(addr >> 8), uint8_t(addr) };
  CRITICAL_SECTION_BEGIN();
  board::GpioWrite(&pin_cs_, 0);
  auto is_good = board::SpiOut(hspi_, data_tx, sizeof(data_tx));
  if(!is_good) {
    board::GpioWrite(&pin_cs_, 1);
    CRITICAL_SECTION_END();
    return false;
  }
  is_good = board::SpiOut(hspi_, data, len);
  board::GpioWrite(&pin_cs_, 1);
  CRITICAL_SECTION_END();
  return is_good;
}
bool Ram23k256::Read(uint32_t addr, uint8_t *data, uint32_t len) {
  const uint8_t data_tx[] = {
    kCmdRead, uint8_t(addr >> 8), uint8_t(addr) };
  CRITICAL_SECTION_BEGIN();
  board::GpioWrite(&pin_cs_, 0);
  auto is_good = board::SpiOut(hspi_, data_tx, sizeof(data_tx));
  if(!is_good) {
    board::GpioWrite(&pin_cs_, 1);
    CRITICAL_SECTION_END();
    return false;
  }
  is_good = board::SpiIn(hspi_, data, len);
  board::GpioWrite(&pin_cs_, 1);
  CRITICAL_SECTION_END();
  return is_good;
}
bool Ram23k256::Test() {
  constexpr auto kBlockSize = 1024;
  uint8_t data_tx[kBlockSize];
  uint8_t data_rx[kBlockSize];
  for(uint32_t i=0; i < kBlockSize; ++i)
    data_tx[i] = i;
  uint32_t count = kSizeInBytes/kBlockSize;
  for(uint32_t i=0; i < count; ++i) {
    memset(data_rx, 0, kBlockSize);
    if(!Write(i*kBlockSize, data_tx, kBlockSize))
      return false;
    if(!Read(i*kBlockSize, data_rx, kBlockSize))
      return false;
    if(memcmp(data_tx, data_rx, kBlockSize) != 0)
      return false;
  }
  return true;
}
}