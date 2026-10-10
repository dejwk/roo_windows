#include "gtest/gtest.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/decoration/shadow_lookup.h"
#include "test/shadow_test_reference.h"

namespace roo_windows {
namespace {
// Verifies every cached radius, a non-cached radius, asymmetric corners and
// all levels against the original formula, at this target's configured zoom.
TEST(ShadowGradientTest, EveryPresetMatchesReferenceAndDamage) {
  for (int dp : {0, 4, 8, 12, 16, 20, 24, 18, 28, -1}) {
    uint8_t radius = Scaled(std::max(dp, 0));
    BorderStyle::CornerRadii radii{radius, radius, radius, radius};
    if (dp < 0)
      radii = {uint8_t(Scaled(4)), uint8_t(Scaled(8)), uint8_t(Scaled(18)),
               uint8_t(Scaled(20))};
    for (int level = 0; level <= 5; ++level) {
      roo_display::Box box(-11, 17, Scaled(96) - 12, Scaled(72) + 16);
      internal::LookupShadowSpec key =
          internal::MakeLookupShadow(box, radii, level, false);
      internal::LookupShadowSpec ambient =
          internal::MakeLookupShadow(box, radii, level, true);
      roo_display::Box bounds =
          internal::LookupShadowExtents(Rect(box), level).asBox();
      for (int y = box.yMin() - Scaled(34); y <= box.yMax() + Scaled(34); ++y) {
        for (int x = box.xMin() - Scaled(34); x <= box.xMax() + Scaled(34);
             ++x) {
          int k = test::ReferenceLayer(key, false, x, y);
          int a = test::ReferenceLayer(ambient, true, x, y);
          int expected = k + a - (k * a + 127) / 255;
          ASSERT_EQ(expected, internal::ReadLookupShadow(key, ambient, x, y))
              << "zoom " << ROO_WINDOWS_ZOOM << " radius " << dp << " level "
              << level << " pixel " << x << ',' << y;
          if (x < bounds.xMin() || x > bounds.xMax() || y < bounds.yMin() ||
              y > bounds.yMax()) {
            ASSERT_EQ(0, expected);
          }
        }
      }
    }
  }
}
}  // namespace
}  // namespace roo_windows
