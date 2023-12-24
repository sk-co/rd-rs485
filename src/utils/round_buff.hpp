#ifndef CONTROL_DEVICE_COMMON_ROUND_BUFF_HPP_
#define CONTROL_DEVICE_COMMON_ROUND_BUFF_HPP_

#include <cinttypes>
#include <memory.h>

namespace utils {

template<typename T, size_t MaxSize>
class RoundBuff {
 public:
  RoundBuff() : ind_read_(0), ind_write_(0) {
  }
  void Reset() {
    ind_read_ = 0;
    ind_write_ = 0;
  }
  [[nodiscard]]
  size_t Size() const {
    if(ind_read_ <= ind_write_) {
      return ind_write_ - ind_read_;
    }
    return ind_write_ + (MaxSize - ind_read_);
  }
  [[nodiscard]]
  bool IsFull() const {
    size_t next_write = (ind_write_ == MaxSize-1) ? 0 : ind_write_ + 1;
    return next_write == ind_read_;
  }
  [[nodiscard]]
  bool IsEmply() const {
    return ind_read_ == ind_write_;
  }
  bool PushBack(const T &value) {
    size_t next_write = (ind_write_ == MaxSize-1) ? 0 : ind_write_ + 1;
    if(next_write == ind_read_) // полный
      return false;
    buf_[ind_write_] = value;
    ind_write_ = next_write;
    return true;
  }
  // Возвращает реально прочитанное кол-во элементов.
  size_t Read(T *buf, size_t len) {
    if(ind_read_ == ind_write_)
      return 0;
    size_t current_size = Size();
    if(len > current_size) {
      // Считываем все.
      len = current_size;
    }
    if(ind_read_ < ind_write_) {
      memcpy(buf, &buf_[ind_read_], len*sizeof(T));
      ind_read_ += len;
    }
    else {
      size_t len_first = MaxSize-ind_read_;
      if(len_first >= len) {
        memcpy(buf, &buf_[ind_read_], len*sizeof(T));
        ind_read_ += len;
        if(ind_read_ == MaxSize)
          ind_read_ = 0;
        return len;
      }
      size_t len_second = len - len_first;
      memcpy(buf, &buf_[ind_read_], len_first*sizeof(T));
      memcpy(buf + len_first, buf_, len_second*sizeof(T));
      ind_read_ = len_second;
    }
    return len;
  }
  // Считываем без изменения указателей.
  // Возвращает реально прочитанное кол-во элементов.
  size_t ReadCopy(T *buf, size_t len) {
    if(ind_read_ == ind_write_)
      return 0;
    size_t current_size = Size();
    if(len > current_size) {
      // Считываем все.
      len = current_size;
    }
    if(ind_read_ < ind_write_) {
      memcpy(buf, &buf_[ind_read_], len*sizeof(T));
    }
    else {
      size_t len_first = MaxSize - ind_read_;
      if(len_first >= len) {
        memcpy(buf, &buf_[ind_read_], len*sizeof(T));
        return len;
      }
      size_t len_second = len - len_first;
      memcpy(buf, &buf_[ind_read_], len_first*sizeof(T));
      memcpy(buf + len_first, buf_, len_second*sizeof(T));
    }
    return len;
  }
 private:
  T buf_[MaxSize];
  size_t ind_read_;
  size_t ind_write_;
};


}

#endif //CONTROL_DEVICE_COMMON_ROUND_BUFF_HPP_
