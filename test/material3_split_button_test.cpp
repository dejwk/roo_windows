#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/touch_sensor.h"
#include "roo_windows/material3/button/split_button.h"

namespace roo_windows::material3 {
namespace {

class TestPanel final : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeLast;
};

class ManualTouchDevice final : public roo_display::TouchDevice {
 public:
  void set(bool down, int x, int y) {
    down_ = down;
    x_ = x;
    y_ = y;
  }

  roo_display::TouchResult getTouch(roo_display::TouchPoint* points,
                                    int max_points) override {
    if (!down_ || max_points == 0) {
      return roo_display::TouchResult(roo_time::Uptime::Now(), 0);
    }
    points[0].id = 0;
    points[0].x = 4095LL * x_ / 319;
    points[0].y = 4095LL * y_ / 239;
    points[0].z = 100;
    points[0].vx = 0;
    points[0].vy = 0;
    return roo_display::TouchResult(roo_time::Uptime::Now(), 1);
  }

 private:
  bool down_ = false;
  int x_ = 0;
  int y_ = 0;
};

class SplitButtonTest : public testing::Test {
 protected:
  SplitButtonTest()
      : device_(320, 240, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_),
        content_(app_.context()),
        owner_(app_.addTaskFullScreen(content_)),
        row_(app_.context()),
        group_(app_.context()),
        menu_(app_.context()),
        button_(app_.context(), menu_, "Run") {
    row_.setMenuItem(item_);
    group_.add(row_);
    menu_.addGroup(group_);
    content_.add(WidgetRef(button_), Rect(20, 20, 179, 59));
    app_.refresh();
  }

  ~SplitButtonTest() override {
    button_.dismissMenu();
    content_.removeLast();
    menu_.clearGroups();
    group_.clear();
  }

  Widget& primary() { return *static_cast<Widget&>(button_).focusChildAt(0); }
  Widget& trailing() { return *static_cast<Widget&>(button_).focusChildAt(1); }

  roo::byte raster_[320 * 240 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Environment environment_;
  Application app_;
  TestPanel content_;
  Task& owner_;
  StandardMenuItem item_{StandardMenuItemInit{"Run once"}};
  MenuEntry row_;
  MenuGroup group_;
  Menu menu_;
  SplitButton button_;
};

// Verifies primary delivery runs only the container callback while the menu
// action takes ownership of the borrowed menu presentation.
TEST_F(SplitButtonTest, PrimaryAndTrailingActionsStayIndependent) {
  int runs = 0;
  button_.setOnInteractiveChange([&runs] { ++runs; });
  primary().onClicked();
  EXPECT_EQ(1, runs);
  EXPECT_FALSE(button_.isMenuOpen());

  trailing().onClicked();
  EXPECT_TRUE(button_.isMenuOpen());
  EXPECT_EQ(1, runs);
  EXPECT_EQ(MenuShowResult::kAlreadyPresented, button_.showMenu());
  button_.dismissMenu();
  EXPECT_FALSE(button_.isMenuOpen());
  EXPECT_EQ(1, runs);
}

// Verifies the chevron uses the shared click controller through sampled,
// held, and terminal feedback without adding a widget animation track.
TEST_F(SplitButtonTest, ChevronFollowsClickProgressWithoutMovingSegments) {
  Rect original = trailing().parent_bounds();
  trailing().onShowPress(20, 20);
  app_.refresh();
  ASSERT_NE(nullptr, trailing().getClickAnimation());
  system_time_delay_micros(kPressAnimationMillis * 500);
  app_.refresh();
  ASSERT_NE(nullptr, trailing().getClickAnimation());
  EXPECT_NEAR(0.5f, trailing().getClickAnimation()->progress(), 0.001f);
  EXPECT_FALSE(button_.isMenuOpen());
  EXPECT_FALSE(app_.context().animations().contains(button_, 0));
  EXPECT_EQ(original, trailing().parent_bounds());
  system_time_delay_micros(kPressAnimationMillis * 500);
  app_.refresh();
  EXPECT_TRUE(trailing().isPressed());
  EXPECT_FALSE(button_.isMenuOpen());
  trailing().onSingleTapUp(20, 20);
  app_.refresh();
  EXPECT_TRUE(button_.isMenuOpen());
  EXPECT_EQ(original, trailing().parent_bounds());
  button_.dismissMenu();
  EXPECT_FALSE(app_.context().animations().contains(button_, 0));
}

// Verifies the exact child rectangles leave a noninteractive gutter, including
// when ordinary sloppy targeting would enlarge a compact segment.
TEST_F(SplitButtonTest, GutterHasNoClickTarget) {
  std::vector<Widget*> path;
  int gap_x = button_.offsetLeft() + primary().offsetLeft() + primary().width();
  int y = button_.offsetTop() + 20;
  ASSERT_TRUE(app_.root().fillTouchTargetPath(gap_x, y, path));
  EXPECT_NE(&primary(), path.back());
  EXPECT_NE(&trailing(), path.back());
}

// Verifies the menu is closed synchronously before disabling children and can
// be reused after the control becomes eligible again.
TEST_F(SplitButtonTest, DisableCancelsOwnedPresentation) {
  ASSERT_EQ(MenuShowResult::kShown, button_.showMenu());
  button_.setEnabled(false);
  EXPECT_FALSE(button_.isMenuOpen());
  EXPECT_FALSE(primary().isEnabled());
  EXPECT_FALSE(trailing().isEnabled());
  button_.setEnabled(true);
  EXPECT_EQ(MenuShowResult::kShown, button_.showMenu());
}

// Verifies a second trigger cannot claim the first trigger's active menu.
TEST_F(SplitButtonTest, SharedMenuAdmissionDoesNotTransferOpenState) {
  SplitButton other(app_.context(), menu_, "Run again");
  content_.add(WidgetRef(other), Rect(20, 80, 219, 119));
  app_.refresh();
  ASSERT_EQ(MenuShowResult::kShown, button_.showMenu());
  EXPECT_EQ(MenuShowResult::kAlreadyPresented, other.showMenu());
  EXPECT_FALSE(other.isMenuOpen());
  button_.dismissMenu();
  content_.removeLast();
}

// Verifies both child buttons participate in ordinary task focus traversal.
TEST_F(SplitButtonTest, PrimaryAndTrailingAreDistinctFocusStops) {
  ASSERT_TRUE(primary().requestFocus());
  EXPECT_EQ(&primary(), owner_.focus().focused());
  ASSERT_TRUE(owner_.focus().moveFocus(trailing(), false));
  EXPECT_EQ(&trailing(), owner_.focus().focused());
}

// Verifies a shared theme or layout refresh ends a captured menu anchor before
// child geometry can be recomputed.
TEST_F(SplitButtonTest, LayoutRefreshCancelsOpenMenu) {
  ASSERT_EQ(MenuShowResult::kShown, button_.showMenu());
  button_.requestLayoutDescending();
  EXPECT_FALSE(button_.isMenuOpen());
  app_.refresh();
  EXPECT_EQ(MenuShowResult::kShown, button_.showMenu());
}

// Verifies a real gesture that starts on the primary and ends over the
// trailing child cannot transfer either semantic action.
TEST(SplitButtonInput, CrossChildReleaseDoesNotInvokeEitherAction) {
  roo::byte raster[320 * 240 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      320, 240, raster, roo_display::Argb4444());
  ManualTouchDevice touch;
  roo_display::Display display(device, touch);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  TestPanel panel(app.context());
  app.addTaskFullScreen(panel);
  Menu menu(app.context());
  SplitButton button(app.context(), menu, "Run");
  int primary_runs = 0;
  button.setOnInteractiveChange([&primary_runs] { ++primary_runs; });
  panel.add(WidgetRef(button), Rect(20, 20, 179, 59));
  app.refresh();
  TouchSensor sensor(display);
  GestureDetector detector(app.root(), sensor);

  touch.set(true, 50, 40);
  sensor.pollOnce();
  detector.tick();
  touch.set(true, 160, 40);
  sensor.pollOnce();
  detector.tick();
  touch.set(false, 160, 40);
  sensor.pollOnce();
  detector.tick();
  app.refresh();

  EXPECT_EQ(0, primary_runs);
  EXPECT_FALSE(button.isMenuOpen());
  panel.removeLast();
}

}  // namespace
}  // namespace roo_windows::material3
