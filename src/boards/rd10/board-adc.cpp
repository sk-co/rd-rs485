#include "board-adc.h"
#include <cstdlib>
#include "mcu.h"
#include "board-gpio.h"
#include "board-config.h"
#include "sysIrqHandlers.h"
#include "board-critical-section.h"
#include "board-delay.h"

namespace board {

ADC_HandleTypeDef adc_hal;
SDADC_HandleTypeDef sdadc_hal;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_sdadc1;
TIM_HandleTypeDef htim;
TIM_HandleTypeDef sdhtim;
Adc_t adc1;
Adc_t sdadc1;
AdcChannelId_t tens_channel = ADC_CN_1;
AdcChannelId_t temp_channel = ADC_CN_2;
uint32_t vref = 3300;
constexpr uint32_t kInternalVref = 1220;
uint16_t adc_value = 0;
bool adc_value_ready = false;
uint32_t start_time_ = 0;

__IO uint32_t InjChannel = 0;

int AdcInit(Adc_t *obj) {
  if(obj == &adc1) {
    obj->hal_obj = nullptr;
    __HAL_RCC_ADC1_CLK_DISABLE();
    __HAL_RCC_ADC1_FORCE_RESET();
    __HAL_RCC_ADC1_RELEASE_RESET();
    __HAL_RCC_ADC1_CLK_ENABLE();
    adc_hal.Instance = ADC1;
    adc_hal.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    adc_hal.Init.ScanConvMode = ADC_SCAN_DISABLE;
    adc_hal.Init.ContinuousConvMode = DISABLE;
    adc_hal.Init.NbrOfConversion = 1;
    adc_hal.Init.DiscontinuousConvMode = DISABLE;
    adc_hal.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T3_TRGO;
    if (HAL_ADC_Init(&adc_hal) != HAL_OK)
      return 1;
    __HAL_RCC_DMA1_CLK_ENABLE();
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
      return 2;
    __HAL_LINKDMA(&adc_hal, DMA_Handle, hdma_adc1);
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    obj->resolution = 12;
    obj->hal_obj = &adc_hal;
    HAL_ADCEx_Calibration_Start(&adc_hal);
  }
  if(obj == &sdadc1) {
    obj->hal_obj = nullptr;
    __HAL_RCC_SDADC1_CLK_DISABLE();
    __HAL_RCC_SDADC1_FORCE_RESET();
    __HAL_RCC_SDADC1_RELEASE_RESET();
    __HAL_RCC_SDADC1_CLK_ENABLE();
    HAL_PWREx_EnableSDADC(PWR_SDADC_ANALOG1);
    sdadc_hal.Instance = SDADC1;
    sdadc_hal.Init.IdleLowPowerMode = SDADC_LOWPOWER_NONE;
    sdadc_hal.Init.FastConversionMode = SDADC_FAST_CONV_ENABLE;
    sdadc_hal.Init.SlowClockMode = SDADC_SLOW_CLOCK_DISABLE;
    sdadc_hal.Init.ReferenceVoltage = SDADC_VREF_VDDA;
    sdadc_hal.InjectedTrigger = SDADC_SOFTWARE_TRIGGER;
    if (HAL_SDADC_Init(&sdadc_hal) != HAL_OK)
      return 1;
    HAL_NVIC_SetPriority(SDADC1_IRQn, 0x01, 0);
    HAL_NVIC_EnableIRQ(SDADC1_IRQn);
    obj->resolution = 16;
    obj->hal_obj = &sdadc_hal;
  }
  return 0;
}
void AdcDeinit( Adc_t *obj ) {
  if(obj == &adc1 && obj->hal_obj) {
    HAL_ADC_DeInit(reinterpret_cast<ADC_HandleTypeDef *>(obj->hal_obj));
    __HAL_RCC_ADC1_FORCE_RESET();
    __HAL_RCC_ADC1_RELEASE_RESET();
    __HAL_RCC_ADC1_CLK_DISABLE();
    obj->hal_obj = nullptr;
  }
  if(obj == &sdadc1 && obj->hal_obj) {
    HAL_NVIC_DisableIRQ(SDADC1_IRQn);
    HAL_PWREx_DisableSDADC(PWR_SDADC_ANALOG1);
    HAL_SDADC_DeInit(reinterpret_cast<SDADC_HandleTypeDef *>(obj->hal_obj));
    __HAL_RCC_SDADC1_FORCE_RESET();
    __HAL_RCC_SDADC1_RELEASE_RESET();
    __HAL_RCC_SDADC1_CLK_DISABLE();
    obj->hal_obj = nullptr;
  }
}
int AdcConfigChannel(Adc_t *obj, AdcChannelId_t channel_id, PinNames pin, uint32_t channel,
                     ChannelConfig_t &config) {
  if(!obj->hal_obj)
    return 1;
  AdcChannel_t *adc_channel = nullptr;
  switch (channel_id) {
    case ADC_CN_1:adc_channel = &obj->channel_1;
      break;
    case ADC_CN_2:adc_channel = &obj->channel_2;
      break;
    default:return 1;
  }
  adc_channel->channel = channel;
  if(obj == &adc1) {
    GpioInit(&adc_channel->pin, pin, PIN_ANALOGIC, PIN_OPEN_DRAIN,
             PIN_NO_PULL, 0);
    ADC_ChannelConfTypeDef sConfig{
      .Channel = adc_channel->channel,
      .Rank = ADC_REGULAR_RANK_1,
      .SamplingTime = config.sample_time,
    };
    if ((HAL_ADC_ConfigChannel(reinterpret_cast<ADC_HandleTypeDef *>(obj->hal_obj), &sConfig)
      != HAL_OK))
      return 2;
  }
  if(obj == &sdadc1) {
    GpioInit(&adc_channel->pin, pin, PIN_ANALOGIC, PIN_OPEN_DRAIN,
             PIN_NO_PULL, 0);
    SDADC_ConfParamTypeDef ConfParamStruct = {};
    ConfParamStruct.InputMode = SDADC_INPUT_MODE_SE_ZERO_REFERENCE;
    ConfParamStruct.Gain = SDADC_GAIN_1;
    ConfParamStruct.CommonMode = SDADC_COMMON_MODE_VSSA;
    ConfParamStruct.Offset = 0;
    if (HAL_SDADC_PrepareChannelConfig(&sdadc_hal, SDADC_CONF_INDEX_0, &ConfParamStruct) != HAL_OK)
      return 1;
    if (HAL_SDADC_AssociateChannelConfig(&sdadc_hal, channel, SDADC_CONF_INDEX_0) != HAL_OK)
      return 2;
    if (HAL_SDADC_InjectedConfigChannel(&sdadc_hal, channel, SDADC_CONTINUOUS_CONV_ON) != HAL_OK)
      return 3;
  }
  return 0;
}
int AdcConfigChannel(Adc_t *obj, AdcChannelId_t channel_id) {
  if(obj == &adc1) {
    switch (channel_id) {
      case ADC_CN_1: {
        ChannelConfig_t channel_config = {
          .sample_time = ADC_SAMPLETIME_28CYCLES_5
        };
        obj->period_mks = 1000000 / 50000;
        return AdcConfigChannel(&adc1, ADC_CN_1, ADC1_1_PIN, ADC1_1_CHANNEL, channel_config);
      }
      case ADC_CN_2: {
        ChannelConfig_t channel_config = {
          .sample_time = ADC_SAMPLETIME_239CYCLES_5
        };
        obj->period_mks = 1000000 / 32000;
        return AdcConfigChannel(&adc1, ADC_CN_2, ADC1_2_PIN, ADC1_2_CHANNEL, channel_config);
      }
      default:return 1;
    }
  }
  if(obj == &sdadc1) {
    ChannelConfig_t channel_config = {};
    obj->period_mks = 1000000 / 50000;
    return AdcConfigChannel(&sdadc1, ADC_CN_1, SDADC_1_PIN, SDADC_1_CHANNEL, channel_config);
  }
  return 0;
}
void AdcSleep( Adc_t *obj ) {
  __HAL_RCC_ADC1_CLK_DISABLE();
}
void AdcWakeup( Adc_t *obj ) {
  __HAL_RCC_ADC1_CLK_ENABLE();
}

uint32_t AdcStart(Adc_t *obj){
  adc_value_ready = false;
  if(obj == &adc1) {
    // Все каналы запускаются по таймеру.
    __HAL_RCC_TIM3_CLK_ENABLE();
    htim.Instance = TIM3;
    htim.Init.Prescaler = HAL_RCC_GetHCLKFreq() / 1000000 - 1; // 1 микросекунда
    htim.Init.Period = obj->period_mks - 1;
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim) != HAL_OK) {
      return false;
    }
    __HAL_TIM_CLEAR_FLAG(&htim, TIM_SR_UIF);
    TIM_ClockConfigTypeDef sClockSourceConfig{};
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim, &sClockSourceConfig) != HAL_OK) {
      return false;
    }
    TIM_MasterConfigTypeDef sMasterConfig{};
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig) != HAL_OK) {
      return false;
    }
    HAL_ADCEx_Calibration_Start(&adc_hal);
    HAL_TIM_Base_Start(&htim);
    HAL_ADC_Start_DMA(reinterpret_cast<ADC_HandleTypeDef *>(obj->hal_obj),
                      (uint32_t *) &adc_value,
                      1);
  }
  if(obj == &sdadc1) {
//    // По внешнему таймеру.
//    __HAL_RCC_TIM19_CLK_ENABLE();
//    sdhtim.Instance = TIM19;
//    sdhtim.Init.ClockDivision = 0;
//    sdhtim.Init.CounterMode = TIM_COUNTERMODE_UP;
//    sdhtim.Init.Prescaler = HAL_RCC_GetPCLK1Freq() / 1000000 - 1; // 1 микросекунда
//    sdhtim.Init.Period = obj->period_mks - 1;
//    sdhtim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
//    HAL_TIM_PWM_Init(&sdhtim);
////    __HAL_TIM_CLEAR_FLAG(&sdhtim, TIM_SR_UIF);
//    TIM_ClockConfigTypeDef sClockSourceConfig {};
//    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
//    if (HAL_TIM_ConfigClockSource(&sdhtim, &sClockSourceConfig) != HAL_OK)
//      return false;
//    TIM_OC_InitTypeDef tim_oc = {};
//    tim_oc.OCMode = TIM_OCMODE_PWM1;
//    tim_oc.Pulse = sdhtim.Init.Period-1;
//    tim_oc.OCPolarity = TIM_OCPOLARITY_HIGH;
//    tim_oc.OCFastMode = TIM_OCFAST_DISABLE;
//    HAL_TIM_PWM_ConfigChannel(&sdhtim, &tim_oc, TIM_CHANNEL_2);
//    if (HAL_SDADC_SelectInjectedTrigger(&sdadc_hal, SDADC_EXTERNAL_TRIGGER) != HAL_OK)
//      return 4;
//    if (HAL_SDADC_SelectInjectedExtTrigger(&sdadc_hal, SDADC_EXT_TRIG_TIM19_CC2,
//                                           SDADC_EXT_TRIG_RISING_EDGE) != HAL_OK)
//      return false;
//    if (HAL_SDADC_CalibrationStart(&sdadc_hal, SDADC_CALIBRATION_SEQ_1) != HAL_OK)
//      return false;
//    // TODO: Калибровка может ожидаться бесконечно.
//    if (HAL_SDADC_PollForCalibEvent(&sdadc_hal, HAL_MAX_DELAY) != HAL_OK)
//      return false;
//    if (HAL_SDADC_InjectedStart_IT(&sdadc_hal) != HAL_OK)
//      return false;
//    if (HAL_TIM_PWM_Start(&sdhtim, TIM_CHANNEL_2) != HAL_OK)
//      return false;

    // По прерыванию таймера.
    __HAL_RCC_TIM19_CLK_ENABLE();
    sdhtim.Instance = TIM19;
    sdhtim.Init.ClockDivision = 0;
    sdhtim.Init.CounterMode = TIM_COUNTERMODE_UP;
    sdhtim.Init.Prescaler = HAL_RCC_GetHCLKFreq() / 1000000 - 1; // 1 микросекунда
    sdhtim.Init.Period = obj->period_mks - 1;
    sdhtim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&sdhtim);
    __HAL_TIM_CLEAR_FLAG(&sdhtim, TIM_SR_UIF);
    TIM_ClockConfigTypeDef sClockSourceConfig {};
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&sdhtim, &sClockSourceConfig) != HAL_OK)
      return false;

    if (HAL_SDADC_SelectInjectedTrigger(&sdadc_hal, SDADC_SOFTWARE_TRIGGER) != HAL_OK)
      return 4;
    if (HAL_SDADC_CalibrationStart(&sdadc_hal, SDADC_CALIBRATION_SEQ_1) != HAL_OK)
      return 5;
    // TODO: Калибровка может ожидаться бесконечно.
    if (HAL_SDADC_PollForCalibEvent(&sdadc_hal, HAL_MAX_DELAY) != HAL_OK)
      return 6;
//    if (HAL_SDADC_InjectedStart_IT(&sdadc_hal) != HAL_OK)
//      return false;
    HAL_NVIC_SetPriority(TIM19_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM19_IRQn);
    if (HAL_TIM_Base_Start_IT(&sdhtim) != HAL_OK)
      return 7;
  }
  return 0;
}
uint32_t AdcStartDma(Adc_t *obj, uint16_t *data, uint32_t len) {
  if(obj->hal_obj == &sdadc_hal) {
    __HAL_RCC_DMA2_CLK_ENABLE();
    hdma_sdadc1.Instance = DMA2_Channel3;
    hdma_sdadc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_sdadc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdadc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdadc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_sdadc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_sdadc1.Init.Mode = DMA_NORMAL;
    hdma_sdadc1.Init.Priority = DMA_PRIORITY_MEDIUM;
    if (HAL_DMA_Init(&hdma_sdadc1) != HAL_OK)
      return 1;
    __HAL_LINKDMA(&sdadc_hal,hdma,hdma_sdadc1);
    HAL_NVIC_SetPriority(DMA2_Channel3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Channel3_IRQn);
    HAL_NVIC_SetPriority(SDADC1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SDADC1_IRQn);
    if (HAL_SDADC_SelectInjectedDelay(&sdadc_hal, SDADC_INJECTED_DELAY_NONE) != HAL_OK)
      return 2;
    if (HAL_SDADC_SelectInjectedTrigger(&sdadc_hal, SDADC_SOFTWARE_TRIGGER) != HAL_OK)
      return 4;
    if (HAL_SDADC_CalibrationStart(&sdadc_hal, SDADC_CALIBRATION_SEQ_1) != HAL_OK)
      return 5;
    // TODO: Калибровка может ожидаться бесконечно.
    if (HAL_SDADC_PollForCalibEvent(&sdadc_hal, HAL_MAX_DELAY) != HAL_OK)
      return 6;
    board::adc_value_ready = false;
    start_time_ = GetSysMs();
    if (HAL_SDADC_InjectedStart_DMA(&sdadc_hal, reinterpret_cast<uint32_t*>(data), len) != HAL_OK)
      return 7;
  }
  return 0;
}
uint32_t AdcStop(Adc_t *obj){
  if(obj == &adc1) {
    HAL_TIM_Base_Stop(&htim);
    HAL_ADC_Stop_DMA(reinterpret_cast<ADC_HandleTypeDef *>(obj->hal_obj));
  }
  if(obj == &sdadc1){
//    HAL_TIM_PWM_Stop(&sdhtim, TIM_CHANNEL_2);
//    HAL_SDADC_InjectedStop_IT(&sdadc_hal);
//    HAL_TIM_Base_Stop_IT(&sdhtim);
    HAL_SDADC_InjectedStop(&sdadc_hal);
//    HAL_SDADC_InjectedStop_DMA(&sdadc_hal);
  }
  return 0;
}
uint32_t AdcRead(Adc_t *obj, AdcChannelId_t channel_id) {
  if(obj->hal_obj == nullptr)
    return 0;
  HAL_ADC_Start(reinterpret_cast<ADC_HandleTypeDef*>(obj->hal_obj));
  HAL_ADC_PollForConversion(reinterpret_cast<ADC_HandleTypeDef*>(obj->hal_obj), 10);
  auto value = HAL_ADC_GetValue(reinterpret_cast<ADC_HandleTypeDef*>(obj->hal_obj));
  HAL_ADC_Stop(reinterpret_cast<ADC_HandleTypeDef*>(obj->hal_obj));
  return value;
}
uint16_t AdcReadWithTimeout(Adc_t *obj, uint32_t timeout_ms) {
  if(obj->hal_obj == nullptr)
    return 0;
  auto end = HAL_GetTick() + timeout_ms;
  while(!adc_value_ready) {
    if(HAL_GetTick() > end)
      return 0xFFFF;
  }
  CRITICAL_SECTION_BEGIN();
  adc_value_ready = false;
  CRITICAL_SECTION_END();
  return adc_value;
}


float GetVRef() {
  return vref;
}
namespace {

int cmp(const void* a, const void* b) {
  auto va = *static_cast<const uint8_t*>(a);
  auto vb = *static_cast<const uint8_t*>(b);
  if(va < vb) return -1;
  if(va > vb) return 1;
  return 0;
}

}
int DefineVRef() {
  HAL_ADCEx_Calibration_Start(&adc_hal);//
  ADC_ChannelConfTypeDef sConfig{
    .Channel = ADC_CHANNEL_VREFINT,
    .Rank = ADC_REGULAR_RANK_1,
    .SamplingTime = ADC_SAMPLETIME_55CYCLES_5,
  };
  if((HAL_ADC_ConfigChannel(&adc_hal, &sConfig) != HAL_OK))
    return 0;
  board::AdcStart(&adc1);
  volatile uint16_t value = board::AdcReadWithTimeout(&adc1, 10);
  board::AdcStop(&adc1);
  vref = (kInternalVref * 4096)/value;
  (void)value;


//
//  uint32_t values[8] = {};
//  for(uint32_t i=0; i < 8; ++i) {
//    values[i] = board::AdcRead(&adc1, ADC_CN_1);
//  }
//  qsort(values, 8, sizeof(uint32_t), cmp);
//  uint32_t adc_value = values[int(8/2u)];
//  vref = __LL_ADC_CALC_VREFANALOG_VOLTAGE(adc_value, ADC_RESOLUTION_12B);
//
//  ADC_COMMON->CCR &= ~ADC_CCR_VREFEN;
  return vref;

//  // отладка
//  GPIO_InitTypeDef GPIO_InitStruct = {};
//  GPIO_InitStruct.Pin = GPIO_PIN_10;
//  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//  sConfig = {
//    .Channel = ADC_CHANNEL_6,
//    .Rank = ADC_REGULAR_RANK_1,
//    .SamplingTime = ADC_SAMPLETIME_39CYCLES_5,
//  };
//  if((HAL_ADC_ConfigChannel(&adc_hal, &sConfig) != HAL_OK))
//    return 2;
////  board::AdcStart(&adc1);
//  volatile float vdd = 0;
//  while(true) {
//    for (uint32_t i = 0; i < 8; ++i) {
//      values[i] = board::AdcRead(&adc1, ADC_CN_1);
//    }
//    qsort(values, 8, sizeof(uint32_t), cmp);
//    adc_value = values[int(8/2u)];
////    vdd = __LL_ADC_CALC_VREFANALOG_VOLTAGE(adc_value, ADC_RESOLUTION_12B);
//    vdd = (VREFINT_CAL_VREF * 1510) / adc_value;
//  }
////  board::AdcStop(&adc1);
//
//  adc_value = values[int(8/2u)];
//  return 0;
}
void BatteryTest() {
//  GPIO_InitTypeDef GPIO_InitStruct = {};
//  GPIO_InitStruct.Pin = GPIO_PIN_3;
//  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//  volatile uint32_t adcData = 0;
//  while(true) {
//    // Enable HSI
//    __HAL_RCC_HSI_ENABLE();
//    // Wait till HSI is ready
//    while (__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == RESET) {
//    }
//    __HAL_RCC_ADC_CLK_ENABLE();
//    // Calibrate ADC if any calibraiton hardware
//    HAL_ADCEx_Calibration_Start(&adc_hal);
//    ADC_ChannelConfTypeDef sConfig{
//      .Channel = ADC_CHANNEL_2,
//      .Rank = ADC_REGULAR_RANK_1,
//      .SamplingTime = ADC_SAMPLETIME_39CYCLES_5,
//    };
//    HAL_ADC_ConfigChannel(&adc_hal, &sConfig);
//    // Start ADC Software Conversion
//    HAL_ADC_Start(&adc_hal);
//    HAL_ADC_PollForConversion(&adc_hal, HAL_MAX_DELAY);
//    adcData = HAL_ADC_GetValue(&adc_hal);
//    ADC_Disable(&adc_hal);
//    __HAL_RCC_ADC_CLK_DISABLE();
//    // Disable HSI
//    __HAL_RCC_HSI_DISABLE();
//  }

//  return adcData;
}
bool AdcWaitComplete(Adc_t *obj) {
  if(obj->hal_obj == &sdadc_hal){
    return adc_value_ready;
  }
  return true;
}
uint32_t AdcGetStartTime() {
  return start_time_;
}

}

[[maybe_unused]]
void DMA1_Channel1_IRQHandler(void) {
  HAL_DMA_IRQHandler(&board::hdma_adc1);
}
[[maybe_unused]]
void DMA2_Channel3_IRQHandler(void) {
  HAL_DMA_IRQHandler(&board::hdma_sdadc1);
}
void ADC1_IRQHandler(void) {
  HAL_ADC_IRQHandler(&board::adc_hal);
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
  board::adc_value_ready = true;
  volatile int i=0;
  (void)i;
}
void SDADC1_IRQHandler(void) {
  HAL_SDADC_IRQHandler(&board::sdadc_hal);
}
void HAL_SDADC_InjectedConvCpltCallback(SDADC_HandleTypeDef *hsdadc) {
  /* Get conversion value */
//  board::adc_value = int16_t(HAL_SDADC_InjectedGetValue(hsdadc, (uint32_t *) &board::InjChannel));
//  if(board::adc_value == 0xFFFF)
//    board::adc_value = 0xFFFE;
  board::adc_value_ready = true;
}
[[maybe_unused]]
void TIM19_IRQHandler(void) {
  HAL_TIM_IRQHandler(&board::sdhtim);
  HAL_SDADC_InjectedStart_IT(&board::sdadc_hal);
//  board::adc_value = 0xFFFE;
//  board::adc_value_ready = true;
}