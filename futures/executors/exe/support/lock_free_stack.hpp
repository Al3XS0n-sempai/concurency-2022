#pragma once

#include <twist/stdlike/atomic.hpp>

#include <optional>

namespace exe::executors {

template <class T>
class LockFreeStack {
 private:
  struct Node {
   public:
    Node() = default;

    explicit Node(T new_item) : item(std::move(new_item)) {
    }

   public:
    Node* next{nullptr};
    T item;
  };

 public:
  LockFreeStack() = default;

  explicit LockFreeStack(Node* head) : head_{head} {
  }

  void Push(T item) {
    Node* new_node = new Node(std::move(item));
    new_node->next = head_.load(std::memory_order_acquire);
    while (!head_.compare_exchange_weak(new_node->next, new_node,
                                        std::memory_order_acq_rel)) {
    }
  }

  std::optional<T> Pop() {
    Node* head = head_.load(std::memory_order_acquire);

    if (head == nullptr) {
      return std::nullopt;
    }

    Node* new_head = head->next;
    while (!head_.compare_exchange_weak(head, new_head,
                                        std::memory_order_acq_rel)) {
      if (head == nullptr) {
        return std::nullopt;
      }

      new_head = head->next;
    }

    T top_item = std::move(head->item);
    delete head;

    return top_item;
  }

  bool Empty() {
    return head_ == nullptr;
  }

  void MoveReversed(LockFreeStack<T>& another) {
    assert(another.head_.load(std::memory_order_acquire) == nullptr);
    another.head_.store(head_.exchange(nullptr, std::memory_order_acq_rel),
                        std::memory_order_release);
    another.Reverse();
  }

 private:
  void Reverse() {
    Node* prev = nullptr;
    Node* next = nullptr;
    Node* cur = head_.load(std::memory_order_acquire);

    while (cur != nullptr) {
      next = cur->next;

      cur->next = prev;
      prev = cur;
      cur = next;
    }

    head_.store(prev, std::memory_order_release);
  }

 private:
  twist::stdlike::atomic<Node*> head_{nullptr};
};

}  // namespace exe::executors