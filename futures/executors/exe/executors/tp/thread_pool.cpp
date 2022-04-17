#include <exe/executors/tp/thread_pool.hpp>

#include <twist/util/thread_local.hpp>

namespace exe::executors {

////////////////////////////////////////////////////////////////////////////////

static twist::util::ThreadLocalPtr<ThreadPool> pool;

////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(size_t workers_count) {
  for (size_t i = 0; i < workers_count; ++i) {
    workers_.push_back(twist::stdlike::thread([this] {
      pool = this;

      WorkerRoutine();
    }));
  }
}

void ThreadPool::WorkerRoutine() noexcept {
  while (std::optional<Task> task = queue_.Take()) {
    ExecuteHere(std::move(task.value()));

    task_counter_.Decrement();
  }
}

void ThreadPool::ExecuteHere(Task task) noexcept {
  try {
    task();
  } catch (...) {
  };
}

ThreadPool::~ThreadPool() {
  assert(workers_.empty());
}

void ThreadPool::Execute(Task task) {
  task_counter_.Increment();
  queue_.Put(std::move(task));
}

void ThreadPool::WaitIdle() {
  task_counter_.WaitForZero();
}

void ThreadPool::Stop() {
  queue_.Cancel();
  for (size_t i = 0; i < workers_.size(); ++i) {
    workers_[i].join();
  }
  workers_.clear();
}

ThreadPool* ThreadPool::Current() {
  return pool;
}

}  // namespace exe::executors
