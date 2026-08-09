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
  bool mayMutate() const;
  bool attached(Destination& destination) const;
  void beginCallback();
  void endCallback();
  /// Pauses the current destination, then removes its attached widget.
  /// Returns false when a lifecycle callback completed a nested transition.
  bool pauseAndDetachCurrent();
  /// Removes the current, already-detached destination from history and stops
  /// it. Returns false when its stop callback supersedes this transition.
  bool stopCurrent();
  /// Starts a just-appended destination, attaches its widget, and resumes it.
  /// Returns false when either lifecycle callback supersedes this transition.
  bool startAndResume(Destination& destination);

  /// Borrowed task that installs this host; null while disconnected.
  UiTask* task_ = nullptr;
  /// Borrowed destinations in history order, with the current entry at back.
  std::vector<Destination*> history_;
  /// Changes after a successful command so an outer callback can detect that
  /// a nested command has taken over its transition.
  unsigned int generation_ = 0;
  /// Number of active navigation commands. Incidental structural callbacks
  /// must not start a command while this is nonzero.
  unsigned int mutation_depth_ = 0;
  /// Number of active destination lifecycle or Back callbacks. These are the
  /// only callbacks allowed to synchronously issue a nested command.
  unsigned int lifecycle_callback_depth_ = 0;
};

}  // namespace roo_windows
