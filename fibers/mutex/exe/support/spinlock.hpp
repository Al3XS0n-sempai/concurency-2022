#pragma once

#include <twist/stdlike/atomic.hpp>

namespace exe::support {

// Test-and-TAS spinlock

class SpinLock {
 public:
  void Lock() {
    while (locked_.exchange(true)) {
    }
  }

  void Unlock() {
    locked_.store(false);
  }

  // BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::stdlike::atomic<bool> locked_{false};
};

}  // namespace exe::support
