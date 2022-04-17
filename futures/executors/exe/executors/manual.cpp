#include <exe/executors/manual.hpp>

namespace exe::executors {

void ManualExecutor::Execute(Task task) {
  task_queue_.push_back(std::move(task));
}

// Run tasks

size_t ManualExecutor::RunAtMost(size_t limit) {
  size_t executed = 0;
  while (executed < limit && HasTasks()) {
    ExecuteHere(std::move(task_queue_.front()));

    task_queue_.pop_front();
    executed++;
  }
  return executed;
}

size_t ManualExecutor::Drain() {
  size_t executed = 0;
  while (HasTasks()) {
    ExecuteHere(std::move(task_queue_.front()));

    task_queue_.pop_front();
    executed++;
  }
  return executed;
}

void ManualExecutor::ExecuteHere(Task task) noexcept {
  try {
    task();
  } catch (...) {
  }
}

}  // namespace exe::executors
