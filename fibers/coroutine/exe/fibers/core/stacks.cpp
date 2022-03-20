#include <exe/fibers/core/stacks.hpp>

#include <vector>

using context::Stack;

namespace exe::fibers {

//////////////////////////////////////////////////////////////////////

class StackAllocator {
 public:
  Stack Allocate() {
    if (pool_.empty()) {
      return AllocateNewStack();
    }
    Stack stack = std::move(pool_.back());
    pool_.pop_back();
    return stack;
  }

  void Release(Stack stack) {
    pool_.push_back(std::move(stack));
  }

 private:
  static Stack AllocateNewStack() {
    static const size_t kStackPages = 16;  // 16 * 4KB = 64KB
    return Stack::AllocatePages(kStackPages);
  }

 private:
  std::vector<Stack> pool_;
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
