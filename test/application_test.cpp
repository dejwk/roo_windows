#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/task.h"
#include "roo_windows/core/transient_presentation.h"

namespace roo_windows {
namespace {

class TestWidget : public BasicWidget {
 public:
  explicit TestWidget(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }
};

class TestActivity : public Activity {
 public:
  explicit TestActivity(ApplicationContext& context) : contents_(context) {}

  Widget& getContents() override { return contents_; }

  BackResult onBackRequested(BackSource source) override {
    ++back_request_count;
    last_source = source;
    return BackResult::kUnhandled;
  }

  int back_request_count = 0;
  BackSource last_source = BackSource::kProgrammatic;

 private:
  TestWidget contents_;
};

class BackPresentation final : public TransientPresentationRegistration {
 public:
  int back_request_count = 0;
  BackSource last_source = BackSource::kProgrammatic;

 protected:
  void detachPresentation(PresentationFinishReason reason) override {}

  BackResult onBackRequested(BackSource source) override {
    ++back_request_count;
    last_source = source;
    return TransientPresentationRegistration::onBackRequested(source);
  }
};

// Verifies that application back dispatch only changes the explicitly targeted
// task, even when another task has a different current activity.
TEST(Application, RequestBackUsesExplicitTargetTask) {
  roo::byte raster[64 * 64 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      64, 64, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  Task* first_task = app.addTaskFullScreen();
  Task* second_task = app.addTaskFullScreen();
  TestActivity first_root(app.context());
  TestActivity first_child(app.context());
  TestActivity second_root(app.context());
  TestActivity second_child(app.context());
  first_task->enterActivity(&first_root);
  first_task->enterActivity(&first_child);
  second_task->enterActivity(&second_root);
  second_task->enterActivity(&second_child);

  EXPECT_EQ(BackResult::kHandled,
            app.requestBack(*second_task, BackSource::kEscapeKey));
  EXPECT_EQ(&first_child, first_task->currentActivity());
  EXPECT_EQ(2u, first_task->activityCount());
  EXPECT_EQ(&second_root, second_task->currentActivity());
  EXPECT_EQ(1u, second_task->activityCount());
  EXPECT_EQ(0, first_child.back_request_count);
  EXPECT_EQ(1, second_child.back_request_count);
  EXPECT_EQ(BackSource::kEscapeKey, second_child.last_source);

  first_task->clear();
  second_task->clear();
}

// Verifies an eligible window presentation receives Back before its task.
TEST(Application, RequestBackPrioritizesTransientPresentation) {
  roo::byte raster[64 * 64 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      64, 64, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  Task* task = app.addTaskFullScreen();
  TestActivity root(app.context());
  TestActivity child(app.context());
  task->enterActivity(&root);
  task->enterActivity(&child);
  BackPresentation presentation;

  EXPECT_EQ(PresentationStartResult::kStarted,
            app.root().transient_presentation_slot().show(
                presentation, TransientPresentationPolicy(true)));
  EXPECT_EQ(BackResult::kHandled,
            app.requestBack(*task, BackSource::kNavigationButton));
  EXPECT_EQ(1, presentation.back_request_count);
  EXPECT_EQ(BackSource::kNavigationButton, presentation.last_source);
  EXPECT_EQ(&child, task->currentActivity());
  EXPECT_EQ(0, child.back_request_count);

  task->clear();
}

}  // namespace
}  // namespace roo_windows
