#pragma once

#include <twist/stdlike/atomic.hpp>

#include <cstdint>

namespace stdlike {

class CondVar {
 public:
  // Mutex - BasicLockable
  // https://en.cppreference.com/w/cpp/named_req/BasicLockable
  template <class Mutex>
  void Wait(Mutex& lock) {
    uint32_t val = counter_.load();
    lock.unlock();
    counter_.wait(val);
    lock.lock();
  }

  void NotifyOne() {
    counter_.fetch_add(1);
    counter_.notify_one();
  }

  void NotifyAll() {
    counter_.fetch_add(1);
    counter_.notify_all();
  }

 private:
  twist::stdlike::atomic<uint32_t> counter_{0};
};

}  // namespace stdlike
