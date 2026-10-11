#include <memory>
#include <vector>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "material3_density_acceptance_theme.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/toolbar/toolbar.h"

namespace roo_windows::material3 {
namespace {

roo_display::Offscreen<roo_display::Rgb888> RenderDocked(bool dark) {
  constexpr int16_t kWidth = 320;
  constexpr int16_t kHeight = 160;
  std::vector<roo::byte> raster(kWidth * kHeight * 2);
  roo_display::OffscreenDevice<roo_display::Rgb565> device(
      kWidth, kHeight, raster.data(), roo_display::Rgb565());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Material3Theme material = test_support::MakeAcceptanceTheme(dark);
  Theme theme{MakeFrameworkTheme(material), &material};
  Environment env(scheduler, theme);
  Application app(&env, display);

  for (int row = 0; row < 2; ++row) {
    auto toolbar = std::make_unique<DockedToolbar>(app.context());
    toolbar->setArrangement(row == 0 ? DockedToolbarArrangement::kEdgeAnchored
                                     : DockedToolbarArrangement::kCentered);
    toolbar->setShowGroupDividers(true);
    if (row == 1) toolbar->setColorStyle(ToolbarColorStyle::kVibrant);
    toolbar->add(std::make_unique<Button>(app.context(), "Pause",
                                          ButtonVariant::kFilledTonal),
                 ToolbarSlot::kLeading);
    toolbar->add(std::make_unique<Button>(app.context(), "Save",
                                          ButtonVariant::kFilledTonal),
                 ToolbarSlot::kCenter);
    toolbar->add(std::make_unique<Button>(app.context(), "More",
                                          ButtonVariant::kFilledTonal),
                 ToolbarSlot::kTrailing);
    const int y = 8 + row * 72;
    app.add(std::move(toolbar), roo_display::Box(0, y, kWidth - 1, y + 63));
  }
  app.refresh();
  return test::CaptureRgb(device.raster(), 0, 0, kWidth, kHeight);
}

// Verifies standard and vibrant docked surfaces, grouped buttons, and dividers
// settle to stable final pixels with the light palette.
TEST(Material3ToolbarGolden, DockedLight) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderDocked(false), "test/goldens/material3_toolbar/docked_light.ppm",
      "material3_toolbar_docked_light"));
}

// Verifies the same toolbar layout against the dark Material palette.
TEST(Material3ToolbarGolden, DockedDark) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderDocked(true), "test/goldens/material3_toolbar/docked_dark.ppm",
      "material3_toolbar_docked_dark"));
}

}  // namespace
}  // namespace roo_windows::material3
