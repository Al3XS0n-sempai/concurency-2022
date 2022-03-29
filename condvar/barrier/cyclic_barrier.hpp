#pragma once

#include <iostream>

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

// std::lock_guard, std::unique_lock
#include <mutex>
#include <cstdint>

namespace solutions {

// CyclicBarrier allows a set of threads to all wait for each other
// to reach a common barrier point

// The barrier is called cyclic because
// it can be re-used after the waiting threads are released.

class CyclicBarrier {
 public:
  explicit CyclicBarrier(size_t thread_count) : thread_count_(thread_count) {
  }

  // Blocks until all participants have invoked Arrive()
  void Arrive() {
    std::unique_lock lock(mutex_);
    arrived_++;
    if (arrived_ == thread_count_) {
      epoch_ ^= 1;
      arrived_ = 0;
      all_arrived_.notify_all();
    } else {
      const size_t my_epoch = epoch_;
      while (my_epoch == epoch_) {
        all_arrived_.wait(lock);
      }
    }
  }

 private:
  const size_t thread_count_;
  size_t epoch_{0};
  size_t arrived_{0};
  twist::stdlike::condition_variable all_arrived_;
  twist::stdlike::mutex mutex_;
};

}  // namespace solutions
