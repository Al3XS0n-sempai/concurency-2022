#pragma once

#include <exe/fibers/core/api.hpp>
#include <exe/support/spinlock.hpp>
#include <exe/fibers/core/fiber.hpp>

#include <twist/stdlike/atomic.hpp>

#include <wheels/intrusive/list.hpp>
#include <wheels/support/assert.hpp>

#include <wheels/logging/logging.hpp>

namespace exe::fibers {

template <typename T>
class FutexLike {
 public:
  explicit FutexLike(twist::stdlike::atomic<T>& cell) : cell_(cell) {
  }

  ~FutexLike() {
    assert(queue_.IsEmpty());
  }

  // Park current fiber if cell.load() == `old`
  void ParkIfEqual(T old) {
    LOG_SIMPLE("old is equal to " << old)
    std::unique_lock lock(spinlock_);
    if (old != cell_.load()) {
      return;
    }
    FutexAwaiter awaiter(Fiber::Self().GetHandler());
    queue_.PushBack(&awaiter);
    lock.unlock();

    self::Suspend(&awaiter);
  }

  void WakeOne() {
    std::lock_guard lock(spinlock_);
    if (queue_.IsEmpty()) {
      return;
    }
    queue_.PopFront()->Schedule();
  }

  void WakeAll() {
    std::lock_guard lock(spinlock_);
    while (!queue_.IsEmpty()) {
      queue_.PopFront()->Schedule();
    }
  }

 private:
  exe::support::SpinLock spinlock_;
  wheels::IntrusiveList<IAwaiter> queue_;
  twist::stdlike::atomic<T>& cell_;
};

}  // namespace exe::fibers
