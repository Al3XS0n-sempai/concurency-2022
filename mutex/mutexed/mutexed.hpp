#pragma once

#include <twist/stdlike/mutex.hpp>

namespace util {

//////////////////////////////////////////////////////////////////////

// Safe API for mutual exclusion

template <typename T>
class Mutexed {
  using MutexImpl = twist::stdlike::mutex;

  class UniqueRef {
    class SafeAccess {
     public:
      explicit SafeAccess(Mutexed<T>& base)
          : lock_(base.mutex_), obj_(base.object_) {
      }

      SafeAccess() = delete;

      T& operator*() {
        return obj_;
      }

      T* operator->() {
        return &obj_;
      }

     private:
      std::lock_guard<MutexImpl> lock_;
      T& obj_;
    };

   public:
    explicit UniqueRef(Mutexed<T>& base) : base_(base) {
    }

    // Non-copyable
    UniqueRef(const UniqueRef&) = delete;

    // Non-movable
    UniqueRef(UniqueRef&&) = delete;

    SafeAccess operator*() {
      return SafeAccess(base_);
    }

    SafeAccess operator->() {
      return SafeAccess(base_);
    }

   private:
    Mutexed<T>& base_;
  };

 public:
  // https://eli.thegreenplace.net/2014/perfect-forwarding-and-universal-references-in-c/
  template <typename... Args>
  explicit Mutexed(Args&&... args) : object_(std::forward<Args>(args)...) {
  }

  UniqueRef Lock() {
    return UniqueRef(*this);
  }

 private:
  T object_;
  MutexImpl mutex_;  // Guards access to object_
};

//////////////////////////////////////////////////////////////////////

// Helper function for single operations over shared object:
// Usage:
//   Mutexed<vector<int>> ints;
//   Locked(ints)->push_back(42);

template <typename T>
auto Locked(Mutexed<T>& object) {
  return object.Lock();
}

}  // namespace util
