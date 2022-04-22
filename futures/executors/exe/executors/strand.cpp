#include <exe/executors/strand.hpp>

#include <wheels/logging/logging.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& executor) : executor_(executor) {
}

void Strand::Execute(Task task) {
  tasks_stack_.Push(std::move(task));

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
  std::deque<Task> tasks(tasks_stack_.PopAll());

  int32_t executed = 0;
  while (!tasks.empty()) {
    ExecuteHere(std::move(tasks.back()));
    executed++;
    tasks.pop_back();
  }

  if (task_counter_.fetch_sub(executed, std::memory_order_acq_rel) > executed) {
    Schedule();
  }
}

}  // namespace exe::executors
