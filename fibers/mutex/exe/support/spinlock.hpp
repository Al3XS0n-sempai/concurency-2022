#pragma once

#include <twist/stdlike/atomic.hpp>
#include <wheels/support/cpu.hpp>

namespace exe::support {

// Test-and-TAS spinlock

class SpinLock {
 public:
  void Lock() {
    while (locked_.exchange(1) == 1) {
      wheels::SpinLockPause();
    }
  }

  void Unlock() {
    locked_.store(0);
  }

  // BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::stdlike::atomic<int> locked_;
};

}  // namespace exe::support
