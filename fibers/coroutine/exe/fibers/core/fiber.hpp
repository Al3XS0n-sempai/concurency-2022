#pragma once

#include <exe/fibers/core/api.hpp>
#include <exe/coroutine/impl.hpp>
#include <context/stack.hpp>

namespace exe::fibers {

// Fiber = Stackful coroutine + Scheduler (Thread pool)

class Fiber {
 public:
  explicit Fiber(Scheduler* scheduler, Routine routine);

  Scheduler& GetCurrentScheduler();

  // ~ System calls
  void Schedule();
  void Yield();

  static Fiber& Self();

 private:
  // Task
  void Step();

 private:
  context::Stack stack_;
  coroutine::CoroutineImpl coroutine_;
  Scheduler* scheduler_;
};

}  // namespace exe::fibers
