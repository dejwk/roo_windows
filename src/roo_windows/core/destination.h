#pragma once

#include "roo_windows/core/back_request.h"

namespace roo_windows {

class Application;
class NavigationHost;
class Task;
class Widget;

/// An entry in a `NavigationHost` history.
///
/// A destination supplies a contents widget and observes start, resume, pause,
/// and stop transitions. It is borrowed by its navigation host and remains
/// paused in history while covered.
class Destination {
 public:
  /// Lifecycle states for membership in history and current visibility.
  enum State {
    kInactive,
    kStarting,
    kResuming,
    kActive,
    kPausing,
    kPaused,
    kStopping
  };

  /// Destroys this detached destination.
  ///
  /// Destinations are borrowed by their host and must be removed from history
  /// before destruction.
  virtual ~Destination();

  /// Returns the root widget that this destination presents.
  virtual Widget& getContents() = 0;

  /// Returns the navigation host this destination belongs to, or nullptr when
  /// detached from history.
  NavigationHost* getNavigationHost() { return host_; }

  /// Returns the task this destination belongs to, or nullptr when detached.
  Task* getTask();

  /// Returns the application this destination belongs to, or nullptr when
  /// detached.
  Application* getApplication();

  /// Removes this current destination from its navigation host.
  void exit();

  /// Gives this destination the first opportunity to consume a Back request.
  ///
  /// The default implementation leaves the request unhandled, allowing the
  /// host to pop this destination when it is not the root entry.
  virtual BackResult onBackRequested(BackSource source) {
    return BackResult::kUnhandled;
  }

  /// Called when this destination enters navigation history while detached.
  virtual void onStart() {}

  /// Called when this destination gains or resumes current visibility.
  ///
  /// The contents are attached before this callback.
  virtual void onResume() {}

  /// Called before this current destination is covered or removed.
  ///
  /// The contents remain attached throughout this callback.
  virtual void onPause() {}

  /// Called when this destination leaves navigation history while detached.
  virtual void onStop() {}

 protected:
  /// Constructs an inactive destination outside navigation history.
  Destination() = default;

  /// Returns the current lifecycle state.
  State state() const { return state_; }

 private:
  friend class NavigationHost;
  NavigationHost* host_ = nullptr;
  State state_ = kInactive;
};

}  // namespace roo_windows
