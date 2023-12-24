#include "app_objects.h"
#include "board-config.h"
#include "app_config.h"
#include "board-peripherals.h"

namespace app {

IUart *Obj::Rs485() {
  static ext::Rs485Interface rs485;
  if(!rs485.IsInit()) {
    rs485.Init(&board::uart2);
  }
  return &rs485;
}
CmdProcessor &Obj::CommandProcessor() {
  static CmdProcessor cmd_processor;
  if(!cmd_processor.IsInit())
    cmd_processor.Init(Obj::Rs485());
  return cmd_processor;
}
ext::Ram23k256 &Obj::Ram() {
  static ext::Ram23k256 ram_23k256(&board::spi1, RAM_CS);
  return ram_23k256;
}
FImpulseGenerator *Obj::ImpulseGenerator(uint32_t channel) {
  static FImpulseGenerator gen_1(SIGNAL_OUT_PIN, T1_SND_PIN,
                                 T1_RCV_PIN);
  static FImpulseGenerator gen_2(SIGNAL_OUT_PIN, T2_SND_PIN,
                                 T2_RCV_PIN);
  static FImpulseGenerator gen_3(SIGNAL_OUT_PIN, T3_SND_PIN,
                                 T3_RCV_PIN);
  static FImpulseGenerator gen_4(SIGNAL_OUT_PIN, T4_SND_PIN,
                                T4_RCV_PIN);
  static FImpulseGenerator* gens[] = {&gen_1, &gen_2, &gen_3, &gen_4};
  if(channel >= conf::kNChannels)
    return nullptr;
  return gens[channel];
}
TempSensor *Obj::TemperatureSensor(uint32_t channel) {
  static TempSensor temp_sensor1(T1_EN_PIN, MODE_1_PIN, MODE_2_PIN,
                                 &board::adc1, ADC_CN_2);
  static TempSensor temp_sensor2(T2_EN_PIN, MODE_1_PIN, MODE_2_PIN,
                                 &board::adc1, ADC_CN_2);
  static TempSensor temp_sensor3(T3_EN_PIN, MODE_1_PIN, MODE_2_PIN,
                                 &board::adc1, ADC_CN_2);
  static TempSensor temp_sensor4(T4_EN_PIN, MODE_1_PIN, MODE_2_PIN,
                                 &board::adc1, ADC_CN_2);
  static TempSensor* temp_sensors[] = {&temp_sensor1, &temp_sensor2, &temp_sensor3, &temp_sensor4};
  if(channel >= conf::kNChannels)
    return nullptr;
  return temp_sensors[channel];
}
FAnswerReader *Obj::FreqReader(uint32_t channel) {
  static FAnswerReader f_reader1(T1_SND_PIN, T1_RCV_PIN,
                                 &board::sdadc1, ADC_CN_1, &Obj::Ram());
  static FAnswerReader f_reader2(T2_SND_PIN, T2_RCV_PIN,
                                 &board::sdadc1, ADC_CN_1, &Obj::Ram());
  static FAnswerReader f_reader3(T3_SND_PIN, T3_RCV_PIN,
                                 &board::sdadc1, ADC_CN_1, &Obj::Ram());
  static FAnswerReader f_reader4(T4_SND_PIN, T4_RCV_PIN,
                                 &board::sdadc1, ADC_CN_1, &Obj::Ram());
  static FAnswerReader* f_readers[] = {&f_reader1, &f_reader2, &f_reader3, &f_reader4};
  if(channel >= conf::kNChannels)
    return nullptr;
  return f_readers[channel];
}
ExternalSignal &Obj::ExtSignal() {
  static ExternalSignal external_signal(INPUT_OK_PIN);
  return external_signal;
}
}
