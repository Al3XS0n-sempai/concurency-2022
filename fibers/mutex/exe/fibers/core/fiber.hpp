#pragma once

#include <exe/fibers/core/api.hpp>
#include <exe/fibers/core/handle.hpp>
#include <exe/fibers/core/awaiter.hpp>

#include <exe/coroutine/impl.hpp>

#include <context/stack.hpp>

namespace exe::fibers {

// Fiber = Stackful coroutine + Scheduler (Thread pool)

class Fiber {
 public:
  explicit Fiber(Scheduler* scheduler, Routine routine);

  Scheduler& GetCurrentScheduler();

  FiberHandle GetHandler();

  void SetAwaiter(IAwaiter* awaiter);

  void Schedule();

  void Yield();

  void Suspend();
  void Resume();

  static Fiber& Self();

 private:
  // Task
  void Step();

 private:
  IAwaiter* awaiter_{nullptr};
  Scheduler* scheduler_;
  context::Stack stack_;
  coroutine::CoroutineImpl routine_;
};

}  // namespace exe::fibers
