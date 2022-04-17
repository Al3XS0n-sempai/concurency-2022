#include <exe/executors/strand.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& executor) : executor_(executor) {
}

void Strand::Execute(Task task) {
  task_queue_.Push(std::move(task));

  if (task_counter_.fetch_add(1, std::memory_order_acq_rel) == 0) {
    Schedule();
  }
}

void Strand::ExecuteHere(Task task) {
  try {
    task();
  } catch (...) {
  }
}

void Strand::Schedule() {
  executor_.Execute([&] {
    StrandWork();
  });
}

void Strand::StrandWork() {
  SafeQueue<Task> tmp(std::move(task_queue_));

  int32_t executed = 0;
  while (std::optional<Task> task = tmp.Pop()) {
    ExecuteHere(std::move(task.value()));

    executed++;
  }

  if (task_counter_.fetch_sub(executed, std::memory_order_acq_rel) > executed) {
    Schedule();
  }
}

}  // namespace exe::executors
