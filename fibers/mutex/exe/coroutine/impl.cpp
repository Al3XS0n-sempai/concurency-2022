#include <exe/coroutine/impl.hpp>

#include <wheels/support/assert.hpp>
#include <wheels/support/compiler.hpp>

namespace exe::coroutine {

CoroutineImpl::CoroutineImpl(Routine routine, wheels::MutableMemView stack)
    : routine_(std::move(routine)) {
  routine_ctx_.Setup(stack, this);
}

void CoroutineImpl::Run() {
  try {
    routine_();
  } catch (...) {
    exc_ = std::current_exception();
  }
  completed_ = true;
  routine_ctx_.ExitTo(caller_ctx_);
}

void CoroutineImpl::Resume() {
  caller_ctx_.SwitchTo(routine_ctx_);
  if (exc_ != nullptr) {
    std::rethrow_exception(exc_);
  }
}

void CoroutineImpl::Suspend() {
  routine_ctx_.SwitchTo(caller_ctx_);
}

bool CoroutineImpl::IsCompleted() const {
  return completed_;
}

}  // namespace exe::coroutine
