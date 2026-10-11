#include <memory>
#include <type_traits>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/fab/fab.h"
#include "roo_windows/material3/internal/elevation.h"

namespace roo_windows::material3 {
namespace {

constexpr size_t AlignUp(size_t n, size_t alignment) {
  return (n + alignment - 1) / alignment * alignment;
}

static_assert(sizeof(FloatingActionButton) <=
              AlignUp(sizeof(SurfaceWidget) + sizeof(const MonoIcon*) + 1,
                      alignof(FloatingActionButton)));
static_assert(sizeof(ExtendedFloatingActionButton) <=
              AlignUp(sizeof(SurfaceWidget) + sizeof(roo::string_view) +
                          sizeof(const MonoIcon*) + 1,
                      alignof(ExtendedFloatingActionButton)));
static_assert(!std::is_constructible<FloatingActionButton, ApplicationContext&,
                                     const MonoIcon*>::value);
static_assert(
    !std::is_constructible<ExtendedFloatingActionButton, ApplicationContext&,
                           roo::string_view, const MonoIcon*>::value);

class PaintedSurface : public SurfaceWidget {
 public:
  PaintedSurface(ApplicationContext& context, bool automatic)
      : SurfaceWidget(context), automatic_(automatic) {}
  Color background() const override { return Color(0xFFFF0000); }
  bool useAutomaticDisabledStyle() const override { return automatic_; }
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(32, 32);
  }
  void paint(PaintContext& ctx) const override { ctx.clear(); }

 private:
  bool automatic_;
};

TEST(Material3Fab, Defaults) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context(env.scheduler(), env.theme(),
                             env.keyboardColorTheme());
  const MonoIcon& icon = ic_outlined_24_action_done();
  FloatingActionButton fab(context, icon);
  ExtendedFloatingActionButton extended(context, "Done", icon);
  EXPECT_EQ(&icon, &fab.icon());
  EXPECT_EQ(FabSize::kRegular, fab.size());
  EXPECT_EQ(FabColorStyle::kPrimaryContainer, fab.colorStyle());
  EXPECT_EQ(ExtendedFabSize::kSmall, extended.size());
  EXPECT_TRUE(extended.expanded());
  EXPECT_TRUE(fab.isClickable());
  EXPECT_TRUE(extended.isClickable());
}

TEST(Material3Fab, IconSizesAndStableCorners) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context(env.scheduler(), env.theme(),
                             env.keyboardColorTheme());
  FloatingActionButton fab(context, ic_outlined_24_action_done());
  const FabSize sizes[] = {FabSize::kSmall, FabSize::kRegular, FabSize::kMedium,
                           FabSize::kLarge};
  const int heights[] = {40, 56, 80, 96};
  for (int i = 0; i < 4; ++i) {
    fab.setSize(sizes[i]);
    EXPECT_EQ(Scaled(heights[i]), fab.getNaturalDimensions().width());
    EXPECT_EQ(Scaled(heights[i]), fab.getNaturalDimensions().height());
    EXPECT_EQ(
        fab.getNaturalDimensions().width(),
        fab.measure(WidthSpec::AtMost(200), HeightSpec::AtMost(200)).width());
  }
}

TEST(Material3Fab, ColorsAndElevationFollowThemeAndState) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context(env.scheduler(), env.theme(),
                             env.keyboardColorTheme());
  FloatingActionButton fab(context, ic_outlined_24_action_done());
  const ColorScheme& colors = env.theme().material3Theme().color;
  EXPECT_EQ(ColorToken::kPrimaryContainer, fab.containerRole());
  EXPECT_EQ(colors.primaryContainer, fab.background());
  EXPECT_EQ(internal::RendererElevation(3), fab.getElevation());
  fab.setHover(true);
  EXPECT_EQ(internal::RendererElevation(4), fab.getElevation());
  fab.setFocused(true);
  EXPECT_EQ(internal::RendererElevation(3), fab.getElevation());
  fab.setFocused(false);
  fab.setColorStyle(FabColorStyle::kSurface);
  EXPECT_EQ(ColorToken::kSurfaceContainerHigh, fab.containerRole());
  EXPECT_TRUE(fab.usesHighlighterColor());
  EXPECT_EQ(colors.surfaceContainerHigh, fab.background());
  fab.setEnabled(false);
  EXPECT_EQ(internal::RendererElevation(0), fab.getElevation());
  EXPECT_EQ(
      roo_display::AlphaBlend(colors.surface, colors.onSurface.withA(0x1F)),
      fab.background());
}

TEST(Material3Fab, ExplicitDisabledSurfaceKeepsItsResolvedFill) {
  roo::byte pixels[128 * 48 * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      128, 48, pixels, roo_display::Argb4444());
  roo_display::Display display(offscreen);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  auto automatic = std::make_unique<PaintedSurface>(app.context(), true);
  auto explicit_color = std::make_unique<PaintedSurface>(app.context(), false);
  automatic->setEnabled(false);
  explicit_color->setEnabled(false);
  app.add(std::move(automatic), roo_display::Box(8, 8, 39, 39));
  app.add(std::move(explicit_color), roo_display::Box(72, 8, 103, 39));
  app.refresh();
  int16_t xs[] = {24, 88};
  int16_t ys[] = {24, 24};
  Color actual[2];
  offscreen.raster().readColors(xs, ys, 2, actual);
  EXPECT_NE(actual[0], actual[1]);
  EXPECT_EQ(Color(0xFFFF0000), actual[1]);
}

}  // namespace
}  // namespace roo_windows::material3
