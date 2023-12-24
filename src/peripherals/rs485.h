#ifndef MOLOT_SRC_PERIPHERAL_RS485_H_
#define MOLOT_SRC_PERIPHERAL_RS485_H_

#include "board-uart.h"
#include "board-gpio.h"
#include "iuart.h"
#include "round_buf.hpp"
#include "static-vector.hpp"

namespace ext {

class Rs485Interface : public IUart {
 public:
  static const size_t kRxBufferSize = 256;
  static const size_t kTxBufferSize = 256;

  void Init(Uart_t *uart);
  [[nodiscard]] bool IsInit() { return uart_ != nullptr; }
  int Config(ConfigParam param) override;
  void On() override;
  void Off() override;
  // Блокируемая запись
  [[nodiscard]] bool Write(const uint8_t *data, uint32_t size, uint32_t timeout_ms) override;
  // Блокируемое чтение
  [[nodiscard]] bool Read(uint8_t *data, uint32_t size, uint32_t timeout_ms) override;
  // Не блокируемые функции.
  [[nodiscard]] bool StartRead() override;
  void StopRead() override;
  [[nodiscard]] bool StartWrite(const uint8_t *data, uint32_t size) override;
  void StopWrite() override;
  [[nodiscard]] Error GetError() const override { return error_; }
  [[nodiscard]] size_t GetRxDataSize() const override { return rx_buf_.Size(); }
  size_t ReadRxData(uint8_t *data, size_t len) override { return rx_buf_.Read(data, len); }
  [[nodiscard]] bool IsReading() const override { return uart_->is_reading; }
  [[nodiscard]] bool IsWriting() const override { return uart_->is_writing; }

  void IrqHandleTx();
  void IrqHandleRx();
  void IrqHandleErr();

 private:
  Uart_t *uart_ = nullptr;
  utils::RoundBuf<uint8_t, kRxBufferSize> rx_buf_;
  utils::StaticVector<uint8_t, kTxBufferSize> tx_buf_;
  Error error_ = Error::NO;
};
}
#endif //MOLOT_SRC_PERIPHERAL_RS485_H_
