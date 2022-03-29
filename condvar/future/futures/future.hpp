#pragma once

#include <futures/detail.hpp>

#include <memory>
#include <cassert>

#include <variant>
#include <twist/stdlike/condition_variable.hpp>
#include <twist/stdlike/mutex.hpp>

namespace stdlike {

template <typename T>
class Future {
  template <typename U>
  friend class Promise;

 public:
  explicit Future(std::shared_ptr<detail::SharedData<T>> data) : chanel_(data) {
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
    return chanel_->GetValue();
  }

 private:
  Future() {
  }

 private:
  std::shared_ptr<detail::SharedData<T>> chanel_;
};

}  // namespace stdlike
