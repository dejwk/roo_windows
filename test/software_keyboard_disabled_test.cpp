#include "gtest/gtest.h"
#include "roo_fonts/NotoSans_Regular/16.h"
#include "roo_windows/material2/text_field.h"
#include "roo_windows/material3/text_field/text_field.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {
static_assert(ROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD == 0);
using DisabledKeyboardTest = test_support::RooWindowsRenderTestSized<320, 240>;

// Verifies both systems edit in place, retain their full viewport, and share
// semantic routing without a keyboard task or editor destination.
TEST_F(DisabledKeyboardTest, BothSystemsEditInPlace) {
  material2::TextField first(context(), roo_display::font_NotoSans_Regular_16(),
                             "", roo_display::kLeft,
                             material2::TextField::NONE);
  material3::TextField second(context(), "Name");
  Task& first_task = app_.addTaskFullScreen(first);
  Task& second_task = app_.addTaskFullScreen(second);
  refresh();
  const Rect before = second.bounds();
  TextInputEmitter input;
  input.connect(app_);
  first.edit();
  ASSERT_TRUE(first_task.textFieldEditor().isEdited(&first));
  EXPECT_TRUE(input.commitRune(U'A'));
  second.edit();
  ASSERT_TRUE(second.isEdited());
  EXPECT_FALSE(first_task.textFieldEditor().isEdited(&first));
  EXPECT_TRUE(input.commitRune(U'B'));
  EXPECT_TRUE(second.onKeyEvent(
      {KeyPhase::kDown, KeyCode::kCharacter, 0, PhysicalKey::kC, U'C'}));
  refresh();
  EXPECT_EQ(before, second.bounds());
  EXPECT_EQ("A", first.content());
  EXPECT_EQ("BC", second.text());
  EXPECT_EQ(1u, second_task.navigation().depth());
  second_task.textFieldEditor().cancel();
  EXPECT_FALSE(input.commitRune(U'D'));
  first_task.navigation().clear();
  second_task.navigation().clear();
}

class OneKeySource : public KeySource {
 public:
  int drain(KeyEvent* out, int count) override {
    if (sent_ || count == 0) return 0;
    out[0] = {KeyPhase::kDown, KeyCode::kCharacter, 0, PhysicalKey::kA, U'A'};
    sent_ = true;
    return 1;
  }

 private:
  bool hasPendingEvents() const override { return !sent_; }
  bool sent_ = false;
};

// Verifies the legacy constructor connects hardware input when the first
// application task is added, without relying on a hidden keyboard task.
TEST(DisabledKeyboard, LegacyHardwareSourceConnectsToFirstTask) {
  roo::byte raster[320 * 240 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      320, 240, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  OneKeySource keys;
  Application app(&environment, display, keys, false);
  material3::TextField field(app.context(), "Name");
  Task& task = app.addTaskFullScreen(field);
  app.refresh();
  field.edit();
  ASSERT_TRUE(keys.isConnected());
  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
  EXPECT_EQ("A", field.text());
  task.navigation().clear();
}
}  // namespace
}  // namespace roo_windows
