#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <exception>
#include <variant>

namespace stdlike::detail {
template <typename T>
class SharedData {
 public:
  void SetValue(T value) {
    std::lock_guard lock(mutex_);
    value_.template emplace<1>(std::move(value));
    ready_to_read_.notify_one();
  }

  void SetException(std::exception_ptr exc) {
    std::lock_guard lock(mutex_);
    value_.template emplace<0>(std::move(exc));
    ready_to_read_.notify_one();
  }

  T GetValue() {
    std::unique_lock lock(mutex_);
    while (value_.index() == 0 && std::get<0>(value_) == nullptr) {
      ready_to_read_.wait(lock);
    }
    if (value_.index() == 1) {
      return std::move(std::get<1>(value_));
    }
    std::rethrow_exception(std::get<0>(value_));
  }

 private:
  std::variant<std::exception_ptr, T> value_;
  twist::stdlike::condition_variable ready_to_read_;
  twist::stdlike::mutex mutex_;
};

}  // namespace stdlike::detail
