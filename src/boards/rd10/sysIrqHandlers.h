/*!
 * \file      sysIrqHandlers.h
 *
 * \brief     Default IRQ handlers
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2020 Semtech
 *
 * \endcode
 */
#ifndef SYS_IRQ_HANDLERS_H
#define SYS_IRQ_HANDLERS_H

#ifdef __cplusplus
 extern "C" {
#endif

[[maybe_unused]]
void NMI_Handler( void );
[[maybe_unused]]
void HardFault_Handler( void );
[[maybe_unused]]
void MemManage_Handler( void );
[[maybe_unused]]
void BusFault_Handler( void );
[[maybe_unused]]
void UsageFault_Handler( void );
[[maybe_unused]]
void SVC_Handler(void);
[[maybe_unused]]
void DebugMon_Handler( void );
[[maybe_unused]]
void PendSV_Handler(void);
[[maybe_unused]]
void SysTick_Handler(void);

[[maybe_unused]]
void EXTI0_IRQHandler(void);
[[maybe_unused]]
void EXTI1_IRQHandler(void);
[[maybe_unused]]
void EXTI2_TSC_IRQHandler(void);
[[maybe_unused]]
void EXTI3_IRQHandler(void);
[[maybe_unused]]
void EXTI4_IRQHandler(void);
[[maybe_unused]]
void EXTI9_5_IRQHandler(void);
[[maybe_unused]]
void EXTI15_10_IRQHandler(void);

[[maybe_unused]]
void RTC_Alarm_IRQHandler( void );

[[maybe_unused]]
void DMA1_Channel1_IRQHandler(void);
[[maybe_unused]]
void DMA1_Channel4_IRQHandler(void);
[[maybe_unused]]
void DMA1_Channel7_IRQHandler(void);
[[maybe_unused]]
void DMA2_Channel3_IRQHandler(void);
[[maybe_unused]]
void ADC1_IRQHandler(void);
[[maybe_unused]]
void SDADC1_IRQHandler(void);
[[maybe_unused]]
void USART1_IRQHandler(void);
[[maybe_unused]]
void USART2_IRQHandler(void);
[[maybe_unused]]
void USART3_IRQHandler(void);
[[maybe_unused]]
void TIM19_IRQHandler(void);



#ifdef __cplusplus
}
#endif

#endif // SYS_IRQ_HANDLERS_H
