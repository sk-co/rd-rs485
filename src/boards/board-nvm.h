//
// Created by Serg on 14.06.2021.
//

#ifndef LORAWAN_DEV_SRC_BOARDS_BOARD_NVM_H_
#define LORAWAN_DEV_SRC_BOARDS_BOARD_NVM_H_

#include <cstdint>

namespace board {

int NvmInit();
void NvmDataMgmtEvent(uint16_t notifyFlags);
int NvmStore();
int NvmRestore();
int NvmReset();

}

#endif //LORAWAN_DEV_SRC_BOARDS_BOARD_NVM_H_
