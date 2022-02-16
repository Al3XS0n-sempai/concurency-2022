#pragma once

#include <twist/stdlike/atomic.hpp>

#include <cstdlib>

namespace stdlike {

class Mutex {
 public:
  explicit Mutex() : flag_(0u) {
  }

  void Lock() {
    uint32_t need = 0u;
    flag_.compare_exchange_strong(need, 1);
    if (need != 0) {
      do {
        uint32_t expected = 1u;
        flag_.compare_exchange_strong(expected, 2);
        if (need == 2 || expected != 0) {
          flag_.FutexWait(2);
        }
        need = 0;
        flag_.compare_exchange_strong(need, 2);
      } while (need != 0);
    }
  }

  void Unlock() {
    if (flag_.fetch_sub(1) != 1) {
      flag_.store(0);
      flag_.FutexWakeOne();
    }
  }

 private:
  twist::stdlike::atomic<uint32_t> flag_;
};

}  // namespace stdlike
