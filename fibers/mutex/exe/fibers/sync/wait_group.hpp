#pragma once

#include <exe/fibers/sync/futex.hpp>
#include <exe/fibers/sync/mutex.hpp>
#include <exe/fibers/sync/condvar.hpp>

#include <twist/stdlike/atomic.hpp>

#include <wheels/logging/logging.hpp>

namespace exe::fibers {

// https://gobyexample.com/waitgroups

class WaitGroup {
  struct Holder {
    explicit Holder(twist::stdlike::atomic<int>& flag,
                    exe::fibers::Mutex& mutex)
        : mutex_(mutex), flag_(flag) {
      flag_.fetch_add(1);
      mutex_.lock();
    }

    ~Holder() {
      mutex_.unlock();
      flag_.fetch_sub(1);
    }

   private:
    exe::fibers::Mutex& mutex_;
    twist::stdlike::atomic<int>& flag_;
  };

 public:
  ~WaitGroup() {
    while (flag_.load() > 0) {
    }
  }

  void Add(size_t counter) {
    Holder hold(flag_, mutex_);

    counter_.fetch_add(counter);
    if (counter_.load() == 0) {
      cv_.NotifyAll();
    }
  }

  void Done() {
    Holder hold(flag_, mutex_);
    counter_.fetch_sub(1);
    if (counter_.load() == 0) {
      cv_.NotifyAll();
    }
  }

  void Wait() {
    std::unique_lock lock(mutex_);
    while (counter_.load() != 0) {
      cv_.Wait(lock);
    }
  }

 private:
  twist::stdlike::atomic<int> counter_{0};
  twist::stdlike::atomic<int> flag_{0};
  exe::fibers::Mutex mutex_;
  exe::fibers::CondVar cv_;
};

}  // namespace exe::fibers
