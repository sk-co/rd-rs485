//
// Created by SERG on 22.10.2020.
//

#ifndef MODEM_CMD_UTILS_FULL_ROUND_BUF_H_
#define MODEM_CMD_UTILS_FULL_ROUND_BUF_H_

#include <cstdint>

namespace utils {

template<typename T, size_t Size>
class SimpleRoundBuf {
 public:
  SimpleRoundBuf() {
    Reset();
  }
  void Reset() {
    memset(buf_, 0, Size*sizeof(T));
    head_ind_ = 0;
    is_full_ = false;
  }
  void AddValue(const T &value) {
    buf_[head_ind_] = value;
    if(head_ind_ == Size-1) {
      head_ind_ = 0;
      is_full_ = true;
    }
    else {
      head_ind_ = head_ind_ + 1;
    }
  }
  const T* Data() {return buf_;}
  [[nodiscard]] bool IsFull() const {return is_full_;}

 private:
  T buf_[Size];
  size_t head_ind_ = 0;
  bool is_full_ = false;
};

}

#endif //MODEM_CMD_UTILS_FULL_ROUND_BUF_H_
