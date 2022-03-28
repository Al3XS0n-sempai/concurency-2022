#pragma once

#include <exe/fibers/core/api.hpp>
#include <exe/coroutine/impl.hpp>

#include <context/stack.hpp>

#include <asio/steady_timer.hpp>

namespace exe::fibers {

// Fiber = Stackful coroutine + Scheduler

class Fiber {
 public:
  // ~ System calls
  explicit Fiber(Scheduler* scheduler, Routine routine);

  Scheduler* GetCurrentScheduler();

  void Schedule();

  void Yield();
  void SleepFor(Millis delay);

  void Suspend();
  void Resume();

  static Fiber& Self();

 private:
  // Task
  void Step();

  void Dispatch();

 private:
  Scheduler* scheduler_;
  context::Stack stack_;
  bool suspended_{false};
  asio::steady_timer* timer_{nullptr};
  coroutine::CoroutineImpl routine_;
};

}  // namespace exe::fibers
