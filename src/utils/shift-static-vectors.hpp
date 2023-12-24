//
// Created by SERG on 04.09.2020.
//

#ifndef SIGNAL_RECORDER_UTILS_ROUND_BUFFERS_H_
#define SIGNAL_RECORDER_UTILS_ROUND_BUFFERS_H_

#include "static-vector.hpp"

namespace utils {

template<typename T, size_t MaxSize, size_t BuffersCount>
class ShiftStaticVectors {
 public:
  using Buffer = StaticVector<T, MaxSize>;
  ShiftStaticVectors() {
    static_assert(BuffersCount >= 2, "BuffersCount need 2 or more.");
    Reset();
  }
  static size_t GetMaxSize() {return MaxSize;}
  void Reset();
  [[nodiscard]]
  bool IsCanRead() const { return read_buffer_ != write_buffer_; }
  [[nodiscard]] size_t ReadSize() const { return read_buffer_->Size(); }
  [[nodiscard]] size_t WriteSize() const { return write_buffer_->Size(); }
  T *ReadData() { return read_buffer_->Data(); }
  void MoveReadToNext();
  void MoveWriteToNext();
  [[nodiscard]]
  bool IsFull() const {
    auto next_buf = (write_buffer_ == &buffers_[BuffersCount-1]) ? &buffers_[0] :
      write_buffer_ + 1;
    return next_buf == read_buffer_;
  }
  bool PushBack(const T &value) {
//    if (IsFull())
//      return false;
    if (!write_buffer_->Full()) {
      write_buffer_->PushBack(value);
      return true;
    }
    return false;
  }
  bool Append(const T* data, size_t data_len) {
//    if (IsFull())
//      return false;
    if(write_buffer_->Size() + data_len > write_buffer_->Capacity())
      return false;
    return (write_buffer_->Append(data, data_len) == data_len);
  }
  bool AppendSize(size_t data_len) {
    if(write_buffer_->Size() + data_len > write_buffer_->Capacity())
      return false;
    return (write_buffer_->AppendSize(data_len) == data_len);
  }
  IStaticVector<T>* GetReadVector() {return read_buffer_;}
  IStaticVector<T>* GetWriteVector() {return write_buffer_;};
 private:
  void MoveBufferPtr(Buffer **buffer_ptr);
 private:
  Buffer buffers_[BuffersCount];
  Buffer *write_buffer_ = nullptr;
  Buffer *read_buffer_ = nullptr;
};
template<typename T, size_t MaxSize, size_t BuffersCount>
void ShiftStaticVectors<T, MaxSize, BuffersCount>::Reset() {
  write_buffer_ = &buffers_[0];
  read_buffer_ = &buffers_[0];
  write_buffer_->Clear();
}
template<typename T, size_t MaxSize, size_t BuffersCount>
void ShiftStaticVectors<T, MaxSize,
                        BuffersCount>::MoveBufferPtr(ShiftStaticVectors::Buffer **buffer_ptr) {
  if (*buffer_ptr == &buffers_[BuffersCount - 1])
    *buffer_ptr = &buffers_[0];
  else
    (*buffer_ptr)++;
}
template<typename T, size_t MaxSize, size_t BuffersCount>
void ShiftStaticVectors<T, MaxSize, BuffersCount>::MoveReadToNext() {
  // Если буффер не заполнен, то перемещать не надо.
  if(write_buffer_ == read_buffer_)
    return;
  read_buffer_->Clear();
  MoveBufferPtr(&read_buffer_);
}
template<typename T, size_t MaxSize, size_t BuffersCount>
void ShiftStaticVectors<T, MaxSize, BuffersCount>::MoveWriteToNext() {
  auto next_buf = (write_buffer_ == &buffers_[BuffersCount-1]) ? &buffers_[0] :
                  write_buffer_ + 1;
  if(next_buf == read_buffer_)
    return;
  write_buffer_ = next_buf;
  write_buffer_->Clear();
}

}

#endif //SIGNAL_RECORDER_UTILS_ROUND_BUFFERS_H_
