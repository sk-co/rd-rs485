#include "rs485.h"

namespace {

void UartNotify(UartNotifyId_t id, void *context) {
  auto rs485 = reinterpret_cast<ext::Rs485Interface *>(context);
  switch (id) {
    case UART_NOTIFY_TX:rs485->IrqHandleTx();
      break;
    case UART_NOTIFY_RX:rs485->IrqHandleRx();
      break;
    case UART_NOTIFY_ERR:rs485->IrqHandleErr();
      break;
  }
}
}

namespace ext {

void Rs485Interface::Init(Uart_t *uart) {
  uart_ = uart;
  uart_->context = this;
  uart_->IrqNotify = UartNotify;
}
int Rs485Interface::Config(ConfigParam param) {
  if (board::UartConfig(uart_,
                        RX_TX,
                        param.baudrate,
                        param.wordLength,
                        param.stopBits,
                        param.parity,
                        DE_FLOW_CRTL))
    return 1;
  board::UartInitDmaTx(uart_);
  return 0;
}
void Rs485Interface::On() {
  tx_buf_.Clear();
  rx_buf_.Reset();
  error_ = Error::NO;
}
void Rs485Interface::Off() {
  StopRead();
  StopWrite();
}
bool Rs485Interface::Write(const uint8_t *data, uint32_t size, uint32_t timeout_ms) {
  auto res = board::UartWrite(uart_, data, size, timeout_ms);
  return res;
}
bool Rs485Interface::Read(uint8_t *data, uint32_t size, uint32_t timeout_ms) {
  return board::UartRead(uart_, data, size, timeout_ms);
}
void Rs485Interface::IrqHandleTx() {
  tx_buf_.Clear();
}
void Rs485Interface::IrqHandleRx() {
  if(is_read_pause_)
    return;
  if (!rx_buf_.PushBack(uart_->rx_byte)) {
    error_ = Error::RX_BUFFER_OVERFLOW;
  }
}
void Rs485Interface::IrqHandleErr() {
  error_ = Error::UART_OVERFLOW;
}
bool Rs485Interface::StartRead() {
  StopRead();
  board::UartClearRxBuf(uart_);
  is_read_pause_ = false;
  return board::UartStartRead(uart_);
}
void Rs485Interface::StopRead() {
  board::UartStopRead(uart_);
}
bool Rs485Interface::StartWrite(const uint8_t *data, uint32_t size) {
  if (uart_->is_writing)
    return false;
  // Копируем данные из одного буфера в другой.
  tx_buf_.Clear();
  tx_buf_.Append(data, size);
  if (board::UartStartWrite(uart_, tx_buf_.Data(), tx_buf_.Size()))
    return true;
  return false;
}
void Rs485Interface::StopWrite() {
  board::UartStopWrite(uart_);
}

}