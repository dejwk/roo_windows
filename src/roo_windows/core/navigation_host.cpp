#include "roo_windows/core/navigation_host.h"

#include "roo_logging.h"
#include "roo_windows/core/ui_task.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

Destination::~Destination() { CHECK(host_ == nullptr); }

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

  history_.push_back(&destination);
  mutating_ = true;
  destination.host_ = this;
  if (history_.size() > 1) task_->detachNavigationContent();
  task_->attachNavigationContent(destination.getContents());
  ++generation_;
  mutating_ = false;
}

void NavigationHost::replace(Destination& destination) {
  CHECK(task_ != nullptr);
  CHECK(!mutating_);
  CHECK(!history_.empty());
  CHECK(destination.host_ == nullptr);
  CHECK(destination.getContents().parent() == nullptr);

  mutating_ = true;
  Destination* previous = history_.back();
  task_->detachNavigationContent();
  previous->host_ = nullptr;
  history_.back() = &destination;
  destination.host_ = this;
  task_->attachNavigationContent(destination.getContents());
  ++generation_;
  mutating_ = false;
}

void NavigationHost::pop() {
  CHECK(task_ != nullptr);
  CHECK(!mutating_);
  CHECK(!history_.empty());

  mutating_ = true;
  Destination* previous = history_.back();
  task_->detachNavigationContent();
  history_.pop_back();
  previous->host_ = nullptr;
  if (!history_.empty()) {
    task_->attachNavigationContent(history_.back()->getContents());
  }
  ++generation_;
  mutating_ = false;
}

void NavigationHost::clear() {
  CHECK(!mutating_);
  if (history_.empty()) return;

  mutating_ = true;
  if (task_ != nullptr) task_->detachNavigationContent();
  for (Destination* destination : history_) destination->host_ = nullptr;
  history_.clear();
  ++generation_;
  mutating_ = false;
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
