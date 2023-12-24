
#include "board-uart.h"
#include "mcu.h"
#include "utilities.h"
#include "board.h"
#include "board-gpio.h"
#include "board-peripherals.h"
#include "error-handler.h"
#include "sysIrqHandlers.h"


namespace board {

UART_HandleTypeDef UartHandle_1;
UART_HandleTypeDef UartHandle_2;
DMA_HandleTypeDef hdma_uart1_tx;
DMA_HandleTypeDef hdma_uart2_tx;
Uart_t uart1;
Uart_t uart2;

void UartInit(Uart_t *obj, UartId_t uartId, PinNames tx, PinNames rx, PinNames rts_de, PinNames cts) {
  obj->UartId = uartId;
  obj->hal_obj = nullptr;
  obj->hal_tx_dma_obj = nullptr;
  uint32_t tx_alt_fn = GPIO_AF7_USART1;
  uint32_t rx_alt_fn = GPIO_AF7_USART1;
  switch (obj->UartId) {
    case UART_1:
      __HAL_RCC_USART1_FORCE_RESET();
      __HAL_RCC_USART1_RELEASE_RESET();
      __HAL_RCC_USART1_CLK_ENABLE();
      tx_alt_fn = GPIO_AF7_USART1;
      rx_alt_fn = GPIO_AF7_USART1;
      break;
    case UART_2:
      __HAL_RCC_USART2_FORCE_RESET();
      __HAL_RCC_USART2_RELEASE_RESET();
      __HAL_RCC_USART2_CLK_ENABLE();
      tx_alt_fn = GPIO_AF7_USART2;
      rx_alt_fn = GPIO_AF7_USART2;
      break;
    default:
      break;
  }
  board::GpioInit(&obj->tx_pin, tx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL,
                  PIN_PULL_UP, tx_alt_fn);
  board::GpioInit(&obj->rx_pin, rx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL,
                  PIN_PULL_UP, rx_alt_fn);
  if(rts_de != NC)
    board::GpioInit(&obj->rts_de_pin, rts_de, PIN_ALTERNATE_FCT, PIN_PUSH_PULL,
                    PIN_PULL_UP, tx_alt_fn);
  (void)cts;
  obj->IsInitialized = true;
}
int UartConfig(Uart_t *obj, UartMode_t mode, uint32_t baudrate,
                WordLength_t wordLength, StopBits_t stopBits,
                Parity_t parity, FlowCtrl_t flowCtrl) {
  UART_HandleTypeDef *uartHandle = nullptr;
  switch (obj->UartId) {
    case UART_1:
      uartHandle = &UartHandle_1;
      uartHandle->Instance = USART1;
      break;
    case UART_2:
      uartHandle = &UartHandle_2;
      uartHandle->Instance = USART2;
      break;
    default:
      assert_param(MCU_FAIL);
      return 1;
  }
  uartHandle->Init.BaudRate = baudrate;
  switch (mode) {
    case TX_ONLY: uartHandle->Init.Mode = UART_MODE_TX; break;
    case RX_ONLY: uartHandle->Init.Mode = UART_MODE_RX; break;
    case RX_TX: uartHandle->Init.Mode = UART_MODE_TX_RX; break;
  }
  switch (wordLength) {
    case UART_8_BIT: uartHandle->Init.WordLength = UART_WORDLENGTH_8B; break;
    case UART_9_BIT: uartHandle->Init.WordLength = UART_WORDLENGTH_9B; break;
  }
  switch (stopBits) {
    case UART_2_STOP_BIT:
      uartHandle->Init.StopBits = UART_STOPBITS_2;
      break;
    case UART_1_STOP_BIT:
    default:
      uartHandle->Init.StopBits = UART_STOPBITS_1;
      break;
  }
  switch (parity) {
    case NO_PARITY:
      uartHandle->Init.Parity = UART_PARITY_NONE;
      break;
    case EVEN_PARITY:
      uartHandle->Init.Parity = UART_PARITY_EVEN;
      break;
    case ODD_PARITY:
      uartHandle->Init.Parity = UART_PARITY_ODD;
      break;
  }
  switch (flowCtrl) {
    case RTS_FLOW_CTRL:
      uartHandle->Init.HwFlowCtl = UART_HWCONTROL_RTS;
      break;
    case CTS_FLOW_CTRL:
      uartHandle->Init.HwFlowCtl = UART_HWCONTROL_CTS;
      break;
    case RTS_CTS_FLOW_CTRL:
      uartHandle->Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
      break;
    default:
      uartHandle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  }
  uartHandle->Init.OverSampling = UART_OVERSAMPLING_16;
  uartHandle->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  uartHandle->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if(flowCtrl == DE_FLOW_CRTL) {
    if (HAL_RS485Ex_Init(uartHandle, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK) {
      return 2;
    }
  }
  else {
    if (HAL_UART_Init(uartHandle) != HAL_OK) {
      return 2;
    }
  }
//  if (HAL_UARTEx_SetTxFifoThreshold(uartHandle, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK) {
//    return 3;
//  }
//  if (HAL_UARTEx_SetRxFifoThreshold(uartHandle, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) {
//    return 3;
//  }
//  if (HAL_UARTEx_DisableFifoMode(uartHandle) != HAL_OK) {
//    return 3;
//  }
  auto irq = (uartHandle->Instance == USART1) ? USART1_IRQn : USART2_IRQn;
  HAL_NVIC_SetPriority(irq, 0, 0);
  HAL_NVIC_EnableIRQ(irq);
  obj->hal_obj = uartHandle;
  obj->is_writing = false;
  obj->is_reading = false;
  return 0;
}

void UartInitDmaTx(Uart_t *obj) {
  DMA_HandleTypeDef *hdma_handle;
  IRQn_Type irq;
  switch (obj->UartId) {
    case UART_1:
      hdma_handle = &hdma_uart1_tx;
      hdma_handle->Instance = DMA1_Channel4;
      irq = DMA1_Channel4_IRQn;
      break;
    case UART_2:
      hdma_handle = &hdma_uart2_tx;
      hdma_handle->Instance = DMA1_Channel7;
      irq = DMA1_Channel7_IRQn;
      break;
    default:
      assert_param(MCU_FAIL);
      return;
  }
  __HAL_RCC_DMA1_CLK_ENABLE();
  hdma_handle->Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_handle->Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_handle->Init.MemInc = DMA_MINC_ENABLE;
  hdma_handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_handle->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_handle->Init.Mode = DMA_NORMAL;
  hdma_handle->Init.Priority = DMA_PRIORITY_MEDIUM;
  if (HAL_DMA_Init(hdma_handle) != HAL_OK) {
    ErrorHandler(nullptr);
  }
  HAL_NVIC_SetPriority(irq, 0, 0);
  HAL_NVIC_EnableIRQ(irq);
  obj->hal_tx_dma_obj = hdma_handle;
  __HAL_LINKDMA(reinterpret_cast<UART_HandleTypeDef*> (obj->hal_obj),
                hdmatx,
                *reinterpret_cast<DMA_HandleTypeDef*>(obj->hal_tx_dma_obj));
}

void UartDeInit(Uart_t *obj) {
  switch (obj->UartId) {
    case UART_1:
      HAL_NVIC_DisableIRQ(USART1_IRQn);
      HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
      __HAL_RCC_USART1_FORCE_RESET();
      __HAL_RCC_USART1_RELEASE_RESET();
      __HAL_RCC_USART1_CLK_DISABLE();
      break;
    case UART_2:
      HAL_NVIC_DisableIRQ(USART2_IRQn);
      HAL_NVIC_DisableIRQ(DMA1_Channel2_IRQn);
      __HAL_RCC_USART2_FORCE_RESET();
      __HAL_RCC_USART2_RELEASE_RESET();
      __HAL_RCC_USART2_CLK_DISABLE();
      break;
    default:
      assert_param(MCU_FAIL);
  }
  board::GpioDeInit(&obj->tx_pin);
  board::GpioDeInit(&obj->rx_pin);
  if(obj->hal_tx_dma_obj) {
    HAL_DMA_DeInit(reinterpret_cast<DMA_HandleTypeDef*>(obj->hal_tx_dma_obj));
    obj->hal_tx_dma_obj = nullptr;
  }
  HAL_UART_DeInit(reinterpret_cast<UART_HandleTypeDef*> (obj->hal_obj));
  obj->hal_obj = nullptr;
  obj->IsInitialized = false;
}
void UartDeconfig(Uart_t *obj) {
  if(obj == nullptr)
    return;
  if(obj->hal_obj) {
    UartStopRead(obj);
    UartStopWrite(obj);
    obj->IrqNotify = nullptr;
    if (obj->hal_tx_dma_obj) {
      HAL_DMA_DeInit(reinterpret_cast<DMA_HandleTypeDef *>(obj->hal_tx_dma_obj));
      obj->hal_tx_dma_obj = nullptr;
    }
    HAL_UART_DeInit(reinterpret_cast<UART_HandleTypeDef*> (obj->hal_obj));
    obj->hal_obj = nullptr;
  }
}
// Инициализация Uart по умолчанию.
int UartDefaultConfig(Uart_t *uart, int baudrate, bool with_dma_tx) {
  if(UartConfig(uart, RX_TX, baudrate, UART_8_BIT,
             UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL)){
    return 1;
  }
  if(with_dma_tx) {
    UartInitDmaTx(uart);
  }
  return 0;
}
// Возвращает настройки по умолчанию.
void UartGetConf(Uart_t *uart, int *baudrate, bool *with_dma_tx) {
  *baudrate = int((!uart->hal_obj) ? -1 :
      reinterpret_cast<UART_HandleTypeDef *>(uart->hal_obj)->Init.BaudRate);
  *with_dma_tx = (uart->hal_tx_dma_obj != nullptr);
}
// Передача данных по Uart.
// Возвращает false в случае ошибки.
bool UartStartWrite(Uart_t *obj, const uint8_t *data, size_t len) {
  if (obj->hal_tx_dma_obj) {
    obj->is_writing =
      HAL_UART_Transmit_DMA(reinterpret_cast<UART_HandleTypeDef *>(obj->hal_obj),
                            const_cast<uint8_t *>(data), len) == HAL_OK;
  }
  else {
    obj->is_writing =
      HAL_UART_Transmit_IT(reinterpret_cast<UART_HandleTypeDef *>(obj->hal_obj),
                           const_cast<uint8_t *>(data), len) == HAL_OK;
  }
  return obj->is_writing;
}
void UartStopWrite(Uart_t *obj) {
  obj->is_writing = false;
  HAL_UART_AbortTransmit_IT(reinterpret_cast<UART_HandleTypeDef *>(obj->hal_obj));
}
// Запуск приема по Uart.
// Возвращает false в случае ошибки.
bool UartStartRead(Uart_t *obj) {
  obj->is_reading =
    HAL_UART_Receive_IT(reinterpret_cast<UART_HandleTypeDef *>(obj->hal_obj),
                        &obj->rx_byte, 1) == HAL_OK;
  return obj->is_reading;
}
void UartStopRead(Uart_t *obj) {
  obj->is_reading = false;
  HAL_UART_AbortReceive_IT(reinterpret_cast<UART_HandleTypeDef *>(obj->hal_obj));
}
bool UartRead(Uart_t *obj, uint8_t *data, size_t len, uint32_t timeout_ms) {
  return (
    HAL_UART_Receive(reinterpret_cast<UART_HandleTypeDef *>(obj->hal_obj),
                     data, len, timeout_ms) == HAL_OK);
}
bool UartWrite(Uart_t *obj, const uint8_t *data, size_t len, uint32_t timeout_ms) {
  return (
    HAL_UART_Transmit(reinterpret_cast<UART_HandleTypeDef *>(obj->hal_obj),
                     const_cast<uint8_t*>(data), len, timeout_ms) == HAL_OK);
}
void UartClearRxBuf(Uart_t *obj) {
  __HAL_UART_CLEAR_FLAG(reinterpret_cast<UART_HandleTypeDef *>(obj->hal_obj), UART_CLEAR_OREF);
  uint8_t buffer[20];
  board::UartRead(obj, buffer, 20, 2);
}

}

#ifndef APP_BOOTLOADER

[[maybe_unused]]
void DMA1_Channel4_IRQHandler() {
  HAL_DMA_IRQHandler(&board::hdma_uart1_tx);
}
[[maybe_unused]]
void DMA1_Channel7_IRQHandler() {
  HAL_DMA_IRQHandler(&board::hdma_uart2_tx);
}
static void RxByOneByte(Uart_t* uart) {
  uint32_t cr1its = READ_REG(reinterpret_cast<UART_HandleTypeDef *>(uart->hal_obj)->Instance->CR1);
  uint32_t isrflags = READ_REG(reinterpret_cast<UART_HandleTypeDef *>(uart->hal_obj)->Instance->ISR);
  if (((isrflags & USART_ISR_RXNE) != RESET)
    && ((cr1its & USART_CR1_RXNEIE) != RESET)) {
    uart->rx_byte = reinterpret_cast<UART_HandleTypeDef *>(uart->hal_obj)->Instance->RDR & (uint8_t) 0X00FF;
    if (uart->IrqNotify) {
      uart->IrqNotify(UART_NOTIFY_RX, uart->context);
    }
  }
  HAL_UART_IRQHandler(reinterpret_cast<UART_HandleTypeDef *>(uart->hal_obj));
}
[[maybe_unused]]
void USART1_IRQHandler(void) {
  RxByOneByte(&board::uart1);
}
[[maybe_unused]]
void USART2_IRQHandler(void) {
  RxByOneByte(&board::uart2);
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *handle) {
  Uart_t *uart = (handle == board::uart1.hal_obj) ? &board::uart1 : &board::uart2;
  if (uart->IrqNotify) {
    uart->IrqNotify(UART_NOTIFY_TX, uart->context);
    uart->is_writing = false;
  }
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *handle) {
  Uart_t *uart = (handle == board::uart1.hal_obj) ? &board::uart1 : &board::uart2;
  if (uart->IrqNotify) {
    uart->IrqNotify(UART_NOTIFY_RX, uart->context);
  }
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *handle) {
  Uart_t *uart = (handle == board::uart1.hal_obj) ? &board::uart1 : &board::uart2;
  if (uart->IrqNotify) {
    uart->IrqNotify(UART_NOTIFY_ERR, uart->context);
  }
}
#endif

