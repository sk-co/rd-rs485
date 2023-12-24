//
// Created by SERG on 31.03.2021.
//

#ifndef LORAWAN_DEV_TOOL_BOARD_FLASH_REGION_HPP_
#define LORAWAN_DEV_TOOL_BOARD_FLASH_REGION_HPP_

#include <cstring>
#include "iflash.h"

namespace board {
 class FlashRegion {
  public:
   const uint32_t kEmptyValue_;
   const uint32_t kPageSize_;
   const uint32_t kMinWriteBlock_;
   const uint32_t kPagesCount_;
   const uint32_t start_addr_;
   const uint32_t end_addr_;

   FlashRegion(IFlash *flash, uint32_t start_addr, uint32_t size)
    : kEmptyValue_(flash->kEmptyValue_),
      kPageSize_(flash->kPageSize_),
      kMinWriteBlock_(flash->kMinWriteBlock_),
      kPagesCount_(size/kPageSize_),
      start_addr_(start_addr), end_addr_(start_addr + size), flash_(flash) {
   };
   void Lock() {
     flash_->Lock();
   };
   void Unlock() {
     flash_->Unlock();
   };
   int ErasePage(uint32_t addr) {
     if(start_addr_ + addr + flash_->kPageSize_ > end_addr_)
       return 1;
     return flash_->ErasePage(addr + start_addr_);
   };
   BoolError IsEmpty(uint32_t addr, uint32_t size) {
     if(start_addr_ + addr + size > end_addr_)
       return BoolError::ERR;
     return flash_->IsEmpty(start_addr_ + addr, size);
   };
   int Write(uint32_t addr, const uint8_t *data, uint32_t size) {
     if(start_addr_ + addr + size > end_addr_)
       return 1;
     return flash_->Write(start_addr_ + addr, data, size);
   };
   int WritePage(uint32_t addr, const uint8_t *data) {
     if(start_addr_ + addr + flash_->kPageSize_ > end_addr_)
       return 1;
     return flash_->WritePage(start_addr_ + addr, data);
   };
   int Read(uint32_t addr, uint8_t *data, uint32_t size) {
     if(start_addr_ + addr + size > end_addr_)
       return 1;
     return flash_->Read(start_addr_ + addr, data, size);
   };
   int ReadPage(uint32_t addr, uint8_t *data) {
     if(start_addr_ + addr + flash_->kPageSize_ > end_addr_)
       return 1;
     return flash_->ReadPage(start_addr_ + addr, data);
   };
   int Clear() {
     for(uint32_t i=0; i < kPagesCount_; ++i) {
       if(flash_->ErasePage(start_addr_ + i*kPageSize_))
         return 1;
     }
     return 0;
   }
   // Заполнение с проверкой, удаляет все данные.
   bool Test() {
     uint8_t data_tx[256];
     uint8_t data_rx[256];
     for(uint32_t i=0; i < 256; ++i) {
       data_tx[i] = i;
     }
     test_bad_addr_ = UINT32_MAX;
     for(uint32_t i=0; i < kPagesCount_; ++i){
       auto page_addr = start_addr_ + i*kPageSize_;
       if(flash_->ErasePage(page_addr)) {
         test_bad_addr_ = flash_->kStartAddr_ + page_addr;
         return false;
       }
       for(uint32_t j=0; j < kPageSize_; j+=256) {
         memset(data_rx, 0, sizeof(data_rx));
         if(flash_->Write(page_addr + j, data_tx, 256)) {
           test_bad_addr_ = flash_->kStartAddr_ + page_addr;
           return false;
         }
         if(flash_->Read(page_addr + j, data_rx, 256)) {
           test_bad_addr_ = flash_->kStartAddr_ + page_addr;
           return false;
         }
         if(memcmp(data_rx, data_tx, 256) != 0) {
           test_bad_addr_ = flash_->kStartAddr_ + page_addr;
           return false;
         }
       }
       if(flash_->ErasePage(page_addr)) {
         test_bad_addr_ = flash_->kStartAddr_ + page_addr;
         return false;
       }
     }
     return true;
   }
   [[nodiscard]] uint32_t GetTestBadAddr() const {return test_bad_addr_;}

  private:
   IFlash *flash_ = nullptr;
   uint32_t test_bad_addr_ = UINT32_MAX;
 };
}

#endif //LORAWAN_DEV_TOOL_BOARD_FLASH_REGION_HPP_
