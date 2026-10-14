#include "roo_windows/core/default_theme.h"

#include "roo_windows/core/theme.h"

namespace roo_windows {

const Theme& DefaultTheme() {
  static const Theme theme;
  return theme;
}

}  // namespace roo_windows
