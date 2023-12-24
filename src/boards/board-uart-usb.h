//
// Created by SERG on 16.12.2021.
//

#ifndef LORAWAN_DEV_SRC_BOARDS_BOARD_UART_USB_H_
#define LORAWAN_DEV_SRC_BOARDS_BOARD_UART_USB_H_

#include <stdint.h>
#include <stddef.h>
#include "uart-def.h"

namespace board {

void USB_CDC_Init(Uart_t *obj, UartId_t uartId, PinNames dp, PinNames dm, PinNames pu);
int USB_CDC_Config(Uart_t *obj, uint32_t baudrate,
                   WordLength_t wordLength, StopBits_t stopBits, Parity_t parity);
void USB_CDC_DeInit(Uart_t *obj);
int USB_CDC_GetBaudrate();
int USB_CDC_StartSend(Uart_t *obj, const uint8_t *data, size_t len);
int USB_CDC_Send(Uart_t *obj, const uint8_t *data, size_t len, uint32_t timeout_ms);
int USB_CDC_Receive(Uart_t *obj, uint8_t *data, size_t len, uint32_t timeout_ms);
void USB_CDC_ClearRxBuf(Uart_t *obj);
}

#endif //LORAWAN_DEV_SRC_BOARDS_BOARD_UART_USB_H_
