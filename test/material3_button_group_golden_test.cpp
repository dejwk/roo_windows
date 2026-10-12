#include <vector>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "material3_density_acceptance_theme.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/button/internal/button_group_appearance.h"
namespace roo_windows::material3 {
namespace {
class GoldenPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeAll;
};

class PaletteSurface : public SurfaceWidget {
 public:
  PaletteSurface(ApplicationContext& context, internal::GroupStyle style,
                 bool selected, bool enabled)
      : SurfaceWidget(context),
        style_(style),
        selected_(selected),
        enabled_(enabled) {}
  Color background() const override {
    return internal::ResolveConnectedAppearance(theme(), style_, selected_,
                                                enabled_)
        .container;
  }
  Color getOutlineColor() const override {
    return internal::ResolveConnectedAppearance(theme(), style_, selected_,
                                                enabled_)
        .outline;
  }
  BorderStyle getBorderStyle() const override {
    return internal::ResolveConnectedBorder(
        ButtonSize::kSmall, ButtonShape::kRound, LayoutDirection::kLeftToRight,
        true, false, selected_, 0, {width(), height()},
        style_ == internal::GroupStyle::kOutlined && !selected_ ? Scaled(1)
                                                                : 0);
  }
  bool useAutomaticDisabledStyle() const override { return false; }
  Dimensions getSuggestedMinimumDimensions() const override { return {60, 40}; }

 private:
  internal::GroupStyle style_;
  bool selected_;
  bool enabled_;
};
// Verifies real surface rendering for each palette and selection combination.
TEST(ButtonGroupGolden, ConnectedHelperSurfaces) {
  for (bool dark : {false, true}) {
    std::vector<roo::byte> raster(320 * 160 * 2);
    roo_display::OffscreenDevice<roo_display::Argb4444> device(
        320, 160, raster.data(), {});
    roo_display::Display display(device);
    roo_scheduler::SchedulingService scheduler;
    Material3Theme material = test_support::MakeAcceptanceTheme(dark);
    Theme theme{MakeFrameworkTheme(material), &material};
    Environment env(scheduler, theme);
    Application app(&env, display);
    GoldenPanel panel(app.context());
    std::vector<std::unique_ptr<PaletteSurface>> entries;
    app.addTaskFullScreen(panel);
    for (int i = 0; i < 12; ++i) {
      entries.push_back(std::make_unique<PaletteSurface>(
          app.context(), static_cast<internal::GroupStyle>(i % 4), i >= 4,
          i < 8));
      panel.add(WidgetRef(*entries.back()),
                Rect(8 + (i % 4) * 76, 8 + (i / 4) * 48, 67 + (i % 4) * 76,
                     47 + (i / 4) * 48));
    }
    app.refresh();
    std::string name = dark ? "helper_dark" : "helper_light";
    EXPECT_TRUE(test::CompareOrUpdateGolden(
        test::CaptureRgb(device.raster(), 0, 0, 320, 160),
        "test/goldens/material3_button_group/" + name + ".ppm", name));
    panel.removeAll();
  }
}
}  // namespace
}  // namespace roo_windows::material3
