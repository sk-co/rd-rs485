#ifndef LORAWAN_DEV_SRC_BOARDS_LLB_S_BOARD_CONFIG_H_
#define LORAWAN_DEV_SRC_BOARDS_LLB_S_BOARD_CONFIG_H_

/*!
 * Defines the time required for the TCXO to wakeup [ms].
 */

#define LED_1_PIN                                   PB_8
#define LED_2_PIN                                   NC

//#define OSC_LSE_IN                                  PC_14
//#define OSC_LSE_OUT                                 PC_15

// #define OSC_HSE_IN                                  NC
// #define OSC_HSE_OUT                                 NC

#define SWCLK                                       PA_14
#define SWDIO                                       PA_13

#define UART1_TX                                    PA_9
#define UART1_RX                                    PA_10

#define UART2_TX                                    PB_3
#define UART2_RX                                    PB_4
#define UART2_DE                                    PA_1

#define RAM_MOSI                                    PB_0
#define RAM_MISO                                    PA_6
#define RAM_SCLK                                    PA_5
#define RAM_CS                                      PA_4

#define ADC1_1_PIN                                  PA_2
#define ADC1_1_CHANNEL                              ADC_CHANNEL_2
#define ADC1_2_PIN                                  PA_3
#define ADC1_2_CHANNEL                              ADC_CHANNEL_3

#define SDADC_1_PIN                                 PB_1
#define SDADC_1_CHANNEL                             SDADC_CHANNEL_5
#define SDADC_2_PIN                                 PB_2
#define SDADC_2_CHANNEL                             SDADC_CHANNEL_4

#define INPUT_OK_PIN                                PA_0

#define T1_EN_PIN                                   PF_6
#define T2_EN_PIN                                   PF_7
#define T3_EN_PIN                                   PB_5
#define T4_EN_PIN                                   PB_6
#define T1_SND_PIN                                  PD_8
#define T2_SND_PIN                                  PA_8
#define T3_SND_PIN                                  PA_11
#define T4_SND_PIN                                  PA_12
#define T1_RCV_PIN                                  PE_8
#define T2_RCV_PIN                                  PE_9
#define T3_RCV_PIN                                  PB_14
#define T4_RCV_PIN                                  PB_15

#define MODE_1_PIN                                  PA_15
#define MODE_2_PIN                                  PB_7

#define SIGNAL_OUT_PIN                              PC_13


#endif //LORAWAN_DEV_SRC_BOARDS_LLB_S_BOARD_CONFIG_H_
