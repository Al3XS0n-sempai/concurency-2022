#pragma once

#include <twist/stdlike/condition_variable.hpp>
#include <twist/stdlike/mutex.hpp>

namespace exe::tp {
class WaitCounter {
 public:
  WaitCounter() = default;

  ~WaitCounter() = default;

  void Increment() {
    std::lock_guard lock(mutex_);
    counter_++;
  }

  void Decrement() {
    std::lock_guard lock(mutex_);
    counter_--;
    if (counter_ == 0) {
      wait_for_null_.notify_one();
    }
  }

  void WaitForZero() {
    std::unique_lock lock(mutex_);
    while (counter_ > 0) {
      wait_for_null_.wait(lock);
    }
  }

 private:
  size_t counter_{0};
  twist::stdlike::condition_variable wait_for_null_;
  twist::stdlike::mutex mutex_;
};
}  // namespace exe::tp