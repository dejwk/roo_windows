#include "gtest/gtest.h"
#include "roo_windows/material2/button.h"
#include "roo_windows/material2/typography.h"
#include "roo_windows/material3/palette.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {
using roo_display::Box;
using roo_display::Color;
class CoordinatedPaletteWidgetTest : public testing::Test {
 protected:
  roo::byte pixels_[64 * 48 * 2]{};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_{
      64, 48, pixels_, roo_display::Argb4444()};
  roo_display::Display display_{device_};
  roo_scheduler::SchedulingService scheduler_;
  Theme theme_;
  Environment environment_{scheduler_, theme_};
  Application app_{&environment_, display_};
  ApplicationContext& context() { return app_.context(); }

  bool containsColor(Color expected, Box bounds) {
    expected = test_support::QuantizeToArgb4444(expected);
    for (int16_t y = bounds.yMin(); y <= bounds.yMax(); ++y) {
      for (int16_t x = bounds.xMin(); x <= bounds.xMax(); ++x) {
        Color actual;
        device_.raster().readColors(&x, &y, 1, &actual);
        if (actual == expected) return true;
      }
    }
    return false;
  }
};

// Verifies installation preserves explicit widget colors and typography.
TEST_F(CoordinatedPaletteWidgetTest, PreservesExplicitOverrides) {
  material2::SimpleButton button(context(), "Apply");
  button.setInteriorColor(Color(0xFF123456));
  button.setContentColor(Color(0xFFABCDEF));
  button.setOutlineColor(Color(0xFF654321));
  button.setFont(material2::font_h6());
  auto colors = material3::DefaultColorScheme();
  colors.primary = Color(0xFFFF0000);
  ApplyCoordinatedMaterial3Palette(theme_, colors);
  EXPECT_EQ(Color(0xFF123456), button.background());
  EXPECT_EQ(Color(0xFFABCDEF), button.contentColor());
  EXPECT_EQ(Color(0xFF654321), button.getOutlineColor());
  EXPECT_EQ(&material2::font_h6(), &button.getFont());
}

// Verifies a borrowed contained button changes its live fill and readable ink,
// but the installed palette does not repaint until the application invalidates.
TEST_F(CoordinatedPaletteWidgetTest, RepaintsBorrowedSurfaceAfterInvalidation) {
  auto colors = material3::DefaultColorScheme();
  colors.primary = Color(0xFFFF0000);
  colors.onPrimary = Color(0xFFFFFFFF);
  ApplyCoordinatedMaterial3Palette(theme_, colors);
  auto button = std::make_unique<material2::SimpleButton>(context(), "A");
  auto* borrowed = button.get();
  app_.add(std::move(button), Box(0, 0, 63, 47));
  app_.refresh();
  EXPECT_TRUE(containsColor(colors.primary, Box(0, 0, 63, 47)));
  colors.primary = Color(0xFF0000FF);
  colors.onPrimary = Color(0xFFFFFF00);
  ApplyCoordinatedMaterial3Palette(theme_, colors);
  EXPECT_EQ(colors.primary, borrowed->background());
  EXPECT_EQ(colors.onPrimary, borrowed->contentColor());
  app_.refresh();
  EXPECT_TRUE(containsColor(Color(0xFFFF0000), Box(0, 0, 63, 47)));
  app_.invalidateTheme();
  app_.refresh();
  EXPECT_TRUE(containsColor(colors.primary, Box(0, 0, 63, 47)));
  EXPECT_TRUE(containsColor(colors.onPrimary, Box(0, 0, 63, 47)));
  EXPECT_FALSE(containsColor(Color(0xFFFF0000), Box(0, 0, 63, 47)));
}
}  // namespace
}  // namespace roo_windows
