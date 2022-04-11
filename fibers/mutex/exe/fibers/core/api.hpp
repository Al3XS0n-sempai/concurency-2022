#pragma once

#include <exe/coroutine/routine.hpp>
#include <exe/tp/thread_pool.hpp>
#include <exe/fibers/core/awaiter.hpp>

namespace exe::fibers {

using Routine = coroutine::Routine;

using Scheduler = tp::ThreadPool;

// Considered harmful

// Starts a new fiber
void Go(Scheduler& scheduler, Routine routine);

// Starts a new fiber in the current scheduler
void Go(Routine routine);

namespace self {

void Yield();

// For synchronization primitives
// Do not use directly
void Suspend(IAwaiter* awaiter);

}  // namespace self

}  // namespace exe::fibers
