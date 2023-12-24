#ifndef LORAWAN_DEV_SRC_BOARDS_RAK811_BOARD_PERIPHERALS_H_
#define LORAWAN_DEV_SRC_BOARDS_RAK811_BOARD_PERIPHERALS_H_

#include "board.h"
#include "board-config.h"
#include "board-uart.h"
#include "gpio-def.h"
#include "board-spi.h"
#include "led.h"
#include "board-flash.h"
#include "board-adc.h"

namespace board {

extern Uart_t uart1;
extern Uart_t uart2;

extern Spi_t spi1;

extern Led led1;
extern Led led2;
extern Led led3;

extern Adc_t adc1;
extern Adc_t sdadc1;
extern AdcChannelId_t bat_channel;
extern AdcChannelId_t temp_channel;

// Возвращает UART для загрузки прошивки/конфигурации.
Uart_t* GetConfigUart();
// Возвращает UART для отладочного вывода.
Uart_t* GetLogUart();

}


#endif //LORAWAN_DEV_SRC_BOARDS_RAK811_BOARD_PERIPHERALS_H_
