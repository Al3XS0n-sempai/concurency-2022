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
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    arrived_++;
    if (arrived_ == thread_count_) {
      gen_ ^= 1;
      arrived_ = 0;
      all_arrived_.notify_all();
    } else {
      size_t my_gen = gen_;
      while (my_gen == gen_) {
        all_arrived_.wait(lock);
      }
      //      all_arrived_.wait(lock, [&] {
      //        return my_gen != gen_;
      //      });
    }
  }

 private:
  size_t thread_count_;
  size_t gen_{0}, arrived_{0};
  twist::stdlike::condition_variable all_arrived_;
  twist::stdlike::mutex mutex_;
};

}  // namespace solutions
