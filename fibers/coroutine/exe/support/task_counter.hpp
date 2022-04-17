#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

namespace exe::tp {

class TaskCounter {
 public:
  void Increment() {
    std::lock_guard lock(mutex_);
    counter_++;
  }

  void Decrement() {
    std::lock_guard lock(mutex_);
    counter_--;
    if (counter_ == 0) {
      counter_is_not_null_.notify_one();
    }
  }

  void WaitForZero() {
    std::unique_lock lock(mutex_);
    while (counter_ != 0) {
      counter_is_not_null_.wait(lock);
    }
  }

 private:
  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable counter_is_not_null_;
  int counter_{0};
};
}  // namespace exe::tp
