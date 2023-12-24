//
// Created by SERG on 04.09.2020.
//

#ifndef SIGNAL_RECORDER_UTILS_STATIC_VECTOR_HPP_
#define SIGNAL_RECORDER_UTILS_STATIC_VECTOR_HPP_

#include <inttypes.h>
#include <memory.h>

namespace utils {

template<typename T>
struct IStaticVector {
  virtual bool Empty() const = 0;
  virtual size_t Size() const = 0;
  virtual bool Full() const = 0;
  virtual T* Data() = 0;
  virtual void PushBack(const T &value) = 0;
  virtual void Clear() = 0;
  virtual void Fill() = 0;
  virtual size_t Append(const T* data, size_t data_len) = 0;
  virtual size_t AppendSize(size_t size) = 0;
  virtual T* End() = 0;
  virtual size_t FreeSize() = 0;
};

template<typename T, size_t MaxSize>
class StaticVector : public IStaticVector<T> {
 public:
  [[nodiscard]]
  bool Empty() const override { return size_ == 0; };
  [[nodiscard]]
  size_t Size() const override { return size_; }
  T *Data() override { return buf_; }
  constexpr size_t Capacity() { return MaxSize; }
  [[nodiscard]]
  bool Full() const override { return size_ == MaxSize; }
  void PushBack(const T &value) override {
    if (size_ < MaxSize)
      buf_[size_++] = value;
  };
  void Clear() override { size_ = 0; }
  void Fill() override { memset(buf_, 0, MaxSize * sizeof(T)); }
  size_t Append(const T* data, size_t data_len) override;
  size_t AppendSize(size_t size) override;
  T* End() override { return buf_ + size_; };
  size_t FreeSize() override { return MaxSize - size_; }
  T& operator[](size_t pos) {return buf_[pos];}
 private:
  T buf_[MaxSize];
  size_t size_ = 0;
};
template<typename T, size_t MaxSize>
size_t StaticVector<T, MaxSize>::Append(const T *data, size_t data_len) {
  size_t append_len = data_len;
  if(size_+data_len > MaxSize) {
    append_len = MaxSize - size_;
  }
  if(append_len == 0)
    return 0;
  memcpy(&buf_[size_], data, append_len*sizeof(T));
  size_ += append_len;
  return append_len;
}
template<typename T, size_t MaxSize>
size_t StaticVector<T, MaxSize>::AppendSize(size_t data_len) {
  size_t append_len = data_len;
  if(size_+data_len > MaxSize) {
    append_len = MaxSize - size_;
  }
  if(append_len == 0)
    return 0;
  size_ += append_len;
  return append_len;
}

}

#endif //SIGNAL_RECORDER_UTILS_STATIC_VECTOR_HPP_
