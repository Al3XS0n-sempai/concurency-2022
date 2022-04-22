#pragma once

#include <twist/stdlike/atomic.hpp>
#include <wheels/intrusive/forward_list.hpp>
#include <optional>
#include <deque>
#include <wheels/logging/logging.hpp>

namespace exe::executors {

template <class T>
class LockFreeStack {
 private:
  struct Node : public wheels::IntrusiveForwardListNode<Node> {
   public:
    Node() = default;

    explicit Node(T new_item) : item(std::move(new_item)) {
    }

   public:
    T item;
  };

  using NodeBase = wheels::IntrusiveForwardListNode<Node>;

 public:
  LockFreeStack() = default;

  void Push(T item) {
    Node* new_node = new Node(std::move(item));
    new_node->next_ = head_.load(std::memory_order_relaxed);
    while (!head_.compare_exchange_weak(new_node->next_, new_node,
                                        std::memory_order_release,
                                        std::memory_order_relaxed)) {
    }
  }

  bool Empty() {
    return head_.load(std::memory_order_relaxed) == nullptr;
  }

  std::deque<T> PopAll() {
    std::deque<T> result;

    NodeBase* all = head_.exchange(nullptr, std::memory_order_acquire);
    NodeBase* remaining = all;

    while (remaining) {
      result.push_back(std::move(remaining->AsItem()->item));
      remaining = remaining->next_;
      delete all;
      all = remaining;
    }

    return result;
  }

 private:
  twist::stdlike::atomic<NodeBase*> head_{nullptr};
};

}  // namespace exe::executors