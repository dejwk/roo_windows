#include <memory>
#include <vector>

#include "examples/material3/settings_shell/settings_shell.h"
#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"

namespace roo_windows::material3::examples {
namespace {
class Keys : public KeySource {
 public:
  void send(KeyCode code, PhysicalKey physical) {
    events_.push_back({KeyPhase::kDown, code, 0, physical, 0});
    events_.push_back({KeyPhase::kUp, code, 0, physical, 0});
    notifyReady();
  }
  int drain(KeyEvent* output, int capacity) override {
    int n = std::min<int>(capacity, events_.size());
    std::copy_n(events_.begin(), n, output);
    events_.erase(events_.begin(), events_.begin() + n);
    return n;
  }

 private:
  bool hasPendingEvents() const override { return !events_.empty(); }
  std::vector<KeyEvent> events_;
};
class SettingsShellTest : public testing::Test {
 protected:
  SettingsShellTest()
      : device_(240, 320, raster_, roo_display::Argb4444()),
        display_(device_),
        env_(scheduler_),
        app_(new Application(&env_, display_)),
        shell_(app_->context()),
        task_(app_->addTaskFullScreen(shell_)) {
    keys_.connect(task_);
    shell_.snackbars().setAnimationsEnabled(false);
    app_->refresh();
    app_->start();
  }
  ~SettingsShellTest() override { app_.reset(); }
  void key(KeyCode code, PhysicalKey physical) {
    keys_.send(code, physical);
    scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum,
                                           4);
    app_->refresh();
  }
  // Use the same callback-free geometric target selection as touch dispatch.
  void click(Widget& widget) {
    int x = widget.bounds().width() / 2;
    int y = widget.bounds().height() / 2;
    for (Widget* p = &widget; p->parent() != nullptr; p = p->parent()) {
      x += p->offsetLeft();
      y += p->offsetTop();
    }
    std::vector<Widget*> path;
    ASSERT_TRUE(app_->root().fillTouchTargetPath(x, y, path));
    ASSERT_FALSE(path.empty());
    EXPECT_EQ(&widget, path.back());
    Widget& target = *path.back();
    target.onDown(target.width() / 2, target.height() / 2);
    target.onSingleTapUp(target.width() / 2, target.height() / 2);
    delay(500);
    app_->refresh();
    scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum,
                                           4);
    app_->refresh();
  }
  roo::byte raster_[240 * 320 * 2]{};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
  std::unique_ptr<Application> app_;
  SettingsShell shell_;
  Task& task_;
  Keys keys_;
};

// Verifies menu and confirmation closure restores base focus and reports
// feedback.
TEST_F(SettingsShellTest, MenuDialogEscapeAndFeedback) {
  ASSERT_TRUE(task_.focus().requestFocus(shell_.modeButton()));
  click(shell_.modeButton());
  EXPECT_TRUE(
      app_->root().transient_presentation_slot().hasActivePresentation());
  key(KeyCode::kEscape, PhysicalKey::kEscape);
  EXPECT_FALSE(
      app_->root().transient_presentation_slot().hasActivePresentation());
  EXPECT_EQ(&shell_.modeButton(), task_.focus().focused());
  click(shell_.resetButton());
  EXPECT_TRUE(shell_.confirmation().isShowing());
  key(KeyCode::kEscape, PhysicalKey::kEscape);
  EXPECT_FALSE(shell_.confirmation().isShowing());
  EXPECT_TRUE(shell_.snackbars().isShowing());
  EXPECT_EQ(1u, task_.navigation().depth());
  const auto capture =
      ::roo_windows::test::CaptureRgb(device_.raster(), 0, 0, 240, 320);
  EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
      capture, "test/goldens/material3_settings_shell/cancel_feedback.ppm",
      "settings_cancel_feedback"));
}

// Verifies bottom navigation, full-screen destination Back, and ordinary Tab
// flow.
TEST_F(SettingsShellTest, MultipleScreensAndNavigationBack) {
  ASSERT_TRUE(task_.focus().requestFocus(shell_.modeButton()));
  key(KeyCode::kTab, PhysicalKey::kTab);
  EXPECT_NE(&shell_.modeButton(), task_.focus().focused());
  // A navigation destination invokes the same public bar path as pointer input.
  std::vector<Widget*> path;
  const Rect bar = shell_.bottomBarBounds();
  ASSERT_TRUE(shell_.fillTouchTargetPath(bar.xMax() - 10,
                                         bar.yMin() + bar.height() / 2, path));
  ASSERT_FALSE(path.empty());
  path.back()->onClicked();
  app_->refresh();
  EXPECT_EQ(1, shell_.navigationBar().selectedIndex());
  ASSERT_TRUE(shell_.detailsButton().isVisible());
  click(shell_.detailsButton());
  EXPECT_EQ(2u, task_.navigation().depth());
  EXPECT_TRUE(shell_.editor().isCurrent());
  key(KeyCode::kEscape, PhysicalKey::kEscape);
  EXPECT_FALSE(shell_.editor().isShowing());
  EXPECT_EQ(1u, task_.navigation().depth());
  EXPECT_NE(nullptr, shell_.parent());
}

// Verifies real Enter activation confirms the dialog and posts reset feedback.
TEST_F(SettingsShellTest, KeyboardConfirmsReset) {
  click(shell_.resetButton());
  key(KeyCode::kTab, PhysicalKey::kTab);
  key(KeyCode::kTab, PhysicalKey::kTab);
  key(KeyCode::kEnter, PhysicalKey::kEnter);
  delay(500);
  app_->refresh();
  scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 4);
  app_->refresh();
  EXPECT_FALSE(shell_.confirmation().isShowing());
  EXPECT_TRUE(shell_.snackbars().isShowing());
  EXPECT_EQ(1u, task_.navigation().depth());
  const auto capture =
      ::roo_windows::test::CaptureRgb(device_.raster(), 0, 0, 240, 320);
  EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
      capture, "test/goldens/material3_settings_shell/confirmed_feedback.ppm",
      "settings_confirmed_feedback"));
}
}  // namespace
}  // namespace roo_windows::material3::examples
