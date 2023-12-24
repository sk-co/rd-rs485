//
// Created by SERG on 28.09.2020.
//

#ifndef SIGNAL_RECORDER_UTILS_COMMON_TYPES_H_
#define SIGNAL_RECORDER_UTILS_COMMON_TYPES_H_

enum class BoolError : uint8_t {
  FALSE_ = 0,
  TRUE_ = 1,
  ERR = 2,
};
struct PtrToBuffer {
  uint8_t *ptr = nullptr;
  uint8_t len = 0;
};
struct ConstPtrToBuffer {
  const uint8_t *ptr = nullptr;
  uint8_t len = 0;
};

#endif //SIGNAL_RECORDER_UTILS_COMMON_TYPES_H_
