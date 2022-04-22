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
  class Guard : public Node {
    friend class QueueSpinLock;

   public:
    explicit Guard(QueueSpinLock& spinlock) : spinlock_(spinlock) {
      spinlock_.Acquire(this);
    }

    ~Guard() {
      spinlock_.Release(this);
    }

   private:
    QueueSpinLock& spinlock_;
    twist::util::SpinWait spinner_;
  };

 private:
  void Acquire(Guard* guard) {
    Node* prev_tail = tail_.exchange(guard);

    if (prev_tail != nullptr) {
      prev_tail->next.store(guard);
      while (!guard->owner.load()) {
        guard->spinner_.Spin();
      }
    }
  }

  void Release(Guard* guard) {
    Node* expected = guard;
    if (tail_.compare_exchange_strong(expected, nullptr)) {
      return;
    }

    while (guard->next.load() == nullptr) {
    }

    guard->next.load()->owner.store(true);
  }

 private:
  twist::stdlike::atomic<Node*> tail_{nullptr};
};

}  // namespace spinlocks
