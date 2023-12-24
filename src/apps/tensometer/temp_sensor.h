#ifndef RD_RS485_SRC_APPS_TENSOMETER_TEMP_SENSOR_H_
#define RD_RS485_SRC_APPS_TENSOMETER_TEMP_SENSOR_H_

#include "gpio-def.h"
#include "adc-def.h"

namespace app {

class TempSensor {
 public:
  TempSensor(PinNames en, PinNames en_mode_110k, PinNames en_mode_2k,
             Adc_t *adc, AdcChannelId_t channel_id);
  int GetValue(int16_t *temp);

 private:
  int16_t GetTemperature(uint16_t adc_value, int r1);
  Gpio_t en_pin_ = {};
  Gpio_t en_mode_110k_pin_ = {};
  Gpio_t en_mode_2k_pin_ = {};
  Adc_t *adc_ = nullptr;
  AdcChannelId_t channel_id_ = {};
};

}

#endif //RD_RS485_SRC_APPS_TENSOMETER_TEMP_SENSOR_H_
