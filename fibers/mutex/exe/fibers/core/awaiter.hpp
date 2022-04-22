#pragma once

#include <exe/fibers/core/handle.hpp>
#include <exe/support/spinlock.hpp>

#include <wheels/intrusive/list.hpp>

namespace exe::fibers {

struct IAwaiter {
  virtual ~IAwaiter() = default;

  virtual void AfterSuspend() = 0;

  virtual void Schedule() = 0;
};

struct YieldAwaiter : IAwaiter {
  explicit YieldAwaiter(FiberHandle handler) : handler_(handler) {
  }

  void AfterSuspend() {
    handler_.Schedule();
  }

  void Schedule() {
    handler_.Schedule();
  }

 private:
  FiberHandle handler_;
};

struct FutexAwaiter : IAwaiter, public wheels::IntrusiveListNode<FutexAwaiter> {
  explicit FutexAwaiter(FiberHandle handler) : handler_(handler) {
    spinlock_.lock();
  }

  void AfterSuspend() {
    spinlock_.unlock();
  }

  void Schedule() {
    spinlock_.lock();
    handler_.Schedule();
  }

 private:
  FiberHandle handler_;
  exe::support::SpinLock spinlock_;
};

}  // namespace exe::fibers
