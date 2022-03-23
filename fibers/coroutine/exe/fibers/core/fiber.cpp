#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>

#include <exe/tp/submit.hpp>

#include <csignal>

namespace exe::fibers {

twist::util::ThreadLocalPtr<Fiber> current;

//////////////////////////////////////////////////////////////////////

Fiber::Fiber(Scheduler* scheduler, Routine routine)
    : stack_(AllocateStack()),
      coroutine_(std::move(routine), stack_.View()),
      scheduler_(scheduler) {
}

void Fiber::Run() {
  tp::Submit(*scheduler_, [this]() {
    this->Step();
  });
}

void Fiber::Schedule(Routine routine) {
  Go(*scheduler_, std::move(routine));
}

void Fiber::Yield() {
  coroutine_.Suspend();
}

void Fiber::Step() {
  current = this;
  coroutine_.Resume();
  if (coroutine_.IsCompleted()) {
    ReleaseStack(std::move(this->stack_));
    delete this;
    return;
  }
  Run();
}

Fiber& Fiber::Self() {
  return *current;
}

//////////////////////////////////////////////////////////////////////

// API Implementation

void Go(Scheduler& scheduler, Routine routine) {
  Fiber* fiber = new Fiber(&scheduler, std::move(routine));
  //  tp::Submit(scheduler, [fiber] {
  fiber->Run();
  //  });
}

void Go(Routine routine) {
  Fiber::Self().Schedule(std::move(routine));
}

namespace self {

void Yield() {
  Fiber::Self().Yield();
}

}  // namespace self

}  // namespace exe::fibers
