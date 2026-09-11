#pragma once

#include <cstddef>
#include <vector>

#include "roo_windows/core/destination.h"

namespace roo_windows {

class Task;
class Widget;
class Application;
namespace test {
class NavigationHostTestAccess;
}

/// Navigation history owned by one `Task`.
///
/// The host borrows every `Destination` and its contents. It stores history in
/// an inline root slot and a growable vector for entries above it. Initial
/// root push and root replacement allocate no history storage; later pushes
/// may allocate once retained capacity has been reached.
class NavigationHost {
 public:
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
  bool empty() const { return root_ == nullptr; }

  /// Returns the number of stored destinations.
  size_t depth() const { return empty() ? 0 : 1 + history_.size(); }

  /// Returns whether new destinations can currently be admitted.
  bool isAvailable() const;

  /// Returns whether this destination is the current history entry.
  bool isCurrent(const Destination& destination) const {
    return current() == &destination;
  }

  /// Returns this host's task while installed, otherwise nullptr.
  Task* getTask() const { return task_; }

 private:
  NavigationHost() = default;

  friend class Task;
  friend class test::NavigationHostTestAccess;
  friend class Application;
  friend class Destination;

  /// Routes Back through the current destination and normal history fallback.
  BackResult requestBack(BackSource source);
  Destination* current() const {
    return history_.empty() ? root_ : history_.back();
  }
  void append(Destination& destination);
  void removeCurrent();
  void install(Task& task);
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
  Task* task_ = nullptr;
  /// Stores the first borrowed destination without a vector allocation.
  Destination* root_ = nullptr;
  /// Borrowed destinations above the root, with the current entry at back.
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
