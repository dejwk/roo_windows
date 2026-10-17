#pragma once

#include "roo_fonts/NotoSans_Medium/11.h"
#include "roo_fonts/NotoSans_Medium/14.h"
#include "roo_fonts/NotoSans_Medium/21.h"
#include "roo_fonts/NotoSans_Medium/28.h"
#include "roo_fonts/NotoSans_Regular/12.h"
#include "roo_fonts/NotoSans_Regular/16.h"
#include "roo_fonts/NotoSans_Regular/24.h"
#include "roo_fonts/NotoSans_Regular/32.h"
#include "roo_windows/config.h"

namespace roo_windows::internal {

/// Returns the keyboard character font at the configured zoom.
inline const roo_display::Font& KeyboardCharacterFont() {
#if ROO_WINDOWS_ZOOM >= 200
  return roo_display::font_NotoSans_Regular_32();
#elif ROO_WINDOWS_ZOOM >= 150
  return roo_display::font_NotoSans_Regular_24();
#elif ROO_WINDOWS_ZOOM >= 100
  return roo_display::font_NotoSans_Regular_16();
#else
  return roo_display::font_NotoSans_Regular_12();
#endif
}

/// Returns the keyboard action-label font at the configured zoom.
inline const roo_display::Font& KeyboardActionFont() {
#if ROO_WINDOWS_ZOOM >= 200
  return roo_display::font_NotoSans_Medium_28();
#elif ROO_WINDOWS_ZOOM >= 150
  return roo_display::font_NotoSans_Medium_21();
#elif ROO_WINDOWS_ZOOM >= 100
  return roo_display::font_NotoSans_Medium_14();
#else
  return roo_display::font_NotoSans_Medium_11();
#endif
}

}  // namespace roo_windows::internal
