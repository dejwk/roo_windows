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

  virtual void onStart() {}
  virtual void onStop() {}

  // virtual roo_display::Box getPlacement();

//  protected:
//   Activity(const Context* context) : context_(context) {}

 private:
  // const Context* context_;
};

}  // namespace
