#pragma once

#include <wheels/support/function.hpp>

namespace exe::executors {

// Intrusive task?
using Task = wheels::UniqueFunction<void()>;

struct ITask {
  virtual ~ITask() = default;
  virtual void Run() = 0;
  virtual void Discard() = 0;
};

//
//[this, threadp]() {
//  djfsl;
//}

// template <typename F>
// struct LambdaHandler : public ITask {
//  LambdaHandler(F lamda) : lambda(std::move(lamda)) {}
//  void Run() {
//    lambda();
//    delete this;
//  }
//
//  void Discard() {
//    delete this;
//  }
//
//  F lambda;
//};

}  // namespace exe::executors
