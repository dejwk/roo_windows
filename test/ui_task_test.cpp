#include "roo_windows/core/ui_task.h"

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

class FocusableWidget : public BasicWidget {
 public:
  explicit FocusableWidget(ApplicationContext& context)
      : BasicWidget(context) {}

  /// Reports a stable test size so Application refresh attaches layout bounds.
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(4, 4);
  }

  bool isFocusable() const override { return true; }
};

class FocusableActivity : public Activity {
 public:
  explicit FocusableActivity(ApplicationContext& context)
      : contents_(context) {}

  Widget& getContents() override { return contents_; }

  FocusableWidget& contents() { return contents_; }

 private:
  FocusableWidget contents_;
};

class EmptyKeySource : public KeySource {
 public:
  int drain(KeyEvent* out, int max_events) override { return 0; }
};

// Verifies that independently attached UI tasks retain independent focus.
TEST(UiTask, FocusDoesNotCrossTaskBoundaries) {
  roo::byte raster[32 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  UiTask& first = app.addUiTaskFullScreen();
  UiTask& second = app.addUiTaskFullScreen();
  FocusableActivity first_activity(app.context());
  FocusableActivity second_activity(app.context());
  first.legacyActivities().enterActivity(&first_activity);
  second.legacyActivities().enterActivity(&second_activity);
  app.refresh();

  ASSERT_TRUE(first_activity.contents().requestFocus());
  ASSERT_TRUE(second_activity.contents().requestFocus());

  EXPECT_EQ(&first_activity.contents(), first.focus().focused());
  EXPECT_EQ(&second_activity.contents(), second.focus().focused());
  first.legacyActivities().clear();
  second.legacyActivities().clear();
}

// Verifies the one-source attachment contract and explicit detachment.
TEST(UiTask, KeySourceAttachmentIsExclusive) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  UiTask& first = app.addUiTaskFullScreen();
  UiTask& second = app.addUiTaskFullScreen();
  EmptyKeySource first_source;
  EmptyKeySource second_source;

  EXPECT_EQ(KeySourceAttachmentResult::kAttached,
            first.attachKeySource(first_source));
  EXPECT_EQ(KeySourceAttachmentResult::kSourceAlreadyAttached,
            second.attachKeySource(first_source));
  EXPECT_EQ(KeySourceAttachmentResult::kTaskAlreadyHasSource,
            first.attachKeySource(second_source));

  first.detachKeySource();
  EXPECT_EQ(KeySourceAttachmentResult::kAttached,
            second.attachKeySource(first_source));
}

}  // namespace
}  // namespace roo_windows
