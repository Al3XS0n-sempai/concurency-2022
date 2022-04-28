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
    T* item;
  };

 public:
  bool TryPush(T* item) {
    size_t cur_tail = tail_.load();
    if (cur_tail - head_.load() == Capacity) {
      return false;
    }
    cur_tail %= Capacity;
    Slot* data = buffer_.data();
    data[cur_tail].item = item;
    tail_.fetch_add(1);
    return true;
  }

  // Returns nullptr if queue is empty
  T* TryPop() {
    size_t my_ind = head_.load(), next;
    do {
      if (my_ind == tail_.load()) {
        return nullptr;
      }
      next = my_ind + 1;

    } while (!head_.compare_exchange_weak(my_ind, next));

    Slot* data = buffer_.data();
    T* result = data[my_ind % Capacity].item;
    return result;
  }

  // Returns number of tasks
  size_t Grab(std::span<T*> store_buffer) {
    size_t my_head = head_.load();
    size_t cnt = 0;
    Slot* data = buffer_.data();
    while (cnt < store_buffer.size() && my_head + cnt < tail_.load()) {
      store_buffer[cnt] = data[(my_head + cnt) % Capacity].item;
      cnt++;
    }

    size_t cur_head = my_head;
    while (cur_head < my_head + cnt &&
           !head_.compare_exchange_weak(cur_head, my_head + cnt)) {
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
