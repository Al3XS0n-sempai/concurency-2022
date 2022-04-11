#pragma once

#include <exe/fibers/sync/mutex.hpp>
#include <exe/fibers/sync/futex.hpp>

#include <twist/stdlike/atomic.hpp>

// std::unique_lock
#include <mutex>

namespace exe::fibers {

class CondVar {
  using Lock = std::unique_lock<Mutex>;

 public:
  void Wait(Lock& lock) {
    uint32_t val = counter_.load();
    lock.unlock();
    futex_.ParkIfEqual(val);
    lock.lock();
  }

  void NotifyOne() {
    counter_.fetch_add(1);
    futex_.WakeOne();
  }

  void NotifyAll() {
    counter_.fetch_add(1);
    futex_.WakeAll();
  }

 private:
  twist::stdlike::atomic<uint32_t> counter_{0};
  FutexLike<uint32_t> futex_{counter_};
};

}  // namespace exe::fibers
