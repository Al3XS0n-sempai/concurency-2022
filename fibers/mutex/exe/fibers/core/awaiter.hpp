#pragma once

#include <exe/fibers/core/handle.hpp>
#include <exe/support/spinlock.hpp>

#include <wheels/intrusive/list.hpp>

namespace exe::fibers {

struct IAwaiter : public wheels::IntrusiveListNode<IAwaiter> {
  explicit IAwaiter(FiberHandle handler) : handler_(handler) {
  }

  virtual void AfterSuspend() = 0;

  virtual void Schedule() = 0;

 protected:
  FiberHandle handler_;
};

struct YieldAwaiter : IAwaiter {
  explicit YieldAwaiter(FiberHandle handler) : IAwaiter(handler) {
  }

  void AfterSuspend() {
    handler_.Schedule();
  }

  void Schedule() {
    handler_.Schedule();
  }
};

struct FutexAwaiter : IAwaiter {
  explicit FutexAwaiter(FiberHandle handler) : IAwaiter(handler) {
    spinlock_.lock();
  }

  void AfterSuspend() {
    spinlock_.unlock();
  }

  void Schedule() {
    std::lock_guard lock(spinlock_);
    handler_.Schedule();
  }

 private:
  exe::support::SpinLock spinlock_;
};

}  // namespace exe::fibers
