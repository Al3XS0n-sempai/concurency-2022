#include <exe/tp/thread_pool.hpp>

#include <twist/util/thread_local.hpp>

namespace exe::tp {

////////////////////////////////////////////////////////////////////////////////

static twist::util::ThreadLocalPtr<ThreadPool> pool;

////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(size_t count_of_workers) {
  for (size_t cur_worker = 0; cur_worker < count_of_workers; ++cur_worker) {
    workers_.push_back(twist::stdlike::thread([this] {
      pool = this;
      while (true) {
        std::optional<Task> task = task_queue_.Take();
        if (task == std::nullopt) {
          return;
        }
        try {
          (task.value())();
        } catch (...) {
        };
        enqueued_tasks_counter_.Dec();
      }
    }));
  }
}

ThreadPool::~ThreadPool() {
  assert(workers_.empty());
}

void ThreadPool::Submit(Task task) {
  enqueued_tasks_counter_.Inc();

  task_queue_.Put(std::move(task));
}

void ThreadPool::WaitIdle() {
  enqueued_tasks_counter_.WaitForNull();
}

void ThreadPool::Stop() {
  task_queue_.Cancel();
  for (size_t cur_worker = 0; cur_worker < workers_.size(); ++cur_worker) {
    workers_[cur_worker].join();
  }
  workers_.clear();
}

ThreadPool* ThreadPool::Current() {
  return pool;
}

}  // namespace exe::tp
