#include <exe/fibers/core/stacks.hpp>

#include <twist/stdlike/mutex.hpp>
#include <vector>

using context::Stack;

namespace exe::fibers {

//////////////////////////////////////////////////////////////////////

class StackAllocator {
 public:
  Stack Allocate() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    if (!pool_.empty()) {
      Stack stack = std::move(pool_.back());
      pool_.pop_back();
      return stack;
    }
    lock.unlock();
    return AllocateNewStack();
  }

  void Release(Stack stack) {
    std::lock_guard<twist::stdlike::mutex> lock(mutex_);
    pool_.push_back(std::move(stack));
  }

 private:
  static Stack AllocateNewStack() {
    static const size_t kStackPages = 16;  // 16 * 4KB = 64KB
    return Stack::AllocatePages(kStackPages);
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
