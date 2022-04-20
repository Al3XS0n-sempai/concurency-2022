#pragma once

#include <exe/support/lock_free_stack.hpp>

namespace exe::executors {

template <typename T>
class MPSCQueue {
 public:
  MPSCQueue() = default;

  void Push(T item) {
    push_stack_.Push(std::move(item));
  }

  void FillPop() {
    if (pop_stack_.Empty()) {
      push_stack_.MoveReversed(pop_stack_);
    }
  }

  std::optional<T> Pop() {
    return pop_stack_.Pop();
  }

 private:
  LockFreeStack<T> push_stack_;
  LockFreeStack<T> pop_stack_;
};
}  // namespace exe::executors
