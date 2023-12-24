
#ifndef __UART_H__
#define __UART_H__

#include "gpio-def.h"

/*!
 * UART peripheral ID
 */
typedef enum {
  UART_1,
  UART_2,
  UART_3,
  UART_USB_CDC = 255,
} UartId_t;

/*!
 * UART notification identifier
 */
typedef enum {
  UART_NOTIFY_TX,
  UART_NOTIFY_RX,
  UART_NOTIFY_ERR,
} UartNotifyId_t;

/*!
 * UART object type definition
 */
typedef struct {
  UartId_t UartId;
  bool IsInitialized;
  Gpio_t tx_pin;
  Gpio_t rx_pin;
  Gpio_t rts_de_pin;
  Gpio_t cts_pin;
  uint8_t rx_byte;     // Последний принятый байт.
  uint8_t *rx_buf = nullptr;
  uint32_t rx_buf_len = 0;
  uint32_t rx_buf_max_len = 0;
  void *hal_obj = nullptr;
  void *hal_tx_dma_obj = nullptr;
  void *context = nullptr;
  bool is_reading = false;
  bool is_writing = false;
  /*!
   * IRQ user notification callback prototype.
   */
  void ( *IrqNotify )(UartNotifyId_t id, void* context);
} Uart_t;

/*!
 * Operation Mode for the UART
 */
typedef enum {
  TX_ONLY = 0,
  RX_ONLY,
  RX_TX
} UartMode_t;

/*!
 * UART word length
 */
typedef enum {
  UART_8_BIT = 0,
  UART_9_BIT
} WordLength_t;

/*!
 * UART stop bits
 */
typedef enum {
  UART_1_STOP_BIT = 0,
  UART_1_5_STOP_BIT,
  UART_2_STOP_BIT,
  UART_0_5_STOP_BIT,
} StopBits_t;

/*!
 * UART parity
 */
typedef enum {
  NO_PARITY = 0,
  ODD_PARITY,
  EVEN_PARITY,
} Parity_t;

/*!
 * UART flow control
 */
typedef enum {
  NO_FLOW_CTRL = 0,
  RTS_FLOW_CTRL,
  CTS_FLOW_CTRL,
  RTS_CTS_FLOW_CTRL,
  DE_FLOW_CRTL
} FlowCtrl_t;

#endif // __UART_H__
