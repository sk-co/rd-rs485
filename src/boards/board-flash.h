/*
 * board-flash.h
 *
 *  Created on: 29 авг. 2019 г.
 *      Author: SERG
 */

#ifndef INC_BOARDS_RAK811_BOARD_FLASH_H_
#define INC_BOARDS_RAK811_BOARD_FLASH_H_

#include "iflash.h"


namespace board {

class CpuFlash : public IFlash {
 public:
  CpuFlash();
  void Lock() override;
  void Unlock() override;
  int ErasePage(uint32_t addr) override;
  BoolError IsEmpty(uint32_t addr, uint32_t size) override;
  int Write(uint32_t addr, const uint8_t *data, uint32_t size) override;
  int WritePage(uint32_t addr, const uint8_t *data) override;
  int Read(uint32_t addr, uint8_t *data, uint32_t size) override;
  int ReadPage(uint32_t addr, uint8_t *data) override;
};

int InitCpuFlash();

}

#endif /* INC_BOARDS_RAK811_BOARD_FLASH_H_ */
