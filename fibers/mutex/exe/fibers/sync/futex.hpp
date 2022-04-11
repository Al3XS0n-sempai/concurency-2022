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
    spinlock_.lock();
    if (old != cell_.load()) {
      spinlock_.unlock();
      return;
    }
    FutexAwaiter awaiter(Fiber::Self().GetHandler());
    queue_.PushBack(&awaiter);
    spinlock_.unlock();

    self::Suspend(&awaiter);
  }

  void WakeOne() {
    std::lock_guard lock(spinlock_);
    if (queue_.IsEmpty()) {
      return;
    }
    FutexAwaiter* awaiter = queue_.PopFront();
    (*awaiter).Schedule();
  }

  void WakeAll() {
    std::lock_guard lock(spinlock_);
    while (!queue_.IsEmpty()) {
      queue_.PopFront()->Schedule();
    }
  }

 private:
  exe::support::SpinLock spinlock_;
  wheels::IntrusiveList<FutexAwaiter> queue_;
  twist::stdlike::atomic<T>& cell_;
};

}  // namespace exe::fibers
