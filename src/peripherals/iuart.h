#ifndef MOLOT_SRC_PERIPHERAL_INTERFACE_IUART_H_
#define MOLOT_SRC_PERIPHERAL_INTERFACE_IUART_H_

#include "board-uart.h"

class IUart {
 public:
  struct ConfigParam {
    uint32_t baudrate;
    WordLength_t wordLength;
    StopBits_t stopBits;
    Parity_t parity;
  };
  enum class Error {
    NO,
    RX_BUFFER_OVERFLOW,
    TX_BUFFER_OVERFLOW,
    UART_OVERFLOW,
  };
  virtual int Config(ConfigParam param) = 0;
  virtual void On() = 0;
  virtual void Off() = 0;
  // Блокируемая запись
  virtual bool Write(const uint8_t *data, uint32_t size, uint32_t timeout_ms) = 0;
  // Блокируемое чтение
  virtual bool Read(uint8_t *data, uint32_t size, uint32_t timeout_ms) = 0;
  // Не блокируемые функции.
  virtual bool StartRead() = 0;
  virtual void StopRead() = 0;
  virtual bool StartWrite(const uint8_t *data, uint32_t size) = 0;
  virtual void StopWrite() = 0;
  virtual Error GetError() const = 0;
  virtual size_t GetRxDataSize() const = 0;
  virtual size_t ReadRxData(uint8_t *data, size_t len) = 0;
  virtual bool IsReading() const = 0;
  virtual bool IsWriting() const = 0;
  virtual void ReadPause() = 0;
  virtual void ReadResume() = 0;
};


#endif //MOLOT_SRC_PERIPHERAL_INTERFACE_IUART_H_
