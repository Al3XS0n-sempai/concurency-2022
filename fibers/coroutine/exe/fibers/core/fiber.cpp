#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>

#include <exe/tp/submit.hpp>

#include <csignal>

namespace exe::fibers {

twist::util::ThreadLocalPtr<Fiber> current;

//////////////////////////////////////////////////////////////////////

Fiber::Fiber(Scheduler& scheduler, Routine routine)
    : stack_(AllocateStack()),
      coroutine_(std::move(routine), stack_.View()),
      scheduler_(scheduler) {
}

Fiber::~Fiber() {
  ReleaseStack(std::move(stack_));
}

Scheduler& Fiber::GetCurrentScheduler() {
  return scheduler_;
}

void Fiber::Schedule() {
  scheduler_.Submit([this] {
    Step();
  });
}

void Fiber::Yield() {
  coroutine_.Suspend();
}

void Fiber::Step() {
  current = this;
  coroutine_.Resume();
  current = nullptr;
  if (coroutine_.IsCompleted()) {
    delete this;
    return;
  }
  Schedule();
}

Fiber& Fiber::Self() {
  return *current;
}

//////////////////////////////////////////////////////////////////////

// API Implementation

void Go(Scheduler& scheduler, Routine routine) {
  Fiber* fiber = new Fiber(scheduler, std::move(routine));
  fiber->Schedule();
}

void Go(Routine routine) {
  Go(Fiber::Self().GetCurrentScheduler(), std::move(routine));
}

namespace self {

void Yield() {
  Fiber::Self().Yield();
}

}  // namespace self

}  // namespace exe::fibers
