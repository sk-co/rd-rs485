//
// Created by SERG on 28.06.2022.
//

#ifndef LORAWAN_DI15_SRC_PERIPHERALS_BATTERY_H_
#define LORAWAN_DI15_SRC_PERIPHERALS_BATTERY_H_

#include "adc-def.h"

namespace sensor {

// Класс для контроля уровня заряда батареи через ацп.
class Battery {
 public:
  Battery(Adc_t *adc, AdcChannelId_t channel_id, uint32_t count, float koeff);
  int GetVoltage(uint32_t *voltage_mv);

 private:
  Adc_t *adc_ = nullptr;
  AdcChannelId_t channel_id_ = {};
  const uint32_t kCount_ = {};
  float koeff_ = 1;
};

}


#endif //LORAWAN_DI15_SRC_PERIPHERALS_BATTERY_H_
