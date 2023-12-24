
#ifndef __UART_BOARD_H__
#define __UART_BOARD_H__

#include <stdint.h>
#include <stddef.h>
#include "uart-def.h"

namespace board {

/*!
 * \brief Initializes the UART object and MCU peripheral
 *
 * \param [IN] obj  UART object
 * \param [IN] tx   UART tx_pin pin name to be used
 * \param [IN] rx   UART rx pin name to be used
 */
void UartInit(Uart_t *obj, UartId_t uartId, PinNames tx, PinNames rx,
              PinNames rts_de = NC, PinNames cts = NC);

/*!
 * \brief Configures the UART object and MCU peripheral
 *
 * \remark UartInit function must be called first.
 *
 * \param [IN] obj          UART object
 * \param [IN] mode         Mode of operation for the UART
 * \param [IN] baudrate     UART baudrate
 * \param [IN] wordLength   packet length
 * \param [IN] stopBits     stop bits setup
 * \param [IN] parity       packet parity
 * \param [IN] flowCtrl     UART flow control
 */
int UartConfig(Uart_t *obj,
                UartMode_t mode,
                uint32_t baudrate,
                WordLength_t wordLength,
                StopBits_t stopBits,
                Parity_t parity,
                FlowCtrl_t flowCtrl);

// Инициализация передачи по uart через dma.
void UartInitDmaTx(Uart_t *obj);
/*!
 * \brief DeInitializes the UART object and MCU pins
 *
 * \param [IN] obj  UART object
 */
void UartDeInit(Uart_t *obj);
void UartDeconfig(Uart_t *obj);
// Инициализация Uart по умолчанию.
int UartDefaultConfig(Uart_t *uart, int baudrate, bool with_dma_tx);
// Возвращает настройки по умолчанию.
void UartGetConf(Uart_t *uart, int *baudrate, bool *with_dma_tx);
// Передача данных по Uart.
// Возвращает false в случае ошибки.
bool UartStartWrite(Uart_t *obj, const uint8_t *data, size_t len);
// Остановка передачи.
void UartStopWrite(Uart_t *obj);
// Запуск побайтного приема по Uart.
bool UartStartRead(Uart_t *obj);
// Остановка приема.
void UartStopRead(Uart_t *obj);
// Блокирующее чтение, возвращает false в случае ошибки.
bool UartRead(Uart_t *obj, uint8_t *data, size_t len, uint32_t timeout_ms);
// Блокирующая запись, возвращает false в случае ошибки.
bool UartWrite(Uart_t *obj, const uint8_t *data, size_t len, uint32_t timeout_ms);
// Очистка буфера для входных данных.
void UartClearRxBuf(Uart_t *obj);

}



#endif // __UART_BOARD_H__
