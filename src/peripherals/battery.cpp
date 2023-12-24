//
// Created by SERG on 28.06.2022.
//

#include "battery.h"
#include <cstdlib>
#include "board-adc.h"
#include "board-delay.h"

namespace {

int cmp(const void* a, const void* b) {
  auto va = *static_cast<const uint32_t*>(a);
  auto vb = *static_cast<const uint32_t*>(b);
  if(va < vb) return -1;
  if(va > vb) return 1;
  return 0;
}

}

namespace sensor {

Battery::Battery(Adc_t *adc, AdcChannelId_t channel_id, uint32_t count, float koeff)
  : adc_(adc), channel_id_(channel_id), kCount_(count), koeff_(koeff) {
}
int Battery::GetVoltage(uint32_t *voltage_mv) {
  if(!adc_)
    return 0;
  auto reference_voltage = board::DefineVRef();

  board::AdcConfigChannel(adc_, channel_id_);
  board::AdcStart(adc_);
  uint32_t values[64] = {};
  for (uint32_t i = 0; i < kCount_; ++i) {
    values[i] = board::AdcRead(adc_, channel_id_);
    DelayMs(1);
  }
  qsort(values, kCount_, sizeof(uint32_t), cmp);
  board::AdcStop(adc_);
  *voltage_mv = values[int(kCount_/2u)]*reference_voltage/4096*koeff_;
  return 0;
}
}
