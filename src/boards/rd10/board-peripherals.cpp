//
// Created by SERG on 16.07.2021.
//
#include "board-peripherals.h"
#include "board.h"

namespace board {

Uart_t* GetConfigUart() {
  if(!uart1.hal_obj) {
    board::InitUart1();
  }
  return &uart1;
}
Uart_t* GetLogUart() {
  if(!uart1.hal_obj) {
    board::InitUart1();
  }
  return &uart1;
}
}
