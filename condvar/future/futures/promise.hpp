#pragma once

#include <futures/future.hpp>
#include <futures/detail.hpp>

#include <memory>

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

namespace stdlike {

template <typename T>
class Promise {
 public:
  Promise() {
  }

  // Non-copyable
  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;

  // Movable
  Promise(Promise&&) = default;
  Promise& operator=(Promise&&) = default;

  // One-shot
  Future<T> MakeFuture() {
    assert(channel_ == nullptr);
    channel_ = std::make_shared<detail::SharedData<T>>();
    return Future<T>(channel_);
  }

  // One-shot
  // Fulfill promise with value
  void SetValue(T value) {
    assert(!std::exchange(set_called_, true));
    channel_->SetValue(std::move(value));
  }

  // One-shot
  // Fulfill promise with exception
  void SetException(std::exception_ptr exc_ptr) {
    assert(!std::exchange(set_called_, true));
    channel_->SetException(std::move(exc_ptr));
  }

 private:
  std::shared_ptr<detail::SharedData<T>> channel_{nullptr};
  bool set_called_{false};
};

}  // namespace stdlike
