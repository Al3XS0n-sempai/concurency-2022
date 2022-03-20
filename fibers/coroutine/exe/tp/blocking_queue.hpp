#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <optional>
#include <deque>
#include <iostream>

namespace exe::tp {
template <typename T>
class UnboundedBlockingQueue {
 public:
  bool Put(T value) {
    std::lock_guard<twist::stdlike::mutex> lock(mutex_);
    if (closed_) {
      return false;
    }
    queue_.push_back(std::move(value));
    item_available_.notify_one();
    return true;
  }

  std::optional<T> Take() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    while (queue_.empty()) {
      if (closed_) {
        return std::nullopt;
      }
      item_available_.wait(lock);
    }
    T front_item = std::move(queue_.front());
    queue_.pop_front();
    return std::move(front_item);
  }

  void Close() {
    CloseImpl(false);
  }

  void Cancel() {
    CloseImpl(true);
  }

 private:
  void CloseImpl(bool need_to_clear) {
    std::lock_guard<twist::stdlike::mutex> lock(mutex_);
    closed_ = true;
    if (need_to_clear) {
      queue_.clear();
    }
    item_available_.notify_all();
  }

 private:
  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable item_available_;
  std::deque<T> queue_;
  bool closed_{false};
};

}  // namespace exe::tp