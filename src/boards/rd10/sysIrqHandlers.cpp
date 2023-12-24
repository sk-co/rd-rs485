#include "sysIrqHandlers.h"
#include "mcu.h"
#include "error-handler.h"

[[maybe_unused]]
void NMI_Handler(void) {
  IrqErrorHandler();
}
[[maybe_unused]]
void HardFault_Handler(void){
  IrqErrorHandler();
}
// Для отладки
//void HardFault_Handler(void){
//  struct Registers {
//    uint32_t r0;
//    uint32_t r1;
//    uint32_t r2;
//    uint32_t r3;
//    uint32_t r12;
//    uint32_t lr;
//    uint32_t pc;
//    uint32_t psr;
//  };
//  volatile Registers *stack_ptr;
//  asm(
//    "TST lr, #4 \n"
//    "ITE EQ \n"
//    "MRSEQ %[ptr], MSP \n"
//    "MRSNE %[ptr], PSP \n"
//    : [ptr] "=r" (stack_ptr)
//    );
//  volatile uint32_t r0 = stack_ptr->r0;
//  volatile uint32_t r1 = stack_ptr->r1;
//  volatile uint32_t r2 = stack_ptr->r2;
//  volatile uint32_t r3 = stack_ptr->r3;
//  volatile uint32_t r12 = stack_ptr->r12;
//  volatile uint32_t lr = stack_ptr->lr;
//  volatile uint32_t pc = stack_ptr->pc;
//  volatile uint32_t psr = stack_ptr->psr;
//  while (true) {
//    (void)stack_ptr;
//  }
//}


[[maybe_unused]]
void MemManage_Handler(void) {
  IrqErrorHandler();
}
[[maybe_unused]]
void BusFault_Handler(void) {
  IrqErrorHandler();
}
[[maybe_unused]]
void UsageFault_Handler(void) {
  IrqErrorHandler();
}
[[maybe_unused]]
void SVC_Handler(void) {
  IrqErrorHandler();
}
[[maybe_unused]]
void DebugMon_Handler(void) {
}
[[maybe_unused]]
void PendSV_Handler(void) {
}
[[maybe_unused]]
void SysTick_Handler(void) {
  HAL_IncTick();
}
