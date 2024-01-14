#include <cstdio>
#include <ctime>
#include "utilities.h"
#include "board.h"
#include "board-critical-section.h"
#include "board-peripherals.h"
#include "log.h"
#include "board-nvm.h"
#include "app_objects.h"
#include "app_core.h"
#include "error-handler.h"
#include "app_config.h"
#include "board-delay.h"
#include "board-gpio.h"


constexpr Version_t kHexVersion {
  .Fields {
    .Revision = 0,
    .Patch = 0,
    .Minor = FIRMWARE_VERSION_LO,
    .Major = FIRMWARE_VERSION_HI,
  }
};

// Шаблон для показа размера структуры во время компиляции.
//template<int N>
//struct dbg_show_size{ operator char() { return N+ 256; } }; //always overflow

void InitBoard() {
  board::BoardInitMcu();
  board::InitRtc();
  board::InitIWDG(conf::kWdgPeriodMs);
  board::InitUart1();
  board::InitUart2();
//  board::InitSpi1();
//  board::InitAdc();
}

int main() {
//  char(dbg_show_size<sizeof(RegionNvmDataGroup1_t)>());
  InitBoard();
  LOG_INIT(board::GetLogUart(), 115200);
  LOG_TRC("Start RD v%d.%d\n", FIRMWARE_VERSION_HI, FIRMWARE_VERSION_LO);
  while(!LOG_IS_TRANSMITED());
  app::Init();
  while (true) {
    app::Work();
    board::RefreshIWDG();
  }
}
