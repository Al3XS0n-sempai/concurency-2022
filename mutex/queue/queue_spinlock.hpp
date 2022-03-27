#pragma once

#include <twist/stdlike/atomic.hpp>
#include <twist/util/spin_wait.hpp>

namespace spinlocks {

/*  Scalable Queue SpinLock
 *
 *  Usage:
 *
 *  QueueSpinLock qspinlock;
 *  {
 *    QueueSpinLock::Guard guard(qspinlock);  // <-- Acquire
 *    // Critical section
 *  }  // <-- Release
 */

class QueueSpinLock {
  struct Node {
    twist::stdlike::atomic<bool> owner{false};
    twist::stdlike::atomic<Node*> next{nullptr};
  };

 public:
  class Guard {
    friend class QueueSpinLock;

   public:
    explicit Guard(QueueSpinLock& spinlock) : spinlock_(spinlock), node_() {
      node_ = new Node;
      spinlock_.Acquire(this);
    }

    ~Guard() {
      spinlock_.Release(this);
      delete node_;
    }

   private:
    QueueSpinLock& spinlock_;
    Node* node_;
    twist::util::SpinWait spinner_;
  };

 private:
  void Acquire(Guard* guard) {
    Node* prev_tail = tail_.exchange(guard->node_);

    if (prev_tail != nullptr) {
      prev_tail->next.store(guard->node_);
      while (!guard->node_->owner.load()) {
        guard->spinner_.Spin();
      }
    }
  }

  void Release(Guard* guard) {
    Node* expected = guard->node_;
    if (tail_.compare_exchange_strong(expected, nullptr)) {
      return;
    }

    Node* next_in_queue = guard->node_->next.load();

    while (next_in_queue == nullptr) {
      next_in_queue = guard->node_->next.load();
    }

    next_in_queue->owner.store(true);
  }

 private:
  twist::stdlike::atomic<Node*> tail_{nullptr};
};

}  // namespace spinlocks
