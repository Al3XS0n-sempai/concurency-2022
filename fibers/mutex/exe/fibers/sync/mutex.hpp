#pragma once

#include <exe/fibers/sync/futex.hpp>

#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

class Mutex {
 public:
  void Lock() {
    counter_.fetch_add(1);
    while (flag_.exchange(1) == 1) {
      futex_.ParkIfEqual(1);
    }
  }

  void Unlock() {
    flag_.store(0);
    if (counter_.fetch_sub(1) > 1) {
      futex_.WakeOne();
    }
  }

  // BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::stdlike::atomic<uint64_t> counter_{0};
  twist::stdlike::atomic<uint32_t> flag_{0};
  FutexLike<uint32_t> futex_{flag_};
};

}  // namespace exe::fibers
