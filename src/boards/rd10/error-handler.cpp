//
// Created by SERG on 17.02.2021.
//

#include "error-handler.h"
#include "log.h"
#include "mcu.h"

void ErrorHandler(const char *str) {
#ifndef APP_BOOTLOADER
  if(str) {
    LOG_ERR("\nERROR FAULT: %s\n", str);
  }
  while(true) {
    LOG_FLUSH();
    HAL_Delay(100);
  }
#else
  while(true) {
    HAL_NVIC_SystemReset();
  }
#endif
}

void IrqErrorHandler() {
  // Где-то 2 минуты в релизе.
  for(uint32_t i=0; i < 0xFFFFFFF; ++i) {
    __NOP();
  }
  HAL_NVIC_SystemReset();
}