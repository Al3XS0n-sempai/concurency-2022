#pragma once

#include "tagged_semaphore.hpp"

#include <deque>

namespace solutions {

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BlockingQueue {
 public:
  explicit BlockingQueue(size_t count_of_slots) : free_slots_(count_of_slots) {
  }

  // Inserts the specified element into this queue,
  // waiting if necessary for space to become available.
  void Put(T value) {
    auto slot = free_slots_.Acquire();
    Push(std::move(value));
    acquired_slots_.Release(std::move(slot));
  }

  // Retrieves and removes the head of this queue,
  // waiting if necessary until an element becomes available
  T Take() {
    auto slot = acquired_slots_.Acquire();
    T front_element = Pop();
    free_slots_.Release(std::move(slot));
    return front_element;
  }

 private:
  struct Owner {};

  struct Slot {};

  void Push(T value) {
    auto guard = mutex_.MakeGuard();
    queue_.push_back(std::move(value));
  }

  T Pop() {
    auto guard = mutex_.MakeGuard();
    T front_element = std::move(queue_.front());
    queue_.pop_front();
    return front_element;
  }

  std::deque<T> queue_;
  TaggedSemaphore<Owner> mutex_{1};
  TaggedSemaphore<Slot> free_slots_;
  TaggedSemaphore<Slot> acquired_slots_{0};
};

}  // namespace solutions
