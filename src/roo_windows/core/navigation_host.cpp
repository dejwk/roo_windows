#include "roo_windows/core/navigation_host.h"

#include "roo_logging.h"
#include "roo_windows/core/ui_task.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

NavigationHost::~NavigationHost() {
  CHECK(task_ == nullptr);
  CHECK(history_.empty());
}

void NavigationHost::install(UiTask& task) {
  CHECK(task_ == nullptr);
  CHECK(history_.empty());
  task_ = &task;
}

void NavigationHost::disconnect() {
  clear();
  task_ = nullptr;
}

void NavigationHost::push(Destination& destination) {
  CHECK(task_ != nullptr);
  CHECK(!mutating_);
  CHECK(destination.host_ == nullptr);
  CHECK(destination.getContents().parent() == nullptr);

  mutating_ = true;
  Destination* previous = current();
  if (previous != nullptr) {
    CHECK(previous->state_ == Destination::kActive);
    previous->state_ = Destination::kPausing;
    previous->onPause();
    if (previous->state_ != Destination::kPausing) {
      mutating_ = false;
      return;
    }
    task_->detachNavigationContent();
    previous->state_ = Destination::kPaused;
  }
  history_.push_back(&destination);
  destination.host_ = this;
  destination.state_ = Destination::kStarting;
  destination.onStart();
  if (destination.state_ != Destination::kStarting ||
      current() != &destination) {
    mutating_ = false;
    return;
  }
  task_->attachNavigationContent(destination.getContents());
  destination.state_ = Destination::kResuming;
  destination.onResume();
  if (destination.state_ == Destination::kResuming &&
      current() == &destination) {
    destination.state_ = Destination::kActive;
  }
  ++generation_;
  mutating_ = false;
}

void NavigationHost::replace(Destination& destination) {
  CHECK(task_ != nullptr);
  CHECK(!mutating_);
  CHECK(!history_.empty());
  CHECK(destination.host_ == nullptr);
  CHECK(destination.getContents().parent() == nullptr);

  pop();
  push(destination);
}

void NavigationHost::pop() {
  CHECK(task_ != nullptr);
  CHECK(!mutating_);
  CHECK(!history_.empty());

  mutating_ = true;
  Destination* previous = history_.back();
  if (previous->state_ == Destination::kActive) {
    previous->state_ = Destination::kPausing;
    previous->onPause();
    if (previous->state_ != Destination::kPausing) {
      mutating_ = false;
      return;
    }
    task_->detachNavigationContent();
    previous->state_ = Destination::kPaused;
  }
  CHECK(previous->state_ == Destination::kPaused);
  history_.pop_back();
  previous->state_ = Destination::kStopping;
  previous->onStop();
  if (previous->state_ == Destination::kStopping) {
    previous->state_ = Destination::kInactive;
    previous->host_ = nullptr;
  }
  if (!history_.empty()) {
    Destination* next = history_.back();
    task_->attachNavigationContent(next->getContents());
    next->state_ = Destination::kResuming;
    next->onResume();
    if (next->state_ == Destination::kResuming)
      next->state_ = Destination::kActive;
  }
  ++generation_;
  mutating_ = false;
}

void NavigationHost::clear() {
  CHECK(!mutating_);
  if (history_.empty()) return;

  while (!history_.empty()) pop();
}

BackResult NavigationHost::requestBack(BackSource source) {
  if (history_.empty()) return task_->requestTaskBackCallback(source);

  Destination* destination = history_.back();
  unsigned int generation = generation_;
  if (destination->onBackRequested(source) == BackResult::kHandled) {
    return BackResult::kHandled;
  }
  if (task_ == nullptr || history_.empty() || history_.back() != destination ||
      generation_ != generation) {
    return BackResult::kHandled;
  }
  if (history_.size() > 1) {
    pop();
    return BackResult::kHandled;
  }
  return task_->requestTaskBackCallback(source);
}

}  // namespace roo_windows
