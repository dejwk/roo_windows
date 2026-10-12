#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/key_source.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/button/button_group.h"
namespace roo_windows::test {
struct ApplicationWorkTestAccess {
  static void PollPointer(Application& app) {
    app.window_.touch_sensor_.pollOnce();
    app.window_.gesture_detector_.tick();
  }
  static void DrainKeys(Application& app) { app.drainKeyEvents(); }
};
}  // namespace roo_windows::test

namespace roo_windows::material3 {
namespace {
class InputPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeAll;
};
class ManualGroupTouch : public roo_display::TouchDevice {
 public:
  void set(bool down, int x, int y) {
    down_ = down;
    x_ = x;
    y_ = y;
  }
  roo_display::TouchResult getTouch(roo_display::TouchPoint* points,
                                    int max_points) override {
    if (!down_ || max_points == 0) return {roo_time::Uptime::Now(), 0};
    points[0].id = 0;
    points[0].x = 4095LL * x_ / 319;
    points[0].y = 4095LL * y_ / 239;
    points[0].z = 100;
    points[0].vx = 0;
    points[0].vy = 0;
    return {roo_time::Uptime::Now(), 1};
  }

 private:
  bool down_ = false;
  int x_ = 0;
  int y_ = 0;
};
class GroupKeys : public KeySource {
 public:
  void push(KeyEvent event) {
    pending_.push_back(event);
    notifyReady();
  }
  int drain(KeyEvent* output, int count) override {
    int take = std::min<int>(count, pending_.size());
    for (int i = 0; i < take; ++i) output[i] = pending_[i];
    pending_.erase(pending_.begin(), pending_.begin() + take);
    return take;
  }

 protected:
  bool hasPendingEvents() const override { return !pending_.empty(); }

 private:
  std::vector<KeyEvent> pending_;
};
class ConnectedGroupInput : public testing::Test {
 protected:
  ConnectedGroupInput()
      : raster_(320 * 240 * 2),
        device_(320, 240, raster_.data(), {}),
        display_(device_, touch_),
        env_(scheduler_),
        app_(&env_, display_),
        panel_(app_.context()),
        task_(app_.addTaskFullScreen(panel_)),
        day_(app_.context(), "Day"),
        week_(app_.context(), "Week"),
        month_(app_.context(), "Month"),
        group_(app_.context()) {
    keys_.connect(task_);
    group_.add(day_);
    group_.add(week_);
    group_.add(month_);
    panel_.add(WidgetRef(group_), Rect(20, 20, 279, 69));
    app_.refresh();
  }
  ~ConnectedGroupInput() override { panel_.removeAll(); }
  void key(KeyCode code, KeyPhase phase = KeyPhase::kDown,
           uint8_t modifiers = 0) {
    PhysicalKey physical = PhysicalKey::kEnter;
    switch (code) {
      case KeyCode::kSpace:
        physical = PhysicalKey::kSpace;
        break;
      case KeyCode::kLeft:
        physical = PhysicalKey::kLeft;
        break;
      case KeyCode::kRight:
        physical = PhysicalKey::kRight;
        break;
      case KeyCode::kHome:
        physical = PhysicalKey::kHome;
        break;
      case KeyCode::kEnd:
        physical = PhysicalKey::kEnd;
        break;
      default:
        break;
    }
    keys_.push({phase, code, modifiers, physical, 0});
    test::ApplicationWorkTestAccess::DrainKeys(app_);
    app_.refresh();
  }
  void pollTouch() {
    test::ApplicationWorkTestAccess::PollPointer(app_);
    app_.refresh();
  }
  void tap(Widget& entry) {
    touch_.set(true, 20 + entry.offsetLeft() + entry.width() / 2, 40);
    pollTouch();
    touch_.set(false, 0, 0);
    pollTouch();
  }
  std::vector<roo::byte> raster_;
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  ManualGroupTouch touch_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Environment env_;
  Application app_;
  InputPanel panel_;
  Task& task_;
  ButtonGroupButton day_;
  ButtonGroupButton week_;
  ButtonGroupButton month_;
  ConnectedButtonGroup group_;
  GroupKeys keys_;
};
// Verifies every normalization path, attachment bit 31 and invalid queries.
TEST_F(ConnectedGroupInput, CanonicalMasksAndProjectionRepair) {
  EXPECT_EQ(1u, group_.selectionMask());
  group_.setSelectionMask(0);
  EXPECT_EQ(1u, group_.selectionMask());
  group_.setSelectionRequired(false);
  group_.setSelectionMask(UINT32_MAX);
  EXPECT_EQ(1u, group_.selectionMask());
  group_.setSelectionMode(ConnectedButtonGroupSelectionMode::kMultiple);
  group_.setSelectionMask(UINT32_MAX);
  EXPECT_EQ(7u, group_.selectionMask());
  EXPECT_EQ(-1, group_.selectedIndex());
  group_.setSelectionMode(ConnectedButtonGroupSelectionMode::kSingle);
  EXPECT_EQ(1u, group_.selectionMask());
  group_.setSelectionMask(0);
  EXPECT_EQ(-1, group_.selectedIndex());
  group_.setSelectionRequired(true);
  EXPECT_EQ(0, group_.selectedIndex());
  group_.setSelectionMask(2);
  static_cast<Widget&>(day_).setSelected(true);
  static_cast<Widget&>(week_).setSelected(false);
  EXPECT_FALSE(day_.isSelected());
  EXPECT_TRUE(week_.isSelected());
  EXPECT_EQ(2u, group_.selectionMask());
  group_.clear();
  for (int i = 0; i < 32; ++i)
    ASSERT_TRUE(
        group_.add(std::make_unique<ButtonGroupButton>(app_.context(), "A")));
  group_.setSelectionMask(uint32_t{1} << 31);
  EXPECT_EQ(31, group_.selectedIndex());
  EXPECT_TRUE(group_.isSelected(31));
  EXPECT_FALSE(group_.isSelected(-1));
  EXPECT_FALSE(group_.isSelected(32));
  group_.setSelectionMode(ConnectedButtonGroupSelectionMode::kMultiple);
  group_.setSelectionMask(UINT32_MAX);
  EXPECT_EQ(UINT32_MAX, group_.selectionMask());
  group_.clear();
  EXPECT_EQ(0u, group_.selectionMask());
}
// Verifies only the group emits changed selection and configuration stays
// silent.
TEST_F(ConnectedGroupInput, InteractiveSingleAndMultiple) {
  int changes = 0;
  int child_changes = 0;
  group_.setOnInteractiveChange([&] {
    ++changes;
    EXPECT_EQ(group_.isSelected(0), day_.isSelected());
    EXPECT_EQ(group_.isSelected(1), week_.isSelected());
  });
  day_.setOnInteractiveChange([&] { ++child_changes; });
  group_.setSelectionMask(2);
  EXPECT_EQ(0, changes);
  Rect day_frame = day_.parent_bounds();
  day_.onClicked();
  EXPECT_EQ(1u, group_.selectionMask());
  EXPECT_EQ(1, changes);
  day_.onClicked();
  EXPECT_EQ(1, changes);
  EXPECT_EQ(day_frame, day_.parent_bounds());
  group_.setSelectionRequired(false);
  day_.onClicked();
  EXPECT_EQ(0u, group_.selectionMask());
  EXPECT_EQ(2, changes);
  group_.setSelectionMode(ConnectedButtonGroupSelectionMode::kMultiple);
  day_.onClicked();
  week_.onClicked();
  EXPECT_EQ(3u, group_.selectionMask());
  group_.setSelectionRequired(true);
  day_.onClicked();
  week_.onClicked();
  EXPECT_EQ(2u, group_.selectionMask());
  EXPECT_EQ(0, child_changes);
}
// Verifies selection persists for disabled/invisible/gone members, and delayed
// actions recheck all ancestors before accepting activation.
TEST_F(ConnectedGroupInput, VisibilityEnablementAndDetachedEntries) {
  group_.setSelectionMask(2);
  week_.setEnabled(false);
  EXPECT_EQ(2u, group_.selectionMask());
  week_.onClicked();
  EXPECT_EQ(2u, group_.selectionMask());
  week_.setVisibility(Visibility::kInvisible);
  EXPECT_EQ(2u, group_.selectionMask());
  week_.setVisibility(Visibility::kGone);
  EXPECT_EQ(2u, group_.selectionMask());
  group_.setSelectionMask(4);
  group_.setEnabled(false);
  EXPECT_TRUE(day_.isEnabled());
  EXPECT_FALSE(week_.isEnabled());
  day_.onClicked();
  EXPECT_EQ(4u, group_.selectionMask());
  group_.setEnabled(true);
  EXPECT_FALSE(week_.isEnabled());
  panel_.setEnabled(false);
  day_.onClicked();
  EXPECT_EQ(4u, group_.selectionMask());
  panel_.setEnabled(true);
  group_.clear();
  EXPECT_FALSE(day_.isClickable());
  EXPECT_FALSE(day_.isSelected());
  panel_.add(WidgetRef(day_), Rect(20, 80, 99, 119));
  EXPECT_FALSE(day_.isClickable());
  day_.onClicked();
  EXPECT_EQ(0u, group_.selectionMask());
}
// Verifies task key delivery, focus-only navigation, RTL and edge consumption.
TEST_F(ConnectedGroupInput, PhysicalKeysAndNavigation) {
  ASSERT_TRUE(day_.requestFocus());
  key(KeyCode::kRight);
  EXPECT_TRUE(week_.isFocused());
  EXPECT_EQ(1u, group_.selectionMask());
  key(KeyCode::kEnter);
  key(KeyCode::kEnter, KeyPhase::kUp);
  EXPECT_EQ(2u, group_.selectionMask());
  key(KeyCode::kEnd);
  EXPECT_TRUE(month_.isFocused());
  key(KeyCode::kRight);
  EXPECT_TRUE(month_.isFocused());
  key(KeyCode::kHome);
  EXPECT_TRUE(day_.isFocused());
  system_time_delay_micros(kPressAnimationMillis * 1000 + 1);
  app_.refresh();
  key(KeyCode::kSpace);
  key(KeyCode::kSpace, KeyPhase::kUp);
  EXPECT_EQ(1u, group_.selectionMask());
  group_.setLayoutDirection(LayoutDirection::kRightToLeft);
  app_.refresh();
  key(KeyCode::kEnd);
  key(KeyCode::kRight);
  EXPECT_TRUE(week_.isFocused());
  week_.setEnabled(false);
  ASSERT_TRUE(day_.requestFocus());
  key(KeyCode::kLeft);
  EXPECT_TRUE(month_.isFocused());
  EXPECT_FALSE(group_.onKeyEvent({KeyPhase::kUp, KeyCode::kHome, 0, 0}));
  EXPECT_FALSE(group_.onKeyEvent(
      {KeyPhase::kDown, KeyCode::kHome, kKeyModifierShift, 0}));
  EXPECT_FALSE(group_.onKeyEvent({KeyPhase::kDown, KeyCode::kUp, 0, 0}));
}
// Verifies real touch activation and cancellation when group disable interrupts
// pointer or physical-key presses without overwriting child flags.
TEST_F(ConnectedGroupInput, PointerAndKeyDisableCancel) {
  tap(week_);
  EXPECT_EQ(2u, group_.selectionMask());
  touch_.set(true, 20 + month_.offsetLeft() + month_.width() / 2, 40);
  pollTouch();
  group_.setEnabled(false);
  group_.setEnabled(true);
  touch_.set(false, 0, 0);
  pollTouch();
  EXPECT_EQ(2u, group_.selectionMask());
  EXPECT_FALSE(month_.isPressed());
  ASSERT_TRUE(day_.requestFocus());
  key(KeyCode::kEnter);
  group_.setEnabled(false);
  group_.setEnabled(true);
  key(KeyCode::kEnter, KeyPhase::kUp);
  EXPECT_EQ(2u, group_.selectionMask());
  EXPECT_FALSE(day_.isPressed());
}
// Verifies callbacks can replace the mask, clear/repopulate, or destroy both
// the group and the currently executing adopted entry, with no later access.
TEST_F(ConnectedGroupInput, ReentrantAndDestructiveCallbacks) {
  group_.setOnInteractiveChange([this] { group_.setSelectionMask(4); });
  week_.onClicked();
  EXPECT_EQ(4u, group_.selectionMask());
  group_.setOnInteractiveChange([this] {
    group_.clear();
    group_.add(month_);
  });
  day_.onClicked();
  EXPECT_EQ(1u, group_.selectionMask());
  EXPECT_EQ(1, group_.entryCount());
  auto owned = std::make_unique<ConnectedButtonGroup>(app_.context());
  owned->add(std::make_unique<ButtonGroupButton>(app_.context(), "A"));
  auto child = std::make_unique<ButtonGroupButton>(app_.context(), "B");
  ButtonGroupButton* raw = child.get();
  owned->add(std::move(child));
  panel_.add(WidgetRef(*owned), Rect(20, 80, 199, 119));
  owned->setOnInteractiveChange([&owned, this] {
    panel_.removeAll();
    owned.reset();
  });
  raw->onClicked();
  EXPECT_EQ(nullptr, owned);
}
static_assert(sizeof(ConnectedButtonGroup) - sizeof(Container) <=
              (sizeof(void*) == 4 ? 24 : 40));
}  // namespace
}  // namespace roo_windows::material3
