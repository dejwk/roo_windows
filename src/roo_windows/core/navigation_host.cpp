#include "roo_windows/core/navigation_host.h"

#include "roo_logging.h"
#include "roo_windows/core/ui_task.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

NavigationHost::~NavigationHost() {
  CHECK(task_ == nullptr);
  CHECK(history_.empty());
}

bool NavigationHost::mayMutate() const {
  // Widget attachment and detachment can synchronously notify arbitrary
  // application code. Only destination lifecycle and Back callbacks receive
  // the explicit reentrant mutation window.
  return mutation_depth_ == 0 || lifecycle_callback_depth_ != 0;
}

bool NavigationHost::attached(Destination& destination) const {
  return destination.getContents().parent() != nullptr;
}

void NavigationHost::beginCallback() { ++lifecycle_callback_depth_; }
void NavigationHost::endCallback() { --lifecycle_callback_depth_; }

void NavigationHost::install(UiTask& task) {
  CHECK(task_ == nullptr);
  CHECK(history_.empty());
  task_ = &task;
}

void NavigationHost::disconnect() {
  // Teardown uses the regular lifecycle path so every entry observes stop
  // while the task and host are still available to it.
  clear();
  task_ = nullptr;
}

bool NavigationHost::pauseAndDetachCurrent() {
  Destination* destination = current();
  if (destination == nullptr) return true;
  // A nested command from onPause() may remove or replace this destination.
  // Snapshot the generation and identity before invoking application code so
  // the outer command never detaches or pauses a stale entry a second time.
  unsigned int generation = generation_;
  if (destination->state_ == Destination::kActive) {
    destination->state_ = Destination::kPausing;
    beginCallback();
    destination->onPause();
    endCallback();
    if (generation != generation_ || current() != destination ||
        destination->state_ != Destination::kPausing) {
      return false;
    }
  }
  CHECK(destination->state_ == Destination::kPausing ||
        destination->state_ == Destination::kPaused ||
        destination->state_ == Destination::kResuming);
  // Detachment is deliberately outside the lifecycle callback window. A
  // navigation request from an incidental widget callback is a contract
  // violation, enforced by mayMutate().
  if (attached(*destination)) task_->detachNavigationContent();
  destination->state_ = Destination::kPaused;
  return true;
}

bool NavigationHost::stopCurrent() {
  Destination* destination = current();
  CHECK(destination != nullptr);
  CHECK(destination->state_ == Destination::kPaused ||
        destination->state_ == Destination::kStarting ||
        destination->state_ == Destination::kStopping);
  if (destination->state_ != Destination::kStopping) {
    // Remove the entry before onStop(): it is no longer current, but retains
    // host_ until the callback returns so it can still resolve its context.
    history_.pop_back();
    destination->state_ = Destination::kStopping;
    unsigned int generation = generation_;
    beginCallback();
    destination->onStop();
    endCallback();
    // A nested lifecycle command owns any state it established. Do not clear
    // host_ or overwrite the state after that command has superseded us.
    if (generation != generation_ ||
        destination->state_ != Destination::kStopping) {
      return false;
    }
    destination->state_ = Destination::kInactive;
    destination->host_ = nullptr;
  }
  return true;
}

bool NavigationHost::startAndResume(Destination& destination) {
  // The destination is already in history. onStart observes membership but
  // not attachment; onResume observes the newly attached content.
  destination.host_ = this;
  destination.state_ = Destination::kStarting;
  unsigned int generation = generation_;
  beginCallback();
  destination.onStart();
  endCallback();
  // Lifecycle callbacks may issue a nested command. Continue only while this
  // destination remains the same transitional current entry.
  if (generation != generation_ || current() != &destination ||
      destination.state_ != Destination::kStarting) {
    return false;
  }
  task_->attachNavigationContent(destination.getContents());
  destination.state_ = Destination::kResuming;
  generation = generation_;
  beginCallback();
  destination.onResume();
  endCallback();
  if (generation != generation_ || current() != &destination ||
      destination.state_ != Destination::kResuming) {
    return false;
  }
  destination.state_ = Destination::kActive;
  return true;
}

void NavigationHost::push(Destination& destination) {
  CHECK(task_ != nullptr);
  CHECK(mayMutate());
  CHECK(destination.host_ == nullptr);
  CHECK(destination.getContents().parent() == nullptr);
  // Do any capacity growth before lifecycle code can observe a transition;
  // a vector allocation cannot then leave a partially paused history.
  if (history_.size() == history_.capacity()) history_.reserve(history_.size() + 1);
  ++mutation_depth_;
  if (!pauseAndDetachCurrent()) {
    --mutation_depth_;
    return;
  }
  history_.push_back(&destination);
  if (startAndResume(destination)) ++generation_;
  --mutation_depth_;
}

void NavigationHost::replace(Destination& destination) {
  CHECK(task_ != nullptr);
  CHECK(mayMutate());
  CHECK(!history_.empty());
  CHECK(destination.host_ == nullptr);
  CHECK(destination.getContents().parent() == nullptr);
  if (history_.size() == history_.capacity()) history_.reserve(history_.size() + 1);
  ++mutation_depth_;
  if (!pauseAndDetachCurrent() || !stopCurrent()) {
    --mutation_depth_;
    return;
  }
  // Do not resume the covered destination below the replacement: replace is
  // one removal followed directly by one start/attach/resume transition.
  history_.push_back(&destination);
  if (startAndResume(destination)) ++generation_;
  --mutation_depth_;
}

void NavigationHost::pop() {
  CHECK(task_ != nullptr);
  CHECK(mayMutate());
  CHECK(!history_.empty());
  ++mutation_depth_;
  if (!pauseAndDetachCurrent() || !stopCurrent()) {
    --mutation_depth_;
    return;
  }
  if (!history_.empty()) {
    Destination* destination = current();
    CHECK(destination->state_ == Destination::kPaused);
    // Only pop restores the now-current covered entry. Its widget must be
    // attached before onResume(), mirroring the Activity lifecycle contract.
    task_->attachNavigationContent(destination->getContents());
    destination->state_ = Destination::kResuming;
    unsigned int generation = generation_;
    beginCallback();
    destination->onResume();
    endCallback();
    if (generation == generation_ && current() == destination &&
        destination->state_ == Destination::kResuming) {
      destination->state_ = Destination::kActive;
    }
  }
  ++generation_;
  --mutation_depth_;
}

void NavigationHost::clear() {
  CHECK(mayMutate());
  if (history_.empty()) return;
  ++mutation_depth_;
  if (!pauseAndDetachCurrent()) {
    --mutation_depth_;
    return;
  }
  // Stop top to bottom; never attach or resume an intermediate destination.
  // This preserves the distinction between leaving history and becoming the
  // visible destination.
  while (!history_.empty()) {
    if (!stopCurrent()) break;
  }
  ++generation_;
  --mutation_depth_;
}

BackResult NavigationHost::requestBack(BackSource source) {
  if (history_.empty()) return task_->requestTaskBackCallback(source);
  Destination* destination = current();
  // Back handlers get the same state-aware reentrant window as lifecycle
  // handlers. A nested command consumes this one semantic Back request.
  unsigned int generation = generation_;
  beginCallback();
  BackResult result = destination->onBackRequested(source);
  endCallback();
  if (result == BackResult::kHandled) return BackResult::kHandled;
  // Do not apply the normal pop fallback after a callback navigated, detached
  // the widget, or disconnected the task.
  if (task_ == nullptr || history_.empty() || current() != destination ||
      generation_ != generation || !attached(*destination)) {
    return BackResult::kHandled;
  }
  if (history_.size() > 1) {
    pop();
    return BackResult::kHandled;
  }
  return task_->requestTaskBackCallback(source);
}

}  // namespace roo_windows
