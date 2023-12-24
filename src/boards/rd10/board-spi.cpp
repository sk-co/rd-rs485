#include "board-spi.h"
#include "mcu.h"
#include "utilities.h"
#include "board.h"
#include "board-gpio.h"
#include "board-critical-section.h"
#include "error-handler.h"

#define SPI_MAX_TIMEOUT 1000

namespace board {

SPI_HandleTypeDef SpiHandle_1;
Spi_t spi1;

void SpiInit(Spi_t *obj, SpiId_t spiId, PinNames mosi,
              PinNames miso, PinNames sclk, PinNames nss) {
  CRITICAL_SECTION_BEGIN();
  obj->SpiId = spiId;
  obj->hal_obj = nullptr;
  uint32_t gpio_alt_fn = GPIO_AF5_SPI1;
  switch (obj->SpiId) {
    case SPI_1:
      __HAL_RCC_SPI1_FORCE_RESET();
      __HAL_RCC_SPI1_RELEASE_RESET();
      __HAL_RCC_SPI1_CLK_ENABLE();
      break;
    default:
      ErrorHandler(__FUNCTION__);
  }
  board::GpioInit(&obj->Mosi,mosi,PIN_ALTERNATE_FCT,PIN_PUSH_PULL,
                  PIN_NO_PULL,gpio_alt_fn);
  GpioInit(&obj->Miso,miso,PIN_ALTERNATE_FCT,PIN_PUSH_PULL,
           PIN_NO_PULL,gpio_alt_fn);
  GpioInit(&obj->Sclk,sclk,PIN_ALTERNATE_FCT,PIN_PUSH_PULL,
           PIN_NO_PULL,gpio_alt_fn);
  GpioInit(&obj->Nss,nss,PIN_ALTERNATE_FCT,PIN_PUSH_PULL,
           PIN_PULL_UP,gpio_alt_fn);
  CRITICAL_SECTION_END();
}

void SpiDeInit(Spi_t *obj) {
  HAL_SPI_DeInit(reinterpret_cast<SPI_HandleTypeDef*>(obj->hal_obj));
  __HAL_RCC_SPI1_CLK_DISABLE();
  // TODO: Внимание, сделал для снижения потребления.
//  GpioInit(&obj->Mosi, obj->Mosi.pin, PIN_OUTPUT,
//           PIN_PUSH_PULL,PIN_NO_PULL,0);
//  GpioInit(&obj->Miso, obj->Miso.pin,PIN_OUTPUT,
//           PIN_PUSH_PULL,PIN_PULL_DOWN,0);
//  GpioInit(&obj->Sclk, obj->Sclk.pin,PIN_OUTPUT,
//           PIN_PUSH_PULL,PIN_NO_PULL,0);
//  GpioInit(&obj->Nss, obj->Nss.pin,PIN_OUTPUT,
//           PIN_PUSH_PULL,PIN_PULL_UP,1);
  GpioDeInit(&obj->Mosi);
  GpioDeInit(&obj->Miso);
  GpioDeInit(&obj->Sclk);
  GpioDeInit(&obj->Nss);
}
void SpiConfig(Spi_t *obj, uint32_t hz, int32_t bits, int32_t cpol, int32_t cpha, int8_t slave) {
  SPI_HandleTypeDef *spiHandle = nullptr;
  switch (obj->SpiId) {
    case SPI_1:
      spiHandle = &SpiHandle_1;
      spiHandle->Instance = SPI1;
      break;
    default:
      assert_param(MCU_FAIL);
      return;
  }
  spiHandle->Init.Direction = SPI_DIRECTION_2LINES;
  if (bits == SPI_DATASIZE_8BIT) {
    spiHandle->Init.DataSize = SPI_DATASIZE_8BIT;
  } else {
    spiHandle->Init.DataSize = SPI_DATASIZE_16BIT;
  }
  spiHandle->Init.CLKPolarity = cpol;
  spiHandle->Init.CLKPhase = cpha;
  spiHandle->Init.FirstBit = SPI_FIRSTBIT_MSB;
  spiHandle->Init.TIMode = SPI_TIMODE_DISABLE;
  spiHandle->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  spiHandle->Init.CRCPolynomial = 7;
  if (slave == 0) {
    spiHandle->Init.Mode = SPI_MODE_MASTER;
  } else {
    spiHandle->Init.Mode = SPI_MODE_SLAVE;
  }
  if(obj->Nss.pin == NC) {
    spiHandle->Init.NSS = SPI_NSS_SOFT;
  }

  uint32_t divisor = 0;
  uint32_t sysClkTmp = SystemCoreClock;
  uint32_t baudRate;
  while (sysClkTmp > hz) {
    divisor++;
    sysClkTmp = (sysClkTmp >> 1);
    if (divisor >= 7) {
      break;
    }
  }
  baudRate = (((divisor & 0x4) == 0) ? 0x0 : SPI_CR1_BR_2) |
    (((divisor & 0x2) == 0) ? 0x0 : SPI_CR1_BR_1) |
    (((divisor & 0x1) == 0) ? 0x0 : SPI_CR1_BR_0);
  spiHandle->Init.BaudRatePrescaler = baudRate;
  if (HAL_SPI_Init(spiHandle) != HAL_OK) {
    ErrorHandler(nullptr);
  }
  obj->hal_obj = spiHandle;
}
uint16_t SpiInOut(Spi_t *obj, uint16_t outData) {
  uint8_t rxData = 0;
  uint32_t timeout_counter = 0;
  auto spiHandle = reinterpret_cast<SPI_HandleTypeDef*> (obj->hal_obj);
  SET_BIT(spiHandle->Instance->CR2, SPI_RXFIFO_THRESHOLD);
  if ((spiHandle->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
    __HAL_SPI_ENABLE(spiHandle);
  CRITICAL_SECTION_BEGIN();
  while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_TXE ) == RESET ){
    if(++timeout_counter > SPI_MAX_TIMEOUT)
      goto ERROR_LABEL;
  }
  *(__IO uint8_t *)&spiHandle->Instance->DR = uint8_t(outData & 0xFF);
  while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_RXNE ) == RESET ){
    if(++timeout_counter > SPI_MAX_TIMEOUT)
      goto ERROR_LABEL;
  }
  rxData = *(__IO uint8_t *)&spiHandle->Instance->DR;
  while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_BSY ) != RESET){
    if(++timeout_counter > SPI_MAX_TIMEOUT)
      goto ERROR_LABEL;
  }
  while( (spiHandle->Instance->SR & SPI_FLAG_FRLVL ) != 0){
    READ_REG(*((__IO uint8_t *)&spiHandle->Instance->DR));
    if(++timeout_counter > SPI_MAX_TIMEOUT)
      goto ERROR_LABEL;
  }
//  HAL_SPI_TransmitReceive( spiHandle, ( uint8_t* )&outData, &rxData, 1, HAL_MAX_DELAY );
  CRITICAL_SECTION_END();
  return (rxData);
  ERROR_LABEL:
  CRITICAL_SECTION_END();
  __HAL_SPI_DISABLE(spiHandle);
  return 0;
}
int SpiInOut(Spi_t *obj, const uint8_t* tx_data, uint8_t* rx_data, uint32_t size) {
  auto spiHandle = reinterpret_cast<SPI_HandleTypeDef*> (obj->hal_obj);
  uint32_t timeout_counter = 0;
//  volatile int err = 0;
  SET_BIT(spiHandle->Instance->CR2, SPI_RXFIFO_THRESHOLD);
  if ((spiHandle->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
    __HAL_SPI_ENABLE(spiHandle);
  for(uint32_t i=0; i < size; ++i) {
//    rx_data[i] = SpiInOut(obj, tx_data[i]);
    while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_TXE ) == RESET) {
      if(++timeout_counter > SPI_MAX_TIMEOUT) {
//        err = 1;
        goto ERROR_LABEL;
      }
    }
    *(__IO uint8_t*)&spiHandle->Instance->DR = tx_data[i];
    while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_RXNE ) == RESET ) {
      if(++timeout_counter > SPI_MAX_TIMEOUT) {
//        err = 2;
        goto ERROR_LABEL;
      }
    }
    rx_data[i] = *(__IO uint8_t*)&spiHandle->Instance->DR;
    timeout_counter = 0;
  }
  while( (spiHandle->Instance->SR & SPI_FLAG_FTLVL ) != 0){
    if(++timeout_counter > SPI_MAX_TIMEOUT) {
//      err = 3;
      goto ERROR_LABEL;
    }
  }
  while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_BSY ) != RESET){
    if(++timeout_counter > SPI_MAX_TIMEOUT) {
//      err = 4;
      goto ERROR_LABEL;
    }
  }
  while( (spiHandle->Instance->SR & SPI_FLAG_FRLVL ) != 0){
    READ_REG(*((__IO uint8_t *)&spiHandle->Instance->DR));
    if(++timeout_counter > SPI_MAX_TIMEOUT) {
//      err = 5;
      goto ERROR_LABEL;
    }
  }
  return 1;
  ERROR_LABEL:
  __HAL_SPI_DISABLE(spiHandle);
  return 0;
//  return (HAL_SPI_TransmitReceive( spiHandle, const_cast<uint8_t*>(tx_data),
//                              rx_data, size, 100 ) == HAL_OK);
}
int SpiIn(Spi_t *obj, uint8_t* rx_data, uint32_t size) {
//  auto spiHandle = reinterpret_cast<SPI_HandleTypeDef*> (obj->hal_obj);
  return SpiInOut(obj, rx_data, rx_data, size);
//  uint32_t timeout_counter = 0;
//  CLEAR_BIT(spiHandle->Instance->CR2, SPI_RXFIFO_THRESHOLD);
//  if ((spiHandle->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
//    __HAL_SPI_ENABLE(spiHandle);
//  uint32_t loops = size/2;
//  for(uint32_t i=0; i < loops; ++i) {
//    while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_TXE ) == RESET) {
//      if(++timeout_counter > SPI_MAX_TIMEOUT)
//        goto ERROR_LABEL;
//    }
//    spiHandle->Instance->DR = 0;
//    while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_RXNE ) == RESET) {
//      if(++timeout_counter > SPI_MAX_TIMEOUT)
//        goto ERROR_LABEL;
//    }
//    *(reinterpret_cast<uint16_t*>(rx_data) + i) = spiHandle->Instance->DR;
//    timeout_counter = 0;
//  }
//  if(size - loops*2) {
//    SET_BIT(spiHandle->Instance->CR2, SPI_RXFIFO_THRESHOLD);
//    while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_TXE ) == RESET ){
//      if(++timeout_counter > SPI_MAX_TIMEOUT)
//        goto ERROR_LABEL;
//    }
//    *(__IO uint8_t *)&spiHandle->Instance->DR = uint8_t(0);
//    while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_RXNE ) == RESET ){
//      if(++timeout_counter > SPI_MAX_TIMEOUT)
//        goto ERROR_LABEL;
//    }
//    rx_data[size-1] = *(__IO uint8_t *)&spiHandle->Instance->DR;
//    timeout_counter = 0;
//  }
//  while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_BSY ) != RESET){
//    if(++timeout_counter > SPI_MAX_TIMEOUT)
//      goto ERROR_LABEL;
//  }
//  return 1;
//  ERROR_LABEL:
//  __HAL_SPI_DISABLE(spiHandle);
//  return 0;
//  return (HAL_SPI_Receive( spiHandle, rx_data, size, 100 ) == HAL_OK);
}
int SpiOut(Spi_t *obj, const uint8_t* tx_data, uint32_t size) {
  auto spiHandle = reinterpret_cast<SPI_HandleTypeDef*> (obj->hal_obj);
  volatile int err = 0;
  CLEAR_BIT(spiHandle->Instance->CR2, SPI_RXFIFO_THRESHOLD);
  uint32_t timeout_counter = 0;
  if ((spiHandle->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
    __HAL_SPI_ENABLE(spiHandle);
  uint32_t loops = size/2;
  volatile uint32_t i =0;
//  volatile uint32_t sr = 0;
  for(i=0; i < loops; ++i) {
//    while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_TXE ) == RESET){
    while( (spiHandle->Instance->SR & SPI_FLAG_TXE) == 0 ){
      if(++timeout_counter > SPI_MAX_TIMEOUT) {
//        sr = spiHandle->Instance->SR;
        err = 1;
        goto ERROR_LABEL;
      }
    }
    timeout_counter = 0;
    spiHandle->Instance->DR = *(reinterpret_cast<const uint16_t*>(tx_data) + i);
  }
  if(size - loops*2) {
    while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_TXE ) == RESET){
      if(++timeout_counter > SPI_MAX_TIMEOUT) {
        err = 2;
        goto ERROR_LABEL;
      }
    }
    *((__IO uint8_t *)&spiHandle->Instance->DR) = tx_data[size-1];
  }
  while( (spiHandle->Instance->SR & SPI_FLAG_FTLVL ) != 0){
    if(++timeout_counter > SPI_MAX_TIMEOUT) {
      err = 3;
      goto ERROR_LABEL;
    }
  }
  while( __HAL_SPI_GET_FLAG( spiHandle, SPI_FLAG_BSY ) != RESET){
    if(++timeout_counter > SPI_MAX_TIMEOUT) {
      err = 4;
      goto ERROR_LABEL;
    }
  }
  while( (spiHandle->Instance->SR & SPI_FLAG_FRLVL ) != 0){
    READ_REG(*((__IO uint8_t *)&spiHandle->Instance->DR));
    if(++timeout_counter > SPI_MAX_TIMEOUT) {
      err = 5;
      goto ERROR_LABEL;
    }
  }
  __HAL_SPI_CLEAR_OVRFLAG(spiHandle);
  return 1;
  ERROR_LABEL:
  __HAL_SPI_DISABLE(spiHandle);
  return 0;
//  return (HAL_SPI_Transmit( spiHandle, const_cast<uint8_t*>(tx_data),
//                            size, 100 ) == HAL_OK);
}
}

