#include "external-signal.h"

#include "board-gpio.h"

namespace {

void IrqHandler(void *context) {
  reinterpret_cast<app::ExternalSignal*>(context)->IrqHandler();
}

}

namespace app {

ExternalSignal::ExternalSignal(PinNames pin) {
  board::GpioInit(&pin_, pin, PIN_INPUT, PIN_OPEN_DRAIN,
                  PIN_NO_PULL, 0);
}
void ExternalSignal::StartWait() {
  is_signal_ = false;
  is_on_ = true;
  board::GpioSetContext(&pin_, this);
  board::GpioSetInterrupt(&pin_, IRQ_FALLING_EDGE, IRQ_HIGH_PRIORITY, ::IrqHandler);
}
void ExternalSignal::Stop() {
  is_on_ = false;
  board::GpioRemoveInterrupt(&pin_);
}
void ExternalSignal::IrqHandler() {
  is_signal_ = true;
}

}
