//
// Created by Serg on 07.08.2021.
//

#ifndef LORAWAN_DEV_SRC_BOARDS_BOARD_PWM_H_
#define LORAWAN_DEV_SRC_BOARDS_BOARD_PWM_H_

#include <cstdint>
#include "gpio-def.h"

namespace board {

struct Pwm {
  Gpio_t pin;
  void *tim_obj;
  uint32_t channel;
  uint16_t period;
};

// Инициализация pwm.
void PwmInit(Pwm *obj, PinNames pin, uint16_t period);
// Установка ширины импульса.
void SetPwmWidth(Pwm *obj, uint16_t width);

}

#endif //LORAWAN_DEV_SRC_BOARDS_BOARD_PWM_H_
