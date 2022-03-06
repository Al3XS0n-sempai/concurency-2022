#include <tp/thread_pool.hpp>

#include <twist/util/thread_local.hpp>

namespace tp {

////////////////////////////////////////////////////////////////////////////////

static twist::util::ThreadLocalPtr<ThreadPool> pool;

////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(size_t workers_count) {
  for (size_t i = 0; i < workers_count; ++i) {
    workers_.push_back(twist::stdlike::thread([this] {
      pool = this;
      while (true) {
        std::optional<Task> task = queue_.Take();
        if (task == std::nullopt) {
          return;
        }
        try {
          (task.value())();
        } catch (...) {
        };
        task_counter_.Dec();
      }
    }));
  }
}

ThreadPool::~ThreadPool() {
  assert(workers_.empty());
}

void ThreadPool::Submit(Task task) {
  task_counter_.Inc();
  queue_.Put(std::move(task));
}

void ThreadPool::WaitIdle() {
  task_counter_.WaitCounterOnNull();
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

}  // namespace tp
