#pragma once

#include <memory>
#include <cassert>

#include <variant>
#include <twist/stdlike/condition_variable.hpp>
#include <twist/stdlike/mutex.hpp>

namespace stdlike {
namespace detail {
template <typename T>
struct Shared_data;
}
template <typename T>
class Future {
  template <typename U>
  friend class Promise;

  template <typename U>
  friend struct detail::Shared_data;

 public:
  explicit Future(std::shared_ptr<stdlike::detail::Shared_data<T>> data)
      : chanel_(data) {
  }

  // Non-copyable
  Future(const Future&) = delete;
  Future& operator=(const Future&) = delete;

  // Movable
  Future(Future&&) = default;
  Future& operator=(Future&&) = default;

  // One-shot
  // Wait for result (value or exception)
  T Get() {
    std::unique_lock<twist::stdlike::mutex> lock(chanel_->mutex_);
    while (chanel_->contain_value_ == 0) {
      chanel_->ready_to_read_.wait(lock);
    }
    if (chanel_->contain_value_ == 2) {
      std::rethrow_exception(std::move(std::get<1>(chanel_->value_)));
    }
    return std::move(std::get<0>(chanel_->value_).GetValue());
  }

 private:
  Future() {
  }

 private:
  std::shared_ptr<stdlike::detail::Shared_data<T>> chanel_;
};

namespace detail {

template <typename T>
struct LateInit {
  LateInit() = default;

  explicit LateInit(T value) : val_(std::make_unique<T>(std::move(value))) {
  }

  T GetValue() {
    return std::move(*val_);
  }

 private:
  std::unique_ptr<T> val_;
};

template <typename T>
struct Shared_data {
  std::variant<LateInit<T>, std::exception_ptr> value_;
  char contain_value_{0};
  twist::stdlike::condition_variable ready_to_read_;
  twist::stdlike::mutex mutex_;
};
}  // namespace detail

}  // namespace stdlike
