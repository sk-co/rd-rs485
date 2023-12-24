#ifndef MODEM_CMD_UTILS_ROUND_BUF_HPP_
#define MODEM_CMD_UTILS_ROUND_BUF_HPP_

#include <cinttypes>
#include <cstddef>
#include <cstring>

namespace utils {

// Циклический буффер.
template<typename T, size_t MaxSize>
class RoundBuf {
 public:
  [[nodiscard]]
  size_t Capacity() const {return MaxSize;}
  [[nodiscard]]
  size_t FreeSpace() const {return (MaxSize - Size());}
  void Reset() {
    write_ind_ = 0;
    read_ind_ = 0;
  }
  [[nodiscard]]
  size_t Size() const {
    if(read_ind_ <= write_ind_) {
      return write_ind_ - read_ind_;
    }
    return write_ind_ + (MaxSize - read_ind_);
  }
  [[nodiscard]]
  bool IsCanRead() const { return write_ind_ != read_ind_; }
  [[nodiscard]]
  bool IsEmpty() const { return read_ind_ == write_ind_; }
  [[nodiscard]]
  bool IsFull() const {
    auto next_ind = (write_ind_ == MaxSize-1) ? 0 : write_ind_ + 1;
    return next_ind == read_ind_;
  }
  // Запись элемента, если буфер полный, то не записывает и возвращает false.
  bool PushBack(const T &value) {
    size_t next_write = (write_ind_ == MaxSize-1) ? 0 : write_ind_ + 1;
    if(next_write == read_ind_) // полный, места нет.
      return false;
    buf_[write_ind_] = value;
    write_ind_ = next_write;
    return true;
  }
  // То же, что и PushBack
  bool Write(const T &value) {
    return PushBack(value);
  }
  // Запись элемента, если буфер полный, то перезаписывает самое старое значение.
  void WriteAnyway(const T &value) {
    buf_[write_ind_] = value;
    write_ind_ = (write_ind_ == MaxSize-1) ? 0 : write_ind_ + 1;
    // Если старые данные еще не считали, то затираем их и сдвигаем указатель.
    if(read_ind_ == write_ind_)
      read_ind_ = (read_ind_ == MaxSize-1) ? 0 : read_ind_ + 1;
  }
  // Считывает один элемент.
  void Read(T *value) {
    if(write_ind_ == read_ind_)
      return;
    *value = buf_[read_ind_];
    read_ind_ = (read_ind_ == MaxSize-1) ? 0 : read_ind_ + 1;
  }
  // Считывает один элемент.
  T Read() {
    if(write_ind_ == read_ind_)
      return T {};
    auto old_ind = read_ind_;
    read_ind_ = (read_ind_ == MaxSize-1) ? 0 : read_ind_ + 1;
    return buf_[old_ind];
  }
  // Считывает несколько элементов, возвращает реально прочитанное кол-во элементов.
  size_t Read(T *buf, size_t len) {
    if(write_ind_ == read_ind_)
      return 0;
    size_t current_size = Size();
    if(len > current_size) {
      // Считываем все.
      len = current_size;
    }
    if(read_ind_ < write_ind_) {
      memcpy(buf, &buf_[read_ind_], len*sizeof(T));
      read_ind_ += len;
    }
    else {
      size_t len_first = MaxSize - read_ind_;
      if(len_first >= len) {
        memcpy(buf, &buf_[read_ind_], len*sizeof(T));
        read_ind_ += len;
        if(read_ind_ == MaxSize)
          read_ind_ = 0;
        return len;
      }
      size_t len_second = len - len_first;
      memcpy(buf, &buf_[read_ind_], len_first*sizeof(T));
      memcpy(buf + len_first, buf_, len_second*sizeof(T));
      read_ind_ = len_second;
    }
    return len;
  }
  // Считывает данные без изменения указателей. Возвращает реально прочитанное кол-во элементов.
  size_t ReadCopy(T *buf, size_t len) {
    if(read_ind_ == write_ind_)
      return 0;
    size_t current_size = Size();
    if(len > current_size) {
      // Считываем все.
      len = current_size;
    }
    if(read_ind_ < write_ind_) {
      memcpy(buf, &buf_[read_ind_], len*sizeof(T));
    }
    else {
      size_t len_first = MaxSize - read_ind_;
      if(len_first >= len) {
        memcpy(buf, &buf_[read_ind_], len*sizeof(T));
        return len;
      }
      size_t len_second = len - len_first;
      memcpy(buf, &buf_[read_ind_], len_first*sizeof(T));
      memcpy(buf + len_first, buf_, len_second*sizeof(T));
    }
    return len;
  }
  T* Data() {
    return buf_;
  }
 private:
  T buf_[MaxSize];
  volatile size_t write_ind_ = 0;
  volatile size_t read_ind_ = 0;
};
}

#endif //MODEM_CMD_UTILS_ROUND_BUF_HPP_
