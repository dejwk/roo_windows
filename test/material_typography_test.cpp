#include "gtest/gtest.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows {
namespace {

void ExpectStyle(const TextStyle& style, int16_t height, int16_t tracking) {
  EXPECT_EQ(height, style.lineHeightPx());
  EXPECT_EQ(tracking, style.trackingPx());
  EXPECT_EQ(height, style.font().metrics().ascent() -
                        style.font().metrics().descent() + style.lineGapPx());
  EXPECT_EQ(style.trackingPx(), style.fontOptions().trackingPx());
}

TEST(MaterialTypography, Material3Catalog) {
#if ROO_WINDOWS_ZOOM >= 200
  const int16_t expected_heights[] = {128, 104, 88, 80, 72, 64, 56, 48,
                                      40,  48,  40, 32, 40, 32, 32};
#elif ROO_WINDOWS_ZOOM >= 150
  const int16_t expected_heights[] = {96, 78, 66, 60, 54, 48, 42, 36,
                                      30, 36, 30, 24, 30, 24, 24};
#elif ROO_WINDOWS_ZOOM >= 100
  const int16_t expected_heights[] = {64, 52, 44, 40, 36, 32, 28, 24,
                                      20, 24, 20, 16, 20, 16, 16};
#else
  const int16_t expected_heights[] = {48, 39, 33, 30, 27, 24, 21, 18,
                                      15, 18, 15, 12, 15, 12, 12};
#endif
  const TextStyle* const styles[] = {
      &material3::text_style_display_large(),
      &material3::text_style_display_medium(),
      &material3::text_style_display_small(),
      &material3::text_style_headline_large(),
      &material3::text_style_headline_medium(),
      &material3::text_style_headline_small(),
      &material3::text_style_title_large(),
      &material3::text_style_title_medium(),
      &material3::text_style_title_small(),
      &material3::text_style_body_large(),
      &material3::text_style_body_medium(),
      &material3::text_style_body_small(),
      &material3::text_style_label_large(),
      &material3::text_style_label_medium(),
      &material3::text_style_label_small(),
  };
  for (size_t i = 0; i < sizeof(styles) / sizeof(styles[0]); ++i) {
    const int16_t tracking =
        ROO_WINDOWS_ZOOM >= 150 ? (i == 9 || i == 11 || i == 13 || i == 14) : 0;
    ExpectStyle(*styles[i], expected_heights[i], tracking);
  }
}

TEST(MaterialTypography, Material2FontHelpersRemainCompatible) {
  EXPECT_EQ(&font_h1(), &material2::text_style_h1().font());
  EXPECT_EQ(&font_button(), &material2::text_style_button().font());
  EXPECT_EQ(&font_overline(), &material2::text_style_overline().font());
}

TEST(MaterialTypography, TextStyleExposesLineBoxAndTracking) {
  const TextStyle style(font_body1(), 3, -2);
  EXPECT_EQ(3, style.lineGapPx());
  EXPECT_EQ(1, style.topLeadingPx());
  EXPECT_EQ(font_body1().metrics().ascent() + 1, style.baselineOffsetPx());
  EXPECT_EQ(-2, style.fontOptions().trackingPx());
}

}  // namespace
}  // namespace roo_windows
