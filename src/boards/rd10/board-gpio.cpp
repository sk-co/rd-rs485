#include "board-gpio.h"
#include "mcu.h"
#include "utilities.h"
#include "board-config.h"
//#include "board-rtc.h"
#include "sysIrqHandlers.h"

namespace {

Gpio_t *GpioIrq[16] = {};

}

namespace board {

void GpioInit(Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config,
                PinTypes type, uint32_t value) {
  GPIO_InitTypeDef GPIO_InitStructure;
  obj->pin = pin;
  if (pin == NC) {
    return;
  }
  obj->pinIndex = (0x01 << (obj->pin & 0x0F));
  if ((obj->pin & 0xF0) == 0x00) {
    obj->port = GPIOA;
    __HAL_RCC_GPIOA_CLK_ENABLE();
  } else if ((obj->pin & 0xF0) == 0x10) {
    obj->port = GPIOB;
    __HAL_RCC_GPIOB_CLK_ENABLE();
  } else if ((obj->pin & 0xF0) == 0x20) {
    obj->port = GPIOC;
    __HAL_RCC_GPIOC_CLK_ENABLE();
  } else if ((obj->pin & 0xF0) == 0x30) {
    obj->port = GPIOD;
    __HAL_RCC_GPIOD_CLK_ENABLE();
  } else if ((obj->pin & 0xF0) == 0x40) {
    obj->port = GPIOE;
    __HAL_RCC_GPIOE_CLK_ENABLE();
  } else {
    obj->port = GPIOF;
    __HAL_RCC_GPIOF_CLK_ENABLE();
  }
  GPIO_InitStructure.Pin = obj->pinIndex;
  GPIO_InitStructure.Pull = obj->pull = type;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  if (mode == PIN_INPUT) {
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  } else if (mode == PIN_ANALOGIC) {
    GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  } else if (mode == PIN_ALTERNATE_FCT) {
    if (config == PIN_OPEN_DRAIN) {
      GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
    } else {
      GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    }
    GPIO_InitStructure.Alternate = value;
  } else // mode output
  {
    if (config == PIN_OPEN_DRAIN) {
      GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
    } else {
      GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    }
  }
  HAL_GPIO_Init(reinterpret_cast<GPIO_TypeDef*>(obj->port), &GPIO_InitStructure);
  // Sets initial output value
  if (mode == PIN_OUTPUT) {
    GpioWrite(obj, value);
  }
}
void GpioDeInitLP(Gpio_t *obj) {
  GpioInit(obj, obj->pin, PIN_ANALOGIC, PIN_OPEN_DRAIN,
           PIN_NO_PULL, 0);
}
void GpioDeInit(Gpio_t *obj) {
  HAL_GPIO_DeInit(reinterpret_cast<GPIO_TypeDef*>(obj->port), obj->pinIndex);
}
// Деинициализация выводов.
void GpioDeInit(const PinNames *pins, uint32_t len) {
  Gpio_t obj = {};
  for(uint32_t i=0; i < len; ++i) {
    GpioInit(&obj, pins[i], PIN_ANALOGIC, PIN_OPEN_DRAIN,
             PIN_NO_PULL, 0);
//    GpioInit(&obj, pins[i], PIN_OUTPUT, PIN_PUSH_PULL,
//             PIN_PULL_DOWN, 0);
  }
}
void GpioSetContext(Gpio_t *obj, void *context) {
  obj->Context = context;
}
void GpioSetInterrupt(Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority,
                      GpioIrqHandler *irqHandler) {
  uint32_t priority = 0;
  IRQn_Type IRQnb = EXTI0_IRQn;
  GPIO_InitTypeDef GPIO_InitStructure;
  if (irqHandler == nullptr) {
    return;
  }
  obj->IrqHandler = irqHandler;
  GPIO_InitStructure.Pin = obj->pinIndex;
  if (irqMode == IRQ_RISING_EDGE) {
    GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
  } else if (irqMode == IRQ_FALLING_EDGE) {
    GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
  } else {
    GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
  }
  GPIO_InitStructure.Pull = obj->pull;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(reinterpret_cast<GPIO_TypeDef*>(obj->port), &GPIO_InitStructure);
  switch (irqPriority) {
    case IRQ_VERY_LOW_PRIORITY:
    case IRQ_LOW_PRIORITY:priority = 3;
      break;
    case IRQ_MEDIUM_PRIORITY:priority = 2;
      break;
    case IRQ_HIGH_PRIORITY:priority = 1;
      break;
    case IRQ_VERY_HIGH_PRIORITY:
    default:priority = 0;
      break;
  }
  switch (obj->pinIndex) {
    case GPIO_PIN_0:IRQnb = EXTI0_IRQn;
      break;
    case GPIO_PIN_1:IRQnb = EXTI1_IRQn;
      break;
    case GPIO_PIN_2:IRQnb = EXTI2_TSC_IRQn;
      break;
    case GPIO_PIN_3:IRQnb = EXTI3_IRQn;
      break;
    case GPIO_PIN_4:IRQnb = EXTI4_IRQn;
      break;
    case GPIO_PIN_5:
    case GPIO_PIN_6:
    case GPIO_PIN_7:
    case GPIO_PIN_8:
    case GPIO_PIN_9:IRQnb = EXTI9_5_IRQn;
      break;
    case GPIO_PIN_10:
    case GPIO_PIN_11:
    case GPIO_PIN_12:
    case GPIO_PIN_13:
    case GPIO_PIN_14:
    case GPIO_PIN_15:IRQnb = EXTI15_10_IRQn;
      break;
    default:break;
  }
  GpioIrq[(obj->pin) & 0x0F] = obj;
  __HAL_GPIO_EXTI_CLEAR_IT(obj->pinIndex);
  HAL_NVIC_SetPriority(IRQnb, priority, 0);
  HAL_NVIC_EnableIRQ(IRQnb);
}
void GpioRemoveInterrupt(Gpio_t *obj) {
  // Clear callback before changing pin mode
  GpioIrq[(obj->pin) & 0x0F] = nullptr;
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.Pin = obj->pinIndex;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(reinterpret_cast<GPIO_TypeDef*>(obj->port), &GPIO_InitStructure);
  switch (obj->pinIndex) {
    case GPIO_PIN_0:HAL_NVIC_DisableIRQ(EXTI0_IRQn);
      break;
    case GPIO_PIN_1:HAL_NVIC_DisableIRQ(EXTI1_IRQn);
      break;
    case GPIO_PIN_2:HAL_NVIC_DisableIRQ(EXTI2_TSC_IRQn);
      break;
    case GPIO_PIN_3:HAL_NVIC_DisableIRQ(EXTI3_IRQn);
      break;
    case GPIO_PIN_4:HAL_NVIC_DisableIRQ(EXTI4_IRQn);
      break;
    case GPIO_PIN_5:
    case GPIO_PIN_6:
    case GPIO_PIN_7:
    case GPIO_PIN_8:
    case GPIO_PIN_9:
      if(GpioIrq[5] == nullptr && GpioIrq[6] == nullptr && GpioIrq[7] == nullptr
        && GpioIrq[8] == nullptr && GpioIrq[9] == nullptr) {
        HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
      }
      break;
    case GPIO_PIN_10:
    case GPIO_PIN_11:
    case GPIO_PIN_12:
    case GPIO_PIN_13:
    case GPIO_PIN_14:
    case GPIO_PIN_15:
      if(GpioIrq[10] == nullptr && GpioIrq[11] == nullptr && GpioIrq[12] == nullptr
        && GpioIrq[13] == nullptr && GpioIrq[14] == nullptr && GpioIrq[15] == nullptr) {
        HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
      }
      break;
    default:break;
  }
}
void GpioWrite(Gpio_t *obj, uint32_t value) {
  if (obj == nullptr) {
    assert_param(MCU_FAIL);
  }
  // Check if pin is not connected
  if (obj->pin == NC) {
    return;
  }
  HAL_GPIO_WritePin(reinterpret_cast<GPIO_TypeDef*>(obj->port), obj->pinIndex, (GPIO_PinState) value);
}
void GpioToggle(Gpio_t *obj) {
  if (obj == nullptr) {
    assert_param(MCU_FAIL);
  }
  // Check if pin is not connected
  if (obj->pin == NC) {
    return;
  }
  HAL_GPIO_TogglePin(reinterpret_cast<GPIO_TypeDef*>(obj->port), obj->pinIndex);
}
uint32_t GpioRead(Gpio_t *obj) {
  if (obj == nullptr) {
    assert_param(MCU_FAIL);
  }
  // Check if pin is not connected
  if (obj->pin == NC) {
    return 0;
  }
  return HAL_GPIO_ReadPin(reinterpret_cast<GPIO_TypeDef*>(obj->port), obj->pinIndex);
}

} // namespace board

#ifndef APP_BOOTLOADER
[[maybe_unused]]
void EXTI0_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}
[[maybe_unused]]
void EXTI1_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}
[[maybe_unused]]
void EXTI2_TSC_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}
[[maybe_unused]]
void EXTI3_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}
[[maybe_unused]]
void EXTI4_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}
[[maybe_unused]]
void EXTI9_5_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}
[[maybe_unused]]
void EXTI15_10_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}
void HAL_GPIO_EXTI_Callback(uint16_t gpioPin) {
  uint8_t callbackIndex = 0;
  // TODO: Переделать через GpioIrq[(obj->pin) & 0x0F]
  if (gpioPin > 0) {
    while (gpioPin != 0x01) {
      gpioPin = gpioPin >> 1;
      callbackIndex++;
    }
  }
  if ((GpioIrq[callbackIndex] != nullptr)
    && (GpioIrq[callbackIndex]->IrqHandler != nullptr)) {
    GpioIrq[callbackIndex]->IrqHandler(GpioIrq[callbackIndex]->Context);
  }
}
#endif
