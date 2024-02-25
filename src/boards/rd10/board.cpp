
#include "board.h"
#include <cstring>
#include "mcu.h"
#include "utilities.h"
#include "gpio-def.h"
#include "adc-def.h"
#include "board-spi.h"
#include "uart-def.h"
#include "timer.h"
#include "board-config.h"
#include "board-lpm.h"
#include "board-rtc.h"
#include "board-delay.h"
#include "board-critical-section.h"
#include "board-uart.h"
#include "board-peripherals.h"
#include "sysIrqHandlers.h"
#include "error-handler.h"
#include "sysIrqHandlers.h"

/*!
 * Unique Devices IDs register set ( STM32L1xxx )
 */
#define         ID1                                 ( 0x1FFF7590 )
#define         ID2                                 ( 0x1FFF7594 )
#define         ID3                                 ( 0x1FFF7598 )

// Инициализируется в скрипте линкера.
extern unsigned int __vectors_start;

namespace board {

/*!
 * LED GPIO pins objects
 */
Gpio_t led1_pin;
Led led1;
Led led2; // На плате нет, но для совместимости кода оставлен.
Led led3; // На плате нет, но для совместимости кода оставлен.
Gpio_t UsbDetect;
IWDG_HandleTypeDef hiwdg;

/*
 * MCU objects
 */
// Adc_t Adc;


#if defined( USE_USB_CDC )
Uart_t UartUsb;

#endif

/*!
 * Initializes the unused GPIO to a know status
 */
static void BoardUnusedIoInit();
/*!
 * System Clock Configuration
 */
static void SystemClockConfig();
/*!
 * System Clock Re-Configuration when waking up from STOP mode
 */
//static void SystemClockReConfig( void );
///*!
// * Flag to indicate if the MCU is Initialized
// */
//static bool McuInitialized = false;

/*!
 * Nested interrupt counter.
 *
 * \remark Interrupt should only be fully disabled once the value is 0
 */
static uint8_t IrqNestLevel = 0;

void BoardDisableIrq() {
  __disable_irq();
  IrqNestLevel++;
}
void BoardEnableIrq() {
  IrqNestLevel--;
  if (IrqNestLevel == 0) {
    __enable_irq();
  }
}

void BoardInitMcu() {
  HAL_Init();
  SystemClockConfig();
  // Включение тактирования портов.
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  GpioInit(&led1_pin, LED_1_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
  led1.Init(&led1_pin, true);
  led1.On();
  HAL_Delay(50);
  led2.Off();
  BoardUnusedIoInit();
}
void InitSpi1() {
  SpiInit(&spi1, SPI_1, RAM_MOSI, RAM_MISO, RAM_SCLK, NC);
  SpiConfig(&spi1, 8000000, SPI_DATASIZE_8BIT, SPI_POLARITY_LOW, SPI_PHASE_1EDGE, 0);
}
void InitUart1() {
  UartInit(&board::uart1, UART_1, UART1_TX, UART1_RX);
}
void InitUart2() {
  UartInit(&board::uart2, UART_2, UART2_TX, UART2_RX, UART2_DE);
}
void InitRtc() {
  // Внешний не подключен, начальное время - 0.
  RtcInit(0);
}
void InitAdc() {
  if(AdcInit(&board::adc1))
    ErrorHandler(nullptr);
  if(AdcConfigChannel(&board::adc1, ADC_CN_2))
    ErrorHandler(nullptr);
//  DefineVRef();

  if(AdcInit(&board::sdadc1))
    ErrorHandler(nullptr);
  if(AdcConfigChannel(&board::sdadc1, ADC_CN_1))
    ErrorHandler(nullptr);
}
void DeInitAdc() {
  AdcDeinit(&board::adc1);
  AdcDeinit(&board::sdadc1);
}
void InitIWDG(uint32_t period_ms) {
  if(period_ms > 32000)
    period_ms = 32000;
  __HAL_DBGMCU_FREEZE_IWDG();
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Reload = period_ms / (1000/ (32000 / 256)) + 1;
  hiwdg.Init.Window = hiwdg.Init.Reload;
  HAL_IWDG_Init(&hiwdg);
}
void RefreshIWDG() {
  if(hiwdg.Instance != nullptr) {
    HAL_IWDG_Refresh(&hiwdg);
  }
}

void BoardResetMcu() {
  CRITICAL_SECTION_BEGIN();
  //Restart system
  NVIC_SystemReset();
}
// Вызывается для снижения потребления при входе в режим STOP.
void BoardDeInitMcu() {
  //Gpio_t ioPin;
  //AdcDeInit( &Adc );
//  board::SpiDeInit(&SX1276.Spi);
//  SX1276IoDeInit();
  GpioDeInit(&led1_pin);

//  PinNames pins[] = {
//    PB_3,
//    PB_4,
//    PB_5,
//    PB_6,
//    PB_7,
//    PB_8,
//    PA_0,
//    PA_1,
//    PA_2,
//    PA_3,
//    PA_4,
//    PA_5,
//    PA_6,
//    PA_7,
//    PA_8,
//    PA_9,//no
//
//    PB_0,
//    PB_2,
//    PB_12,  // DIR_PIN, почему-то уменьшается потребление на 0.3 на пустой плате.
//    PA_10,
//    PA_11,
//    PA_12,
//
//    PC_13,
//    PA_15,
//
//    PH_3,
//
//    PC_14,
//    PC_15,
//    PA_0,
//  };
//  GpioDeInit(pins, sizeof(pins));
}
uint32_t BoardGetRandomSeed() {
  return ((*(uint32_t *) ID1) ^ (*(uint32_t *) ID2) ^ (*(uint32_t *) ID3));
}
void BoardGetUniqueId(uint8_t *id) {
  id[7] = ((*(uint32_t *) ID1) + (*(uint32_t *) ID3)) >> 24;
  id[6] = ((*(uint32_t *) ID1) + (*(uint32_t *) ID3)) >> 16;
  id[5] = ((*(uint32_t *) ID1) + (*(uint32_t *) ID3)) >> 8;
  id[4] = ((*(uint32_t *) ID1) + (*(uint32_t *) ID3));
  id[3] = ((*(uint32_t *) ID2)) >> 24;
  id[2] = ((*(uint32_t *) ID2)) >> 16;
  id[1] = ((*(uint32_t *) ID2)) >> 8;
  id[0] = ((*(uint32_t *) ID2));
}
/*!
 * Factory power supply
 */
#define FACTORY_POWER_SUPPLY                        3300 // mV

/*!
 * VREF calibration value
 */
#define VREFINT_CAL                                 ( *( uint16_t* )0x1FF80078 )

/*!
 * ADC maximum value
 */
#define ADC_MAX_VALUE                               4095

/*!
 * Battery thresholds
 */
#define BATTERY_MAX_LEVEL                           4150 // mV
#define BATTERY_MIN_LEVEL                           3200 // mV
#define BATTERY_SHUTDOWN_LEVEL                      3100 // mV

//static uint16_t BatteryVoltage = BATTERY_MAX_LEVEL;

//uint16_t BoardBatteryMeasureVolage() {
//  // Батарейку не измеряем, всегда все хорошо.
//  BatteryVoltage = BATTERY_MAX_LEVEL;
//  return BatteryVoltage;
//
//  // uint16_t vdd = 0;
//  // uint16_t vref = VREFINT_CAL;
//  // uint16_t vdiv = 0;
//  // uint16_t batteryVoltage = 0;
//  // vdiv = AdcReadChannel( &Adc, BAT_LEVEL_CHANNEL );
//  // vref = AdcReadChannel( &Adc, ADC_CHANNEL_VREFINT );
//  // vdd = ( float )FACTORY_POWER_SUPPLY * ( float )VREFINT_CAL / ( float )vref;
//  // batteryVoltage = vdd * ( ( float )vdiv / ( float )ADC_MAX_VALUE );
//  // //                                vDiv
//  // // Divider bridge  VBAT <-> 470k -<--|-->- 470k <-> GND => vBat = 2 * vDiv
//  // batteryVoltage = 2 * batteryVoltage;
//  // return batteryVoltage;
//}

//uint32_t BoardGetBatteryVoltage() {
//  return BatteryVoltage;
//}
//uint8_t BoardGetBatteryLevel() {
//  uint8_t batteryLevel = 0;
//  BatteryVoltage = BoardBatteryMeasureVolage();
//
//  if (GetBoardPowerSource() == USB_POWER) {
//    batteryLevel = 0;
//  } else {
//    if (BatteryVoltage >= BATTERY_MAX_LEVEL) {
//      batteryLevel = 254;
//    } else if ((BatteryVoltage > BATTERY_MIN_LEVEL)
//      && (BatteryVoltage < BATTERY_MAX_LEVEL)) {
//      batteryLevel = ((253 * (BatteryVoltage - BATTERY_MIN_LEVEL))
//        / (BATTERY_MAX_LEVEL - BATTERY_MIN_LEVEL)) + 1;
//    } else if ((BatteryVoltage > BATTERY_SHUTDOWN_LEVEL)
//      && (BatteryVoltage <= BATTERY_MIN_LEVEL)) {
//      batteryLevel = 1;
//    } else if (BatteryVoltage <= BATTERY_SHUTDOWN_LEVEL) {
//      batteryLevel = 255;
//    }
//  }
//  return batteryLevel;
//}
static void BoardUnusedIoInit() {
#if defined( USE_DEBUGGER )
  HAL_DBGMCU_EnableDBGStopMode();
  HAL_DBGMCU_EnableDBGSleepMode();
  HAL_DBGMCU_EnableDBGStandbyMode();
#else
  HAL_DBGMCU_DisableDBGSleepMode( );
  HAL_DBGMCU_DisableDBGStopMode( );
  HAL_DBGMCU_DisableDBGStandbyMode( );

  Gpio_t ioPin = {};
  GpioInit( &ioPin, SWDIO, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
  GpioInit( &ioPin, SWCLK, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
}
void SystemClockConfig() {
//  // Инициализация из другого проекта.
//  __HAL_RCC_COMP_CLK_ENABLE();
//  __HAL_RCC_SYSCFG_CLK_ENABLE();
//  __HAL_RCC_PWR_CLK_ENABLE();
//  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
//  /* System interrupt init*/
//  /* MemoryManagement_IRQn interrupt configuration */
//  HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
//  /* BusFault_IRQn interrupt configuration */
//  HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
//  /* UsageFault_IRQn interrupt configuration */
//  HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
//  /* SVC_IRQn interrupt configuration */
//  HAL_NVIC_SetPriority(SVC_IRQn, 0, 0);
//  /* DebugMonitor_IRQn interrupt configuration */
//  HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);
//  /* PendSV_IRQn interrupt configuration */
//  HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);

  // 24 MHz
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;
  /** Configure LSE Drive Capability
 */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
//  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3 ;  // 24 MHz

//  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSI;
//  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
//  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
//  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
////  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
//  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
//  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;  // 48 MHz
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
    ErrorHandler(nullptr);
  }
  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK
    |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
    |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)  {
    ErrorHandler(nullptr);
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
    |RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_SDADC|RCC_PERIPHCLK_ADC1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.SdadcClockSelection = RCC_SDADCSYSCLK_DIV4; // 6 MHz
  PeriphClkInit.Adc1ClockSelection = RCC_ADC1PCLK2_DIV4;  // 6 MHz
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
    ErrorHandler(nullptr);
  }
//  volatile uint32_t freq = HAL_RCC_GetHCLKFreq();
//  /**Configure the Systick interrupt time  */
//  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
//  /**Configure the Systick  */
//  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
//  /* SysTick_IRQn interrupt configuration */
//  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
  __enable_irq();
}
/**
  * \brief Enters Low Power Stop Mode
  *
  * \note ARM exists the function when waking up
  */
void LpmEnterStopMode() {
//  CRITICAL_SECTION_BEGIN();
//  BoardDeInitMcu();
//  HAL_SuspendTick();
//  /* Clear Status Flag before entering STOP/STANDBY Mode */
//  LL_PWR_ClearFlag_C1STOP_C1STB();
//  CRITICAL_SECTION_END();
//  // Enter Stop Mode
////  HAL_PWREx_EnterSHUTDOWNMode();
//  HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
////  HAL_PWR_EnterSTANDBYMode();
}
/*!
 * \brief Exists Low Power Stop Mode
 */
void LpmExitStopMode() {
  // Disable IRQ while the MCU is not running on HSI
  CRITICAL_SECTION_BEGIN();
  // Initilizes the peripherals
//  BoardInitMcu();
  BoardReInitMcu();
  HAL_ResumeTick();
  CRITICAL_SECTION_END();
}
/*!
 * \brief Enters Low Power Sleep Mode
 *
 * \note ARM exits the function when waking up
 */
void LpmEnterSleepMode() {
  HAL_SuspendTick();
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  HAL_ResumeTick();
}
void BoardLowPowerHandler() {
  __disable_irq();
  /*!
   * If an interrupt has occurred after __disable_irq( ), it is kept pending
   * and cortex will not enter low power anyway
   */
  LpmEnterLowPower();
  __enable_irq();
}

uint32_t GetSysTick() {
  return HAL_GetTick();
}
} // namespace board

#ifdef USE_FULL_ASSERT
/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
void assert_failed( uint8_t* file, uint32_t line )
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\n", file, line) */

    /* Infinite loop */
    while( 1 )
    {
    }
}
#endif

