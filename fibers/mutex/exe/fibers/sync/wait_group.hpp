#pragma once

#include <exe/fibers/sync/futex.hpp>

#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

// https://gobyexample.com/waitgroups

class WaitGroup {
 public:
  void Add(size_t /*count*/) {
  }

  void Done() {
  }

  void Wait() {
  }

 private:
  // ???
};

}  // namespace exe::fibers
