#include "mcu.h"
#include "board-delay.h"

static TIM_HandleTypeDef tim_mks;

void Delay(float s) {
  DelayMs(s * 1000.0f);
}
void DelayMs(uint32_t ms) {
  HAL_Delay(ms);
}
uint32_t GetSysMs() {
  return HAL_GetTick();
}
void DelayMks( uint16_t mks ) {
  __HAL_RCC_TIM16_CLK_ENABLE();
  tim_mks.Instance = TIM16;
  tim_mks.Init.Prescaler = HAL_RCC_GetHCLKFreq()/1000000-1;
  tim_mks.Init.Period = 0xFFFF;
  tim_mks.Init.CounterMode = TIM_COUNTERMODE_UP;
  tim_mks.Init.ClockDivision = 0;
  tim_mks.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&tim_mks);
  HAL_TIM_Base_Start(&tim_mks);

  __HAL_TIM_SET_COUNTER(&tim_mks, 0);
  size_t start = __HAL_TIM_GET_COUNTER(&tim_mks);
  while(__HAL_TIM_GET_COUNTER(&tim_mks) - start < mks);

  HAL_TIM_Base_Stop(&tim_mks);
  __HAL_RCC_TIM16_CLK_DISABLE();
}