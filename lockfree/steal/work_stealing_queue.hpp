#pragma once

#include <array>
#include <span>

#include <twist/stdlike/atomic.hpp>

#include <wheels/logging/logging.hpp>

namespace lockfree {

// Single-Producer / Multi-Consumer Bounded Ring Buffer

template <typename T, size_t Capacity>
class WorkStealingQueue {
  struct Slot {
    twist::stdlike::atomic<T*> item;
  };

 public:
  bool TryPush(T* item) {
    size_t cur_tail = tail_.load(std::memory_order_relaxed);
    if (cur_tail - head_.load(std::memory_order_relaxed) == Capacity) {
      return false;
    }
    cur_tail %= Capacity;
    buffer_[cur_tail].item.store(item, std::memory_order_relaxed);
    tail_.fetch_add(1, std::memory_order_release);
    return true;
  }

  // Returns nullptr if queue is empty
  T* TryPop() {
    size_t my_ind = head_.load(std::memory_order_relaxed);
    T* result;
    do {
      if (my_ind == tail_.load(std::memory_order_relaxed)) {
        return nullptr;
      }
      result = buffer_[my_ind % Capacity].item.load(std::memory_order_relaxed);
    } while (!head_.compare_exchange_weak(my_ind, my_ind + 1,
                                          std::memory_order_acquire,
                                          std::memory_order_relaxed));
    return result;
  }

  // Returns number of tasks
  size_t Grab(std::span<T*> store_buffer) {
    size_t my_head = head_.load(std::memory_order_relaxed);
    size_t cnt = 0;

    while (cnt < store_buffer.size() &&
           my_head + cnt < tail_.load(std::memory_order_acquire)) {
      store_buffer[cnt] = buffer_[(my_head + cnt) % Capacity].item.load(
          std::memory_order_relaxed);
      cnt++;
    }

    size_t cur_head = my_head;
    while (cur_head < my_head + cnt &&
           !head_.compare_exchange_weak(cur_head, my_head + cnt,
                                        std::memory_order_relaxed,
                                        std::memory_order_relaxed)) {
    }
    if (my_head + cnt <= cur_head) {
      return 0;
    }
    cnt -= (cur_head - my_head);
    for (size_t i = 0; i < cnt; ++i) {
      store_buffer[i] = store_buffer[i + (cur_head - my_head)];
    }
    return cnt;
  }

 private:
  twist::stdlike::atomic<size_t> head_{0};
  twist::stdlike::atomic<size_t> tail_{0};
  std::array<Slot, Capacity> buffer_;
};

}  // namespace lockfree
