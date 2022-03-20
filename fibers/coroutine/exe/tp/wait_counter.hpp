#pragma once

#include <twist/stdlike/condition_variable.hpp>
#include <twist/stdlike/mutex.hpp>

namespace exe::tp {
class WaitCounter {
 public:
  WaitCounter() = default;

  ~WaitCounter() = default;

  void Inc() {
    std::lock_guard<twist::stdlike::mutex> lock(mutex_);
    counter_++;
  }

  void Dec() {
    std::lock_guard<twist::stdlike::mutex> lock(mutex_);
    counter_--;
    if (counter_ == 0) {
      wait_for_null_.notify_one();
    }
  }

  void WaitForNull() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
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