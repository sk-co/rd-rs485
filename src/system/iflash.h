//
// Created by SERG on 31.03.2021.
//

#ifndef LORAWAN_DEV_TOOL_BOARD_IFLASH_H_
#define LORAWAN_DEV_TOOL_BOARD_IFLASH_H_

#include <cstdint>
#include "common_types.h"

namespace board {

class IFlash {
 public:
  const uint32_t kStartAddr_;
  const uint32_t kPageSize_;
  const uint32_t kEmptyValue_;
  const uint32_t kMinWriteBlock_;
  const uint32_t kPagesCount_;
  const uint32_t kSize_;

  IFlash(uint32_t start_addr, uint32_t page_size,
          uint32_t empty_value, uint32_t min_write_block, uint32_t pages_count)
    : kStartAddr_(start_addr), kPageSize_(page_size), kEmptyValue_(empty_value),
      kMinWriteBlock_(min_write_block), kPagesCount_(pages_count),
      kSize_(kPageSize_*kPagesCount_) {
  }
  virtual void Lock() = 0;
  virtual void Unlock() = 0;
  virtual int ErasePage(uint32_t addr) = 0;
  virtual BoolError IsEmpty(uint32_t addr, uint32_t size) = 0;
  virtual int Write(uint32_t addr, const uint8_t *data, uint32_t size) = 0;
  virtual int WritePage(uint32_t addr, const uint8_t *data) = 0;
  virtual int Read(uint32_t addr, uint8_t *data, uint32_t size) = 0;
  virtual int ReadPage(uint32_t addr, uint8_t *data) = 0;

};

}

#endif //LORAWAN_DEV_TOOL_BOARD_IFLASH_H_
