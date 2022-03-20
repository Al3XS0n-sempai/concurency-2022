#include <exe/coroutine/impl.hpp>

#include <wheels/support/assert.hpp>
#include <wheels/support/compiler.hpp>

namespace exe::coroutine {

CoroutineImpl::CoroutineImpl(Routine routine, wheels::MutableMemView stack) {
  routine_ = std::move(routine);
  callee_ctx_.Setup(stack, this);
}

void CoroutineImpl::Run() {
  try {
    routine_();
  } catch (...) {
    exc_ptr_ = std::current_exception();
  }
  completed_ = true;
  callee_ctx_.ExitTo(caller_ctx_);
}

void CoroutineImpl::Resume() {
  caller_ctx_.SwitchTo(callee_ctx_);
  if (exc_ptr_ != nullptr) {
    std::rethrow_exception(exc_ptr_);
  }
}

void CoroutineImpl::Suspend() {
  callee_ctx_.SwitchTo(caller_ctx_);
}

bool CoroutineImpl::IsCompleted() const {
  return completed_;
}

}  // namespace exe::coroutine
