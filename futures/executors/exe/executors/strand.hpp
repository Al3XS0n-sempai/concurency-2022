#pragma once

#include <exe/executors/executor.hpp>

#include <twist/stdlike/atomic.hpp>

#include <exe/support/safe_queue.hpp>
#include <optional>

namespace exe::executors {

// Strand (serial executor, asynchronous mutex)
// Executes (via underlying executor) tasks
// non-concurrently and in FIFO order

class Strand : public IExecutor {
 public:
  explicit Strand(IExecutor& underlying);

  // IExecutor
  void Execute(Task task) override;

 private:
  void StrandWork();

  void ExecuteHere(Task task);

  void Schedule();

 private:
  IExecutor& executor_;
  SafeQueue<Task> task_queue_;
  twist::stdlike::atomic<int32_t> task_counter_;
};

}  // namespace exe::executors
