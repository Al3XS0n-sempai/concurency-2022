#pragma once

#include <twist/stdlike/atomic.hpp>
#include <wait_queue.hpp>

#include <cstdlib>

namespace stdlike {

class Mutex {
 public:
  void Lock() {
    counter_.fetch_add(1);
    while (flag_.exchange(1) == 1) {
      flag_.wait(1);
    }
  }

  void Unlock() {
    flag_.store(0);
    if (counter_.fetch_sub(1) > 1) {
      flag_.notify_one();
    }
  }

 private:
  twist::stdlike::atomic<uint32_t> flag_{0};
  twist::stdlike::atomic<uint64_t> counter_{0};
};

}  // namespace stdlike