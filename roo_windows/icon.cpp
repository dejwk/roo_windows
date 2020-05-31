#include "icon.h"

namespace roo_windows {

using namespace roo_display;

const MaterialIconDef& ic_empty() {
  static MaterialIconDef value(
      RleImage4bppxPolarized<Alpha4, PrgMemResource>(0, 0, nullptr, Alpha4(color::Black)));
  return value;
}

}  // namespace roo_windows
