#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/switch/badged_switch.h"
#include "roo_windows/material3/switch/switch.h"

namespace roo_windows {
namespace material3 {
namespace {

using roo_display::Color;

Color QuantizeToArgb4444(Color color) {
  roo_display::Argb4444 mode;
  return mode.toArgbColor(mode.fromArgbColor(color));
}

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

class RecordingPanel : public Panel {
 public:
  explicit RecordingPanel(ApplicationContext& context) : Panel(context) {}

  using Panel::add;

  std::vector<Rect> invalidated_regions;

 protected:
  void childShown(const Widget* child) override { (void)child; }

  void propagateDirty(const Widget* child, const Rect& rect) override {
    (void)child;
    (void)rect;
  }

  void childInvalidatedRegion(const Widget* child, Rect rect) override {
    (void)child;
    invalidated_regions.push_back(rect);
  }
};

class Material3BadgedSwitchRenderTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 120;
  static constexpr int16_t kHeight = 80;

  Material3BadgedSwitchRenderTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_),
        app_(&env_, display_) {}

  bool refresh(roo_time::Uptime deadline = roo_time::Uptime::Max()) {
    return app_.refresh(deadline);
  }

  Color pixelAt(int16_t x, int16_t y) const {
    int16_t px[] = {x};
    int16_t py[] = {y};
    Color color[1];
    offscreen_.raster().readColors(px, py, 1, color);
    return color[0];
  }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
  Application app_;
};

// Verifies that the Material 3 switch advertises a POINT overlay anchored at
// the thumb center and that the focus point tracks the thumb as the switch
// toggles between off (left) and on (right) positions.
TEST(Material3Switch, UsesThumbCenteredPointOverlay) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Switch sw(context, Switch::OnOffState::kOff);
  sw.layout(Rect(0, 0, Scaled(52) - 1, Scaled(32) - 1));

  EXPECT_EQ(Widget::OVERLAY_POINT, sw.getOverlayType());

  roo_display::FpPoint off_focus = sw.getPointOverlayFocus();
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(32) - 1), off_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(32) - 1), off_focus.y);

  sw.setOn();
  roo_display::FpPoint on_focus = sw.getPointOverlayFocus();
  EXPECT_FLOAT_EQ(
      0.5f * (float)(Scaled(32) - 1) + (float)(Scaled(52) - Scaled(32)),
      on_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(32) - 1), on_focus.y);
}

// Verifies that the switch reports the Material 3 prescribed minimum size
// (52x32 dp) for layout, independent of its current on/off state.
TEST(Material3Switch, ReportsMaterial3MinimumSize) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Switch sw(context, Switch::OnOffState::kOff);
  Dimensions dims = sw.getSuggestedMinimumDimensions();

  EXPECT_EQ(Scaled(52), dims.width());
  EXPECT_EQ(Scaled(32), dims.height());
}

// Verifies that the effective container color role flips with the switch
// state: surfaceContainerHighest while off (so the track reads as a neutral
// inset) and primary while on (so the track adopts the selected accent).
TEST(Material3Switch, EffectiveContainerRoleTracksState) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Switch sw(context, Switch::OnOffState::kOff);
  EXPECT_EQ(::roo_windows::material3::ColorToken::kSurfaceContainerHighest, sw.effectiveContainerRole());

  sw.setOn();
  EXPECT_EQ(::roo_windows::material3::ColorToken::kPrimary, sw.effectiveContainerRole());
}

// Verifies that the badge-aware switch opts into unclipped parent paint and
// expands its ink insets conservatively for a top-end badge overhang.
TEST(Material3BadgedSwitch, ReportsBadgeOverflowViaInkInsets) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  BadgedSwitch sw(context, false);
  sw.layout(Rect(0, 0, Scaled(52) - 1, Scaled(32) - 1));
  sw.setBadgeText("999+");

  EXPECT_EQ(ParentClipMode::kUnclipped, sw.getParentClipMode());
  Insets insets = sw.getInkInsets();
  EXPECT_LT(insets.left(), 0);
  EXPECT_EQ(0, insets.top());
  EXPECT_EQ(0, insets.right());
  EXPECT_EQ(0, insets.bottom());
}

// Verifies that changing badge content reports the union of the old and new
// badge envelopes to the parent so overhanging pixels are repainted correctly.
TEST(Material3BadgedSwitch, BadgeContentChangeInvalidatesOldAndNewEnvelope) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  RecordingPanel panel(context);
  panel.layout(Rect(0, 0, 119, 79));
  auto sw = std::make_unique<BadgedSwitch>(context, false);
  BadgedSwitch* sw_ptr = sw.get();
  panel.add(std::move(sw),
            Rect(20, 20, 20 + Scaled(52) - 1, 20 + Scaled(32) - 1));

  sw_ptr->setBadgeText("1");
  Rect old_bounds = sw_ptr->badge().bounds().translate(sw_ptr->offsetLeft(),
                                                       sw_ptr->offsetTop());
  panel.invalidated_regions.clear();

  sw_ptr->setBadgeValue(1000);
  Rect new_bounds = sw_ptr->badge().bounds().translate(sw_ptr->offsetLeft(),
                                                       sw_ptr->offsetTop());

  ASSERT_FALSE(panel.invalidated_regions.empty());
  EXPECT_EQ(Rect::Extent(old_bounds, new_bounds),
            panel.invalidated_regions.back());
}

// Verifies that the badge is painted after the switch content by checking an
// overlapping pixel inside the badge/switch intersection resolves to the badge
// error color rather than the underlying switch track.
TEST_F(Material3BadgedSwitchRenderTest, BadgePaintsFrontMostOverSwitchTrack) {
  auto sw = std::make_unique<BadgedSwitch>(app_.context(), false);
  BadgedSwitch* sw_ptr = sw.get();
  sw_ptr->setBadgeDot();
  int16_t x0 = 24;
  int16_t y0 = 24;
  app_.add(std::move(sw),
           roo_display::Box(x0, y0, x0 + Scaled(52) - 1, y0 + Scaled(32) - 1));
  ASSERT_TRUE(refresh());

  Rect overlap = Rect::Intersect(sw_ptr->bounds(), sw_ptr->badge().bounds());
  ASSERT_FALSE(overlap.empty());
  int16_t sample_x = x0 + (overlap.xMin() + overlap.xMax()) / 2;
  int16_t sample_y = y0 + (overlap.yMin() + overlap.yMax()) / 2;

  EXPECT_EQ(QuantizeToArgb4444(env_.theme().material3Theme().color.error),
            pixelAt(sample_x, sample_y));
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
