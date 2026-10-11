#include <memory>
#include <vector>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "material3_density_acceptance_theme.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/static_layout.h"
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

roo_display::Offscreen<roo_display::Rgb888> RenderFloating(bool dark) {
  constexpr int16_t kWidth = 320;
  constexpr int16_t kHeight = 280;
  std::vector<roo::byte> raster(kWidth * kHeight * 2);
  roo_display::OffscreenDevice<roo_display::Rgb565> device(
      kWidth, kHeight, raster.data(), roo_display::Rgb565());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Material3Theme material = test_support::MakeAcceptanceTheme(dark);
  Theme theme{MakeFrameworkTheme(material), &material};
  Environment env(scheduler, theme);
  Application app(&env, display);

  auto horizontal = std::make_unique<FloatingToolbar>(app.context());
  horizontal->add(std::make_unique<Button>(app.context(), "Pen",
                                           ButtonVariant::kFilledTonal));
  horizontal->add(std::make_unique<Button>(app.context(), "Line",
                                           ButtonVariant::kFilledTonal));
  horizontal->add(std::make_unique<Button>(app.context(), "Erase",
                                           ButtonVariant::kFilledTonal));
  const Dimensions horizontal_size = horizontal->measure(
      WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  app.add(std::move(horizontal),
          roo_display::Box(16, 16, 15 + horizontal_size.width(),
                           15 + horizontal_size.height()));

  auto vertical = std::make_unique<FloatingToolbar>(app.context());
  vertical->setOrientation(FloatingToolbarOrientation::kVertical);
  vertical->setColorStyle(ToolbarColorStyle::kVibrant);
  vertical->add(std::make_unique<Button>(app.context(), "Pen",
                                         ButtonVariant::kFilledTonal));
  vertical->add(std::make_unique<Button>(app.context(), "Line",
                                         ButtonVariant::kFilledTonal));
  vertical->add(std::make_unique<Button>(app.context(), "Erase",
                                         ButtonVariant::kFilledTonal));
  const Dimensions vertical_size =
      vertical->measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  app.add(std::move(vertical),
          roo_display::Box(208, 104, 207 + vertical_size.width(),
                           103 + vertical_size.height()));
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

// Verifies rounded horizontal and vertical surfaces, shadows, and contained
// buttons settle correctly against the light palette.
TEST(Material3ToolbarGolden, FloatingLight) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderFloating(false),
      "test/goldens/material3_toolbar/floating_light.ppm",
      "material3_toolbar_floating_light"));
}

// Verifies both floating orientations and their surface treatments in dark.
TEST(Material3ToolbarGolden, FloatingDark) {
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      RenderFloating(true), "test/goldens/material3_toolbar/floating_dark.ppm",
      "material3_toolbar_floating_dark"));
}

// Verifies rounded corners and shadow damage are restored when a floating
// toolbar is moved, has elevation toggled, and is finally hidden.
TEST(Material3ToolbarGolden, FloatingMoveAndHideRestoresBackdrop) {
  constexpr int16_t kWidth = 240;
  constexpr int16_t kHeight = 320;
  std::vector<roo::byte> raster(kWidth * kHeight * 2);
  roo_display::OffscreenDevice<roo_display::Rgb565> device(
      kWidth, kHeight, raster.data(), roo_display::Rgb565());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  StaticLayout host(app.context());
  Task& task = app.addTaskFullScreen(host);
  app.refresh();
  const std::vector<roo::byte> baseline = raster;

  auto toolbar = std::make_unique<FloatingToolbar>(app.context());
  FloatingToolbar* placed = toolbar.get();
  toolbar->add(std::make_unique<Button>(app.context(), "Draw"));
  const Dimensions size =
      toolbar->measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  host.add(std::move(toolbar), 16, 24);
  app.refresh();
  placed->layout(Rect(112, 88, 111 + size.width(), 87 + size.height()));
  app.refresh();
  placed->setElevated(false);
  app.refresh();
  placed->setVisibility(Visibility::kGone);
  app.refresh();
  EXPECT_EQ(baseline, raster);
  task.navigation().clear();
}

}  // namespace
}  // namespace roo_windows::material3
