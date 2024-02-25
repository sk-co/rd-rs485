#include "f-impulse-generator.h"
#include "mcu.h"
#include "board-delay.h"

FImpulseGenerator::FImpulseGenerator(PinNames pin_name,
                             PinNames pin_name_out_en,
                             PinNames pin_name_in_en) {
  board::GpioInit(&pin_, pin_name, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
  board::GpioInit(&pin_out_en_, pin_name_out_en, PIN_OUTPUT, PIN_PUSH_PULL,
                  PIN_PULL_DOWN, 0);
  board::GpioInit(&pin_in_en_, pin_name_in_en, PIN_OUTPUT, PIN_PUSH_PULL,
                  PIN_PULL_DOWN, 0);
}
void FImpulseGenerator::Generate(int f_start,
                                 int f_end,
                                 int impulse_count) {
//  int f_count = f_end - f_start + 1;
//  float f_step = float(f_count) / float(impulse_count);
  board::GpioWrite(&pin_, 0);
  // Подаем питание на выход, ждем 200 мс.
  board::GpioWrite(&pin_in_en_, 0);
  board::GpioWrite(&pin_out_en_, 1);
  HAL_Delay(50);
//  TIM_HandleTypeDef tim_mks = {};
//  __HAL_RCC_TIM6_CLK_ENABLE();
//  tim_mks.Instance = TIM6;
//  tim_mks.Init.Prescaler = HAL_RCC_GetHCLKFreq()/1000000-1;
//  tim_mks.Init.Period = 0xFFFF;
//  tim_mks.Init.CounterMode = TIM_COUNTERMODE_UP;
//  tim_mks.Init.ClockDivision = 0;
//  tim_mks.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
//  HAL_TIM_Base_Init(&tim_mks);
//  HAL_TIM_Base_Start(&tim_mks);

  auto freq = float(f_start);
  int count = 0;
  while(impulse_count > count) {
    float period_mks = ((1.0f / freq) * 1000000);
    uint32_t mks = uint32_t(period_mks/2) - 23;
    DelayMks(mks);
    board::GpioToggle(&pin_);
    freq += (freq <= 920)? 1.2f: 1.6f;
    count++;
  };
  board::GpioWrite(&pin_out_en_, 0);
}
