#ifndef RD_RS485_SRC_APPS_TENSOMETER_APP_CORE_H_
#define RD_RS485_SRC_APPS_TENSOMETER_APP_CORE_H_

#include "cmd-processor.h"

namespace app {

enum class State {
  NO,
  IDLE,
  START_MEASUREMENT,
  SENDING_MEASUREMENT,
  ERROR,
  MEASUREMENT_BY_SIGNAL,
};

void Init();
void Work();
bool CmdHandler(const proto::MsgHdr *, const uint8_t *in, proto::MsgHdr *, uint8_t *out);

}

#endif //RD_RS485_SRC_APPS_TENSOMETER_APP_CORE_H_
