#pragma once

#include <exe/fibers/core/handle.hpp>
#include <exe/support/spinlock.hpp>

#include <wheels/intrusive/list.hpp>

namespace exe::fibers {

struct IAwaiter {
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

struct FutexAwaiter : IAwaiter, public wheels::IntrusiveListNode<FutexAwaiter> {
  explicit FutexAwaiter(FiberHandle handler) : IAwaiter(handler) {
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
  exe::support::SpinLock spinlock_;
};

}  // namespace exe::fibers
