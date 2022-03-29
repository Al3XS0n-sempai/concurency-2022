#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>

#include <asio/steady_timer.hpp>

namespace exe::fibers {

//////////////////////////////////////////////////////////////////////

twist::util::ThreadLocalPtr<Fiber> current_fiber;

Fiber::Fiber(Scheduler* scheduler, Routine routine)
    : scheduler_(scheduler),
      stack_(AllocateStack()),
      routine_(std::move(routine), stack_.View()) {
}

Scheduler* Fiber::GetCurrentScheduler() {
  return scheduler_;
}

void Fiber::Schedule() {
  scheduler_->post([this] {
    this->Step();
  });
}

void Fiber::Yield() {
  routine_.Suspend();
}

void Fiber::Suspend() {
  suspended_ = true;
  routine_.Suspend();
}

void Fiber::SleepFor(Millis delay) {
  asio::steady_timer timer{*scheduler_, delay};
  timer_ = &timer;
  Suspend();
  timer_ = nullptr;
}

void Fiber::Resume() {
  suspended_ = false;
  scheduler_->post([this] {
    this->Dispatch();
  });
}

void Fiber::Step() {
  current_fiber = this;
  routine_.Resume();
  current_fiber = nullptr;
  scheduler_->post([this] {
    this->Dispatch();
  });
}

Fiber& Fiber::Self() {
  return *current_fiber;
}

void Fiber::Dispatch() {
  if (suspended_) {
    timer_->async_wait([this](std::error_code) {
      this->Resume();
    });
    return;
  }
  if (routine_.IsCompleted()) {
    ReleaseStack(std::move(stack_));
    delete this;
  } else {
    Schedule();
  }
}

//////////////////////////////////////////////////////////////////////

// API Implementation

void Go(Scheduler& scheduler, Routine routine) {
  Fiber* fiber = new Fiber(&scheduler, std::move(routine));
  fiber->Schedule();
}

void Go(Routine routine) {
  Go(*Fiber::Self().GetCurrentScheduler(), std::move(routine));
}

namespace self {

void Yield() {
  Fiber::Self().Yield();
}

void SleepFor(Millis delay) {
  Fiber::Self().SleepFor(delay);
}

}  // namespace self

}  // namespace exe::fibers
