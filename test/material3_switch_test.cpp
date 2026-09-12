#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/switch/switch.h"
#include "roo_windows/widgets/switch.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace material3 {
namespace {

using roo_display::Color;
using test_support::RooWindowsRenderTestSized;

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

// Verifies that the switch lets its thumb-centered point overlay escape a
// tight structural parent by default.
TEST(Material3Switch, IsParentUnclippedByDefault) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Switch sw(context);

  EXPECT_EQ(ParentClipMode::kUnclipped, sw.getParentClipMode());
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

class TestMaterialSwitch : public Switch {
 public:
  using Switch::Switch;

  bool thumbAnimationActive() const {
    return context().animations().contains(*this, kThumb);
  }
};

class TestLegacySwitch : public ::roo_windows::Switch {
 public:
  using ::roo_windows::Switch::Switch;

  bool thumbAnimationActive() const {
    return context().animations().contains(*this, kThumb);
  }
};

class SwitchAnimationTest
    : public RooWindowsRenderTestSized<Scaled(120), Scaled(60)> {};

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
  EXPECT_EQ(::roo_windows::material3::ColorToken::kSurfaceContainerHighest,
            sw.effectiveContainerRole());

  sw.setOn();
  EXPECT_EQ(::roo_windows::material3::ColorToken::kPrimary,
            sw.effectiveContainerRole());
}

// Verifies a click starts the 100 ms Material thumb value track while the
// independent click overlay remains active.
TEST_F(SwitchAnimationTest, MaterialClickAnimationCoexistsWithThumbTrack) {
  auto sw = std::make_unique<TestMaterialSwitch>(context());
  TestMaterialSwitch* sw_ptr = sw.get();
  app_.add(std::move(sw),
           roo_display::Box(Scaled(10), Scaled(8), Scaled(10) + Scaled(52) - 1,
                            Scaled(8) + Scaled(32) - 1));
  ASSERT_TRUE(refresh());
  const float off_x = sw_ptr->getPointOverlayFocus().x;

  sw_ptr->onSingleTapUp(sw_ptr->width() / 2, sw_ptr->height() / 2);
  EXPECT_TRUE(sw_ptr->isOn());
  EXPECT_TRUE(sw_ptr->isClicking());
  EXPECT_TRUE(sw_ptr->thumbAnimationActive());
  EXPECT_FLOAT_EQ(off_x, sw_ptr->getPointOverlayFocus().x);

  ASSERT_TRUE(refresh());
  delay(50);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(sw_ptr->isClicking());
  EXPECT_TRUE(sw_ptr->thumbAnimationActive());
  EXPECT_GT(sw_ptr->getPointOverlayFocus().x, off_x);
  EXPECT_LT(sw_ptr->getPointOverlayFocus().x,
            off_x + static_cast<float>(Scaled(20)));

  delay(60);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(sw_ptr->thumbAnimationActive());
  EXPECT_FLOAT_EQ(off_x + static_cast<float>(Scaled(20)),
                  sw_ptr->getPointOverlayFocus().x);
}

// Verifies a rapid reverse starts from the last applied Material thumb
// fraction rather than jumping to either endpoint.
TEST_F(SwitchAnimationTest, MaterialRapidToggleRetargetsContinuously) {
  auto sw = std::make_unique<TestMaterialSwitch>(context());
  TestMaterialSwitch* sw_ptr = sw.get();
  app_.add(std::move(sw),
           roo_display::Box(Scaled(10), Scaled(8), Scaled(10) + Scaled(52) - 1,
                            Scaled(8) + Scaled(32) - 1));
  ASSERT_TRUE(refresh());
  const float off_x = sw_ptr->getPointOverlayFocus().x;

  sw_ptr->onClicked();
  ASSERT_TRUE(refresh());
  delay(40);
  ASSERT_TRUE(refresh());
  const float midpoint = sw_ptr->getPointOverlayFocus().x;
  ASSERT_GT(midpoint, off_x);

  sw_ptr->onClicked();
  ASSERT_FALSE(sw_ptr->isOn());
  ASSERT_TRUE(refresh());
  EXPECT_FLOAT_EQ(midpoint, sw_ptr->getPointOverlayFocus().x);
  delay(40);
  ASSERT_TRUE(refresh());
  EXPECT_LT(sw_ptr->getPointOverlayFocus().x, midpoint);
  delay(70);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(sw_ptr->thumbAnimationActive());
  EXPECT_FLOAT_EQ(off_x, sw_ptr->getPointOverlayFocus().x);
}

// Verifies the legacy switch preserves its distinct 120 ms duration while
// using the same application-owned value-track mechanism.
TEST_F(SwitchAnimationTest, LegacySwitchUsesOneHundredTwentyMillisecondTrack) {
  auto sw = std::make_unique<TestLegacySwitch>(context());
  TestLegacySwitch* sw_ptr = sw.get();
  app_.add(std::move(sw),
           roo_display::Box(Scaled(10), Scaled(10), Scaled(10) + Scaled(42) - 1,
                            Scaled(10) + Scaled(24) - 1));
  ASSERT_TRUE(refresh());
  const float off_x = sw_ptr->getPointOverlayFocus().x;

  sw_ptr->onClicked();
  ASSERT_TRUE(refresh());
  delay(60);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(sw_ptr->thumbAnimationActive());
  EXPECT_GT(sw_ptr->getPointOverlayFocus().x, off_x);
  EXPECT_LT(sw_ptr->getPointOverlayFocus().x,
            off_x + static_cast<float>(Scaled(19)));

  delay(70);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(sw_ptr->thumbAnimationActive());
  EXPECT_FLOAT_EQ(off_x + static_cast<float>(Scaled(19)),
                  sw_ptr->getPointOverlayFocus().x);
}

// Verifies programmatic setters retain their immediate behavior and cancel a
// click-started Material or legacy transition.
TEST_F(SwitchAnimationTest, ProgrammaticSettersSnapAndCancelTracks) {
  auto host = std::make_unique<RecordingPanel>(context());
  auto material = std::make_unique<TestMaterialSwitch>(context());
  TestMaterialSwitch* material_ptr = material.get();
  auto legacy = std::make_unique<TestLegacySwitch>(context());
  TestLegacySwitch* legacy_ptr = legacy.get();
  host->add(std::move(material),
            Rect(Scaled(5), Scaled(8), Scaled(5) + Scaled(52) - 1,
                 Scaled(8) + Scaled(32) - 1));
  host->add(std::move(legacy),
            Rect(Scaled(68), Scaled(10), Scaled(68) + Scaled(42) - 1,
                 Scaled(10) + Scaled(24) - 1));
  app_.add(std::move(host), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  ASSERT_TRUE(refresh());

  material_ptr->onClicked();
  legacy_ptr->onClicked();
  ASSERT_TRUE(material_ptr->thumbAnimationActive());
  ASSERT_TRUE(legacy_ptr->thumbAnimationActive());
  material_ptr->setOff();
  legacy_ptr->setOff();

  EXPECT_FALSE(material_ptr->thumbAnimationActive());
  EXPECT_FALSE(legacy_ptr->thumbAnimationActive());
  EXPECT_FLOAT_EQ(0.5f * static_cast<float>(Scaled(32) - 1),
                  material_ptr->getPointOverlayFocus().x);
  EXPECT_FLOAT_EQ(0.5f * static_cast<float>(Scaled(24) - 1),
                  legacy_ptr->getPointOverlayFocus().x);
}

// Verifies hidden Material switches cancel and snap to logical state without
// resuming stale animation when shown again.
TEST_F(SwitchAnimationTest, HiddenMaterialSwitchSnapsWithoutResume) {
  auto sw = std::make_unique<TestMaterialSwitch>(context());
  TestMaterialSwitch* sw_ptr = sw.get();
  app_.add(std::move(sw),
           roo_display::Box(Scaled(10), Scaled(8), Scaled(10) + Scaled(52) - 1,
                            Scaled(8) + Scaled(32) - 1));
  ASSERT_TRUE(refresh());
  const float off_x = sw_ptr->getPointOverlayFocus().x;
  sw_ptr->onClicked();
  ASSERT_TRUE(refresh());
  delay(40);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(sw_ptr->thumbAnimationActive());

  sw_ptr->setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(sw_ptr->thumbAnimationActive());
  EXPECT_FLOAT_EQ(off_x + static_cast<float>(Scaled(20)),
                  sw_ptr->getPointOverlayFocus().x);

  sw_ptr->setVisibility(Visibility::kVisible);
  ASSERT_TRUE(refresh());
  delay(120);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(sw_ptr->thumbAnimationActive());
  EXPECT_FLOAT_EQ(off_x + static_cast<float>(Scaled(20)),
                  sw_ptr->getPointOverlayFocus().x);
}

// Verifies detaching a borrowed legacy switch cancels and snaps its track
// while leaving the widget safe for later destruction by its caller.
TEST_F(SwitchAnimationTest, DetachedLegacySwitchSnapsWithoutResume) {
  TestLegacySwitch sw(context());
  Task& task = app_.addTaskFullScreen(sw);
  ASSERT_TRUE(refresh());
  const float off_x = sw.getPointOverlayFocus().x;
  sw.onClicked();
  ASSERT_TRUE(refresh());
  delay(50);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(sw.thumbAnimationActive());

  task.navigation().clear();
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(sw.thumbAnimationActive());
  EXPECT_FLOAT_EQ(off_x + static_cast<float>(Scaled(19)),
                  sw.getPointOverlayFocus().x);
  delay(140);
  ASSERT_TRUE(refresh());
  EXPECT_FLOAT_EQ(off_x + static_cast<float>(Scaled(19)),
                  sw.getPointOverlayFocus().x);
}

// Records the idle widget costs separately from the registry's bounded active
// record (budgeted at 128 bytes in the animation-registry design).
TEST(Material3Switch, RegistryMigrationKeepsCompactIdleState) {
  EXPECT_LE(sizeof(Switch), sizeof(BasicWidget) + 2 * sizeof(void*) + 8U);
  EXPECT_LE(sizeof(::roo_windows::Switch), sizeof(BasicWidget) + 8U);
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
