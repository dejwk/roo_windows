#pragma once

#include <vector>

#include "roo_windows/core/widget.h"

namespace roo_windows {

// class Context {
//  public:
//   virtual Dimensions getScreenDimensions() const = 0;
// };

class Activity {
 public:
  virtual Widget& getContents() = 0;

  virtual roo_display::Box getPreferredPlacement(const Task& task);

  // Exits this activity. The activity must have been at the top of the task
  // stack, or else the behavior is undefined.
  void exit();

  // Called when the activity gets entered.
  virtual void onStart() {}

  // Called when the activity resumes focus (after start, or after a child
  // activity has exited).
  virtual void onResume() {}

  // Called when the activity loses focus (just before it stops, or when a child
  // activity has started).
  virtual void onPause() {}

  // Called when the activity gets exited.
  virtual void onStop() {}

  // virtual roo_display::Box getPlacement();

  //  protected:
  //   Activity(const Context* context) : context_(context) {}

 private:
  // const Context* context_;
};

}  // namespace roo_windows
