#include "curves.h"

#include "gtest/gtest.h"

namespace shadow_curves {
namespace {
using roo_windows::internal::MakeLookupShadow;
using roo_windows::internal::ReadLookupShadow;

// Verifies timing, nearest-index rounding, and continuity at pattern changes.
TEST(ShadowCurves, Timing) {
  EXPECT_EQ(SampleLevel(0), 0);
  EXPECT_EQ(SampleLevel(2500), 16);
  EXPECT_EQ(SampleLevel(5000), 31);
  EXPECT_EQ(SampleLevel(10000), 0);
  EXPECT_EQ(SampleLevel(12500), 16);
  EXPECT_EQ(SampleLevel(15000), 31);
  EXPECT_EQ(SampleLevel(20000), 0);
  for (uint32_t t = 0; t < 20000; ++t) {
    EXPECT_GE(SampleLevel(t), 0);
    EXPECT_LE(SampleLevel(t), 31);
    EXPECT_EQ(SampleLevel(t), SampleLevel(t + 20000));
  }
}

// Verifies the example uses library presets and disabled zero elevation.
TEST(ShadowCurves, UsesAuthoritativeRenderer) {
  for (int level = 0; level < 32; ++level) {
    auto key = MakeSpec(roo_display::Box(48, 40, 191, 159), level, false);
    auto ambient = MakeSpec(roo_display::Box(48, 40, 191, 159), level, true);
    for (int t = 0; t < 240; ++t) {
      EXPECT_EQ(ReadLookupShadow(key, ambient, t, 0), 0);
      EXPECT_EQ(ReadLookupShadow(key, ambient, t, 239), 0);
      if (level == 0) {
        EXPECT_EQ(ReadLookupShadow(key, ambient, t, 160), 0);
      }
    }
  }
}
}  // namespace
}  // namespace shadow_curves
