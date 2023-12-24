#ifndef TENSOMETER_SRC_PERIPHERALS_F_IMPULS_GENERATOR_H_
#define TENSOMETER_SRC_PERIPHERALS_F_IMPULS_GENERATOR_H_

#include "board-gpio.h"

class FImpulseGenerator {
 public:
  FImpulseGenerator(PinNames pin_name, PinNames pin_name_out_en, PinNames pin_name_in_en);
  void Generate(int f_start, int f_end, int impulse_count);

 private:
  Gpio_t pin_ = {};
  Gpio_t pin_out_en_ = {};
  Gpio_t pin_in_en_ = {};
};

#endif //TENSOMETER_SRC_PERIPHERALS_F_IMPULS_GENERATOR_H_
