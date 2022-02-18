#pragma once

#include <twist/stdlike/atomic.hpp>

#include <cstdlib>

namespace stdlike {

class Mutex {
 public:
  explicit Mutex() : flag_(0u) {
  }

  void Lock() {
    while (flag_.exchange(1) == 1) {
      flag_.FutexWait(1);
    }
  }

  void Unlock() {
    flag_.store(0);
    flag_.FutexWakeOne();
  }

 private:
  twist::stdlike::atomic<uint32_t> flag_;
};

}  // namespace stdlike
