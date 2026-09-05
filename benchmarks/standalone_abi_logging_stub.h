#pragma once

// Parsing-only logging backend for standalone target-ABI probes. Normal Bazel
// and platform builds select their real backend and never include this file.
#include <stddef.h>

class StreamBase {
 public:
  StreamBase(char* buffer, size_t capacity)
      : buf_(buffer), capacity_(capacity) {}

  template <typename T>
  void print(T, int) {}
  template <typename... Args>
  void printf(const char*, Args...) {}
  void write(unsigned char) {}
  int number_base() const { return 10; }
  void setBase(int) {}
  int ctr() const { return 0; }
  size_t pcount() const { return pos_; }
  size_t remaining_capacity() const { return capacity_ - pos_; }

  char* buf_;
  size_t pos_ = 0;

 private:
  size_t capacity_;
};
