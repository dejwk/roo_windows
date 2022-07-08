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

  virtual void onStart() {}
  virtual void onStop() {}

  // virtual roo_display::Box getPlacement();

  //  protected:
  //   Activity(const Context* context) : context_(context) {}

 private:
  // const Context* context_;
};

}  // namespace roo_windows
