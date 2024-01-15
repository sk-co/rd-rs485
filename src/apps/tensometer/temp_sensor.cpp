#include "temp_sensor.h"
#include <algorithm>
#include <array>
#include "board-gpio.h"
#include "board-adc.h"
#include "board-delay.h"

namespace {

constexpr auto kStartTemp = -50;
constexpr float kResistance[] = {
  201100.0f,  // -50
  187300.0f,
  174500.0f,
  162700.0f,
  151700.0f,
  141600.0f,
  132200.0f,
  123500.0f,
  115400.0f,
  107900.0f,
  101000.0f,  // -40
  94480.0f,
  88460.0f,
  82870.0f,
  77660.0f,
  71810.0f,
  68300.0f,
  64090.0f,
  60170.0f,
  56510.0f,
  53100.0f, // -30
  49910.0f,
  46940.0f,
  44160.0f,
  41560.0f,
  39130.0f,
  36860.0f,
  34730.0f,
  32740.0f,
  30870.0f,
  29130.0f, // -20
  27490.0f,
  25950.0f,
  24510.0f,
  23160.0f,
  21890.0f,
  20700.0f,
  19580.0f,
  18520.0f,
  17530.0f,
  16600.0f, // -10
  15720.0f,
  14600.0f,
  14120.0f,
  13390.0f,
  12700.0f,
  12050.0f,
  11440.0f,
  10860.0f,
  10310.0f,
  9796.0f,  // 0
  9310.0f,
  8851.0f,
  8417.0f,
  8006.0f,
  7618.0f,
  7252.0f,
  6905.0f,
  6576.0f,
  6265.0f,
  5971.0f,  // 10
  5692.0f,
  5427.0f,
  5177.0f,
  4939.0f,
  4714.0f,
  4500.0f,
  4297.0f,
  4105.0f,
  3922.0f,
  3748.0f,  // 20
  3583.0f,
  3426.0f,
  3277.0f,
  3135.0f,
  3000.0f,
  2872.0f,
  2750.0f,
  2633.0f,
  2523.0f,
  2417.0f,  // 30
  2317.0f,
  2221.0f,
  2130.0f,
  2042.0f,
  1959.0f,
  1880.0f,
  1805.0f,
  1733.0f,
  1664.0f,
  1598.0f,  // 40
  1535.0f,
  1475.0f,
  1418.0f,
  1363.0f,
  1310.0f,
  1260.0f,
  1212.0f,
  1167.0f,
  1123.0f,
  1081.0f,  // 50
  1040.0f,
  1002.0f,
  965.0f,
  929.6f,
  895.8f,
  863.3f,
  832.2f,
  802.3f,
  773.7f,
  746.3f, // 60
  719.9f,
  694.7f,
  670.4f,
  647.1f,
  624.7f,
  603.3f,
  582.6f,
  562.8f,
  543.7f,
  525.4f, // 70
  507.8f,
  490.9f,
  474.7f,
  459.0f,
  444.0f,
  429.5f,
  415.6f,
  402.2f,
  389.3f,
  376.9f, // 80
  364.9f,
  353.4f,
  342.2f,
  331.5f,
  321.2f,
  311.3f,
  301.7f,
  292.4f,
  283.5f,
  274.9f, // 90
  266.6f,
  258.6f,
  250.9f,
  243.4f,
  236.2f,
  229.3f,
  222.6f,
  216.1f,
  209.8f,
  203.8f, // 100
  197.9f,
  192.2f,
  186.8f,
  181.5f,
  176.4f,
  171.4f,
  166.7f,
  162.0f,
  157.6f,
  153.2f, // 110
  149.0f,
  145.0f,
  141.1f,
  137.2f,
  133.6f,
  130.0f,
  126.5f,
  123.2f,
  119.9f,
  116.8f, // 120
  113.8f,
  110.8f,
  107.9f,
  105.2f,
  102.5f,
  99.9f,
  97.3f,
  94.9f,
  92.5f,
  90.2f,  // 130
  87.9f,
  85.7f,
  83.6f,
  81.6f,
  79.6f,
  77.6f,
  75.8f,
  73.9f,
  72.2f,
  70.4f,  // 140
  68.8f,
  67.1f,
  65.5f,
  64.0f,
  62.5f,
  61.1f,
  59.6f,
  58.3f,
  56.8f,
  55.6f,  // 150
};

}

namespace app {

TempSensor::TempSensor(PinNames en, PinNames en_mode_110k, PinNames en_mode_2k,
                       Adc_t *adc, AdcChannelId_t channel_id) {
  adc_ = adc;
  channel_id_ = channel_id;
  board::GpioInit(&en_pin_, en, PIN_OUTPUT, PIN_PUSH_PULL,
                  PIN_PULL_DOWN, 0);
  board::GpioInit(&en_mode_110k_pin_, en_mode_110k, PIN_OUTPUT, PIN_PUSH_PULL,
                  PIN_PULL_DOWN, 0);
  board::GpioInit(&en_mode_2k_pin_, en_mode_2k, PIN_OUTPUT, PIN_PUSH_PULL,
                  PIN_PULL_DOWN, 0);
}
int TempSensor::GetValue(int16_t *temp) {
  board::AdcConfigChannel(adc_, channel_id_);
  board::GpioWrite(&en_mode_110k_pin_, 1);
  board::GpioWrite(&en_mode_2k_pin_, 0);
  board::GpioWrite(&en_pin_, 1);
  DelayMs(1);
  board::AdcStart(adc_);
  std::array<uint16_t,7> values = {};
  for(auto &value: values){
    value = board::AdcReadWithTimeout(adc_, 10);
  }
  std::sort(values.begin(), values.end());
  uint16_t value = values[3];
  board::AdcStop(adc_);
  board::GpioWrite(&en_mode_110k_pin_, 0);
  if(value == 0xFFFF) {
    // Ошибка
    board::GpioWrite(&en_pin_, 0);
    return 1;
  }
  float voltage = float(value)*(float(board::GetVRef())/1000.0f/4096);
  if(voltage >= 0.13) {
    board::GpioWrite(&en_pin_, 0);
    *temp = GetTemperature(value, 110000);
    return 0;
  }
  // Слишком малое значение, переключаемся на режим 2.
  board::GpioWrite(&en_mode_2k_pin_, 1);
  DelayMs(1);
  board::AdcStart(adc_);
  value = board::AdcReadWithTimeout(adc_, 10);
  board::AdcStop(adc_);
  board::GpioWrite(&en_mode_2k_pin_, 0);
  if(value == 0xFFFF) {
    // Ошибка
    board::GpioWrite(&en_pin_, 0);
    return 1;
  }
  board::GpioWrite(&en_pin_, 0);
  *temp = GetTemperature(value, 2000);
  return 0;
}
int16_t TempSensor::GetTemperature(uint16_t adc_value, int r1) {
  float voltage = float(adc_value)*(float(board::GetVRef())/1000.0f/4096);
  volatile float resistance = voltage*float(r1)/(5.0f - voltage);
  constexpr auto kTableSize = sizeof(kResistance)/sizeof(kResistance[0]);
  for(uint32_t i = 0; i < kTableSize; ++i) {
    if(kResistance[i] <= resistance) {
      if(i == 0) {
        // Меньше -50.
        return kStartTemp;
      }
      else {
        if((kResistance[i - 1] - resistance) < (resistance - kResistance[i]))
          return kStartTemp + i - 1;
        else
          return kStartTemp + i;
      }
    }
  }
  // Больше 150.
  return kStartTemp + kTableSize;
}
}
