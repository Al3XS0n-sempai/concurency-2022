#pragma once

#include "tagged_semaphore.hpp"

#include <deque>

namespace solutions {

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BlockingQueue {
 public:
  explicit BlockingQueue(size_t count_of_slots)
      : free_slots_(count_of_slots), work_(1) {
  }

  // Inserts the specified element into this queue,
  // waiting if necessary for space to become available.
  void Put(T value) {
    free_slots_.Acquire();
    work_.Acquire();
    queue_.push_back(std::move(value));
    items_in_queue_.Release();
    work_.Release();
  }

  // Retrieves and removes the head of this queue,
  // waiting if necessary until an element becomes available
  T Take() {
    items_in_queue_.Acquire();
    work_.Acquire();
    T front_element = std::move(queue_.front());
    queue_.pop_front();
    work_.Release();
    free_slots_.Release();
    return front_element;
  }

 private:
  std::deque<T> queue_;
  Semaphore free_slots_, items_in_queue_{0};
  Semaphore work_;
};

}  // namespace solutions
