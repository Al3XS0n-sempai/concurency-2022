#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

// std::lock_guard, std::unique_lock
#include <mutex>
#include <cstdint>

namespace solutions {

// A Counting semaphore

// Semaphores are often used to restrict the number of threads
// than can access some (physical or logical) resource

class Semaphore {
 public:
  // Creates a Semaphore with the given number of permits
  explicit Semaphore(size_t initial_ticket_count)
      : ticket_count_(initial_ticket_count) {
  }

  // Acquires a permit from this semaphore,
  // blocking until one is available
  void Acquire() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    while (ticket_count_ == 0) {
      free_ticket_.wait(lock);
    }
    ticket_count_--;
  }

  // Releases a permit, returning it to the semaphore
  void Release() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    ticket_count_++;
    free_ticket_.notify_one();
  }

 private:
  size_t ticket_count_;
  twist::stdlike::condition_variable free_ticket_;
  twist::stdlike::mutex mutex_;
};

}  // namespace solutions
