//
// Created by SERG on 20.02.2021.
//
#include "board-critical-section.h"
#include "mcu.h"

void BoardCriticalSectionBegin(uint32_t *mask) {
  *mask = __get_PRIMASK();
  __disable_irq();
}

void BoardCriticalSectionEnd(uint32_t *mask) {
  __set_PRIMASK(*mask);
}
