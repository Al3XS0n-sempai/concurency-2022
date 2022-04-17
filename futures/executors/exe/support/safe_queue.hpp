#pragma once

#include <twist/stdlike/mutex.hpp>
#include <deque>
#include <optional>

namespace exe::executors {

template <class T>
class SafeQueue {
 public:
  SafeQueue() = default;

  SafeQueue(SafeQueue&& another) {
    std::lock_guard lock(mutex_);
    while (std::optional<T> item = another.Pop()) {
      queue_.push_back(std::move(item.value()));
    }
  }

  void Push(T obj) {
    std::lock_guard lock(mutex_);
    queue_.push_back(std::move(obj));
  }

  std::optional<T> Pop() {
    std::lock_guard lock(mutex_);
    if (Empty()) {
      return std::nullopt;
    }
    T result = std::move(queue_.front());
    queue_.pop_front();
    return result;
  }

  bool Empty() {
    return queue_.empty();
  }

 private:
  twist::stdlike::mutex mutex_;
  std::deque<T> queue_;
};
}  // namespace exe::executors