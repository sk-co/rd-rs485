//
// Created by SERG on 16.02.2021.
//

#include "log.h"
#include "board-uart.h"
#include "shift-static-vectors.hpp"

#if (LOG_LEVEL != L_NO)

namespace {

Uart_t *uart_ = nullptr;
char str_[128];
utils::ShiftStaticVectors<char, 512, 3> buffers_;
volatile bool tx_complete_ = true;

void TxCompleteHandle();

void LogUartNotify(UartNotifyId_t id, void* context) {
  (void)context;
  if(id == UART_NOTIFY_TX)
    TxCompleteHandle();
}
void TxCompleteHandle() {
  buffers_.MoveReadToNext();
  tx_complete_ = true;
}

} // namespace

namespace utils::log {

void Init(Uart_t *uart, uint32_t baudrate) {
  if(uart_ != nullptr && uart == nullptr) {
    board::UartStopWrite(uart_);
    uart_->IrqNotify = nullptr;
  }
  uart_ = uart;
  if(uart_ == nullptr)
    return;
  board::UartDefaultConfig(uart_, baudrate, false);
  uart_->IrqNotify = LogUartNotify;
  tx_complete_ = true;
}
void Out(const char *str, ...) {
  if(!uart_)
    return;
  va_list arg_ptr;
  va_start(arg_ptr, str);
  size_t sz = vsprintf(str_, str, arg_ptr);
  va_end(arg_ptr);
  buffers_.Append(str_, sz);
  Flush();
}
void Flush() {
  if(!uart_)
    return;
  if (tx_complete_) {
    if (board::UartStartWrite(uart_,
                              (uint8_t *) buffers_.ReadData(),
                              buffers_.ReadSize())) {
      tx_complete_ = false;
      buffers_.MoveWriteToNext();
    }
  }
}
bool IsTransmited() {
  if(buffers_.WriteSize()) {
    Flush();
    return false;
  }
  return (!buffers_.IsCanRead() || tx_complete_);
}

} // namespace utils

#endif
