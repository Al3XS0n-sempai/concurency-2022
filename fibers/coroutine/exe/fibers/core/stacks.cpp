#include <exe/fibers/core/stacks.hpp>

#include <optional>
#include <vector>
#include <twist/stdlike/mutex.hpp>

using context::Stack;

namespace exe::fibers {

//////////////////////////////////////////////////////////////////////

class StackAllocator {
 public:
  Stack Allocate() {
    std::optional<Stack> stack = TryGetStackFromPool();
    return stack ? std::move(stack.value()) : AllocateNewStack();
  }

  void Release(Stack stack) {
    std::lock_guard lock(mutex_);
    pool_.push_back(std::move(stack));
  }

 private:
  static Stack AllocateNewStack() {
    static const size_t kStackPages = 16;  // 16 * 4KB = 64KB
    return Stack::AllocatePages(kStackPages);
  }

  std::optional<Stack> TryGetStackFromPool() {
    std::lock_guard lock(mutex_);
    if (pool_.empty()) {
      return std::nullopt;
    }
    Stack stack = std::move(pool_.back());
    pool_.pop_back();
    return stack;
  }

 private:
  std::vector<Stack> pool_;
  twist::stdlike::mutex mutex_;
};

//////////////////////////////////////////////////////////////////////

StackAllocator allocator;

context::Stack AllocateStack() {
  return allocator.Allocate();
}

void ReleaseStack(context::Stack stack) {
  allocator.Release(std::move(stack));
}

}  // namespace exe::fibers
