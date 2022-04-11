#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>

#include <wheels/logging/logging.hpp>

namespace exe::fibers {

twist::util::ThreadLocalPtr<Fiber> current_fiber;

//////////////////////////////////////////////////////////////////////

Fiber::Fiber(Scheduler* scheduler, Routine routine)
    : scheduler_(scheduler),
      stack_(AllocateStack()),
      routine_(std::move(routine), stack_.View()) {
}

Scheduler& Fiber::GetCurrentScheduler() {
  return *scheduler_;
}

void Fiber::SetAwaiter(IAwaiter* awaiter) {
  awaiter_ = awaiter;
}

FiberHandle Fiber::GetHandler() {
  return FiberHandle(this);
}

void Fiber::Schedule() {
  scheduler_->Submit([&] {
    Step();
  });
}

void Fiber::Yield() {
  YieldAwaiter awaiter(GetHandler());
  SetAwaiter(&awaiter);
  Suspend();
}

void Fiber::Step() {
  current_fiber = this;
  routine_.Resume();
  if (routine_.IsCompleted()) {
    ReleaseStack(std::move(stack_));
    delete this;
    return;
  }

  awaiter_->AfterSuspend();
  current_fiber = nullptr;
}

Fiber& Fiber::Self() {
  return *current_fiber;
}

void Fiber::Suspend() {
  routine_.Suspend();
}

void Fiber::Resume() {
  //  awaiter_ = nullptr;
  //  Schedule();
}

//////////////////////////////////////////////////////////////////////

// API Implementation

void Go(Scheduler& scheduler, Routine routine) {
  auto fiber = new Fiber(&scheduler, std::move(routine));
  fiber->Schedule();
}

void Go(Routine routine) {
  Go(Fiber::Self().GetCurrentScheduler(), std::move(routine));
}

namespace self {

void Yield() {
  Fiber::Self().Yield();
}

void Suspend(IAwaiter* awaiter) {
  Fiber::Self().SetAwaiter(awaiter);
  Fiber::Self().Suspend();
}

}  // namespace self

}  // namespace exe::fibers
