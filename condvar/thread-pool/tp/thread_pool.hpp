#pragma once

#include <tp/blocking_queue.hpp>
#include <tp/task.hpp>

#include <twist/stdlike/thread.hpp>

#include <vector>
#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

namespace tp {

class TaskCounter {
 public:
  TaskCounter() = default;

  void Inc() {
    std::lock_guard<twist::stdlike::mutex> lock(mutex_);
    counter_++;
  }

  void Dec() {
    std::lock_guard<twist::stdlike::mutex> lock(mutex_);
    counter_--;
    if (counter_ == 0) {
      counter_is_not_null_.notify_one();
    }
  }

  void WaitCounterOnNull() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    while (counter_ != 0) {
      counter_is_not_null_.wait(lock);
    }
  }

 private:
  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable counter_is_not_null_;
  int counter_{0};
};

// Fixed-size pool of worker threads

class ThreadPool {
 public:
  explicit ThreadPool(size_t workers);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // Schedules task for execution in one of the worker threads
  void Submit(Task task);

  // Waits until outstanding work count has reached zero
  void WaitIdle();

  // Stops the worker threads as soon as possible
  // Pending tasks will be discarded
  void Stop();

  // Locates current thread pool from worker thread
  static ThreadPool* Current();

 private:
  UnboundedBlockingQueue<Task> queue_;
  std::vector<twist::stdlike::thread> workers_;
  TaskCounter task_counter_;
};

inline ThreadPool* Current() {
  return ThreadPool::Current();
}

}  // namespace tp
