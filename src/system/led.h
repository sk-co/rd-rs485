//
// Created by SERG on 13.07.2021.
//

#ifndef LORAWAN_DEV_SRC_PERIPHERALS_LED_H_
#define LORAWAN_DEV_SRC_PERIPHERALS_LED_H_

#include "board-gpio.h"

class Led {
 public:
  void Init(Gpio_t *pin, bool active_level) {
    active_level_ = active_level;
    pin_ = pin;
  }
  void On() {
    if(pin_)
      board::GpioWrite(pin_, (active_level_) ? 1 : 0);
  }
  void Off() {
    if(pin_)
      board::GpioWrite(pin_, (active_level_) ? 0 : 1);
  }
  void Toggle() {
    if(pin_)
      board::GpioToggle(pin_);
  }
  void SaveState() {
    if(!pin_)
      return;
    state_ = board::GpioRead(pin_);
  }
  void ResumeState() {
    if(!pin_)
      return;
    board::GpioWrite(pin_, state_);
  }
 private:
  Gpio_t *pin_ = nullptr;
  bool active_level_ = true;
  uint32_t state_ = false;
};

#endif //LORAWAN_DEV_SRC_PERIPHERALS_LED_H_
