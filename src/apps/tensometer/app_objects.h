#ifndef LORAWAN_DEV_SRC_APPS_DEVICES_OTHER_CLASSA_RAK811_APP_OBJECTS_H_
#define LORAWAN_DEV_SRC_APPS_DEVICES_OTHER_CLASSA_RAK811_APP_OBJECTS_H_

#include "f-impulse-generator.h"
#include "rs485.h"
#include "cmd-processor.h"
#include "ram_23k256.h"
#include "temp_sensor.h"
#include "f-answer-reader.h"
#include "external-signal.h"

namespace app {

class Obj {
 public:
  static IUart *Rs485();
  static CmdProcessor &CommandProcessor();
  static ext::Ram23k256 &Ram();
  static FImpulseGenerator* ImpulseGenerator(uint32_t channel);
  static TempSensor* TemperatureSensor(uint32_t channel);
  static FAnswerReader* FreqReader(uint32_t channel);
  static ExternalSignal &ExtSignal();
};

}

#endif //LORAWAN_DEV_SRC_APPS_DEVICES_OTHER_CLASSA_RAK811_APP_OBJECTS_H_
