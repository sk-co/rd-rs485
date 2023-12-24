#ifndef RD_RS485_SRC_APPS_TENSOMETER_EXTERNAL_SIGNAL_H_
#define RD_RS485_SRC_APPS_TENSOMETER_EXTERNAL_SIGNAL_H_

#include "gpio-def.h"

namespace app {

class ExternalSignal {
 public:
  explicit ExternalSignal(PinNames pin);
  void StartWait();
  void Stop();
  [[nodiscard]] bool IsSignal() const {return is_signal_;};
  [[nodiscard]] bool IsWait() const {return is_on_;};

  void IrqHandler();

 private:
  Gpio_t pin_ = {};
  bool is_on_ = false;
  bool is_signal_ = false;
};

}

#endif //RD_RS485_SRC_APPS_TENSOMETER_EXTERNAL_SIGNAL_H_
