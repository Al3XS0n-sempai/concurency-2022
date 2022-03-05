#pragma once

#include <futures/future.hpp>

#include <memory>

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

namespace stdlike {

template <typename T>
class Promise {
 public:
  Promise() : chanel_(std::make_shared<detail::Shared_data<T>>()) {
  }

  // Non-copyable
  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;

  // Movable
  Promise(Promise&&) = default;
  Promise& operator=(Promise&&) = default;

  // One-shot
  Future<T> MakeFuture() {
    return Future<T>(chanel_);
  }

  // One-shot
  // Fulfill promise with value
  void SetValue(T value) {
    std::lock_guard<twist::stdlike::mutex> lock(chanel_->mutex_);
    chanel_->contain_value_ = 1;
    chanel_->value_.template emplace<0>(std::move(value));
    chanel_->ready_to_read_.notify_one();
  }

  // One-shot
  // Fulfill promise with exception
  void SetException(std::exception_ptr exc_ptr) {
    std::lock_guard<twist::stdlike::mutex> lock(chanel_->mutex_);
    chanel_->contain_value_ = 2;
    chanel_->value_.template emplace<1>(std::move(exc_ptr));
    chanel_->ready_to_read_.notify_one();
  }

 private:
  std::shared_ptr<stdlike::detail::Shared_data<T>> chanel_;
};

}  // namespace stdlike
