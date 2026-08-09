#pragma once

#include <cstddef>
#include <vector>

#include "roo_windows/core/destination.h"

namespace roo_windows {

class NavigationHost;
class UiTask;
class Widget;
class Application;

/// Optional, caller-owned navigation history for one `UiTask`.
///
/// The host borrows every `Destination` and its contents. It stores history in
/// a growable vector; only `push()` may allocate after its retained capacity
/// has been reached.
class NavigationHost {
 public:
  NavigationHost() = default;
  ~NavigationHost();

  NavigationHost(const NavigationHost&) = delete;
  NavigationHost& operator=(const NavigationHost&) = delete;

  /// Pushes a caller-owned destination, making it current.
  void push(Destination& destination);

  /// Replaces the current destination with a caller-owned destination.
  void replace(Destination& destination);

  /// Removes the current destination. Popping the root leaves the host empty.
  void pop();

  /// Removes every destination. Empty hosts remain unchanged.
  void clear();

  /// Returns whether the host has no destinations.
  bool empty() const { return history_.empty(); }

  /// Returns the number of stored destinations.
  size_t depth() const { return history_.size(); }

  /// Returns this host's task while installed, otherwise nullptr.
  UiTask* getUiTask() const { return task_; }

 private:
  friend class UiTask;
  friend class Application;
  friend class Destination;

  /// Routes Back through the current destination and normal history fallback.
  BackResult requestBack(BackSource source);
  Destination* current() const {
    return history_.empty() ? nullptr : history_.back();
  }
  void install(UiTask& task);
  void disconnect();
  UiTask* task_ = nullptr;
  std::vector<Destination*> history_;
  unsigned int generation_ = 0;
  bool mutating_ = false;
};

}  // namespace roo_windows
