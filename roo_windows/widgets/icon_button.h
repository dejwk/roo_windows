#pragma once

#include "roo_windows/widgets/icon.h"

namespace roo_windows {

class IconButton : public Icon {
 public:
  using Icon::Icon;
  bool isClickable() const override { return true; }
};

}  // namespace roo_windows
