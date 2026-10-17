#include "gtest/gtest.h"
#include "roo_windows/keyboard/typography.h"

namespace roo_windows {
// Verifies both local font selectors retain their exact assets and metrics at
// every supported zoom; each isolated target selects one zoom.
TEST(KeyboardLayoutTest, ZoomFontsMatchExistingMetrics) {
#if ROO_WINDOWS_ZOOM >= 200
  const auto& characters = roo_display::font_NotoSans_Regular_32();
  const auto& actions = roo_display::font_NotoSans_Medium_28();
#elif ROO_WINDOWS_ZOOM >= 150
  const auto& characters = roo_display::font_NotoSans_Regular_24();
  const auto& actions = roo_display::font_NotoSans_Medium_21();
#elif ROO_WINDOWS_ZOOM >= 100
  const auto& characters = roo_display::font_NotoSans_Regular_16();
  const auto& actions = roo_display::font_NotoSans_Medium_14();
#else
  const auto& characters = roo_display::font_NotoSans_Regular_12();
  const auto& actions = roo_display::font_NotoSans_Medium_11();
#endif
  EXPECT_EQ(&characters, &internal::KeyboardCharacterFont());
  EXPECT_EQ(&actions, &internal::KeyboardActionFont());
}

}  // namespace roo_windows
