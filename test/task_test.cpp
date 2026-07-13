#include <memory>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/shape/basic.h"
#include "roo_scheduler.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/task.h"

namespace roo_windows {
namespace {

using roo_display::Argb4444;
using roo_display::Display;

class TestWidget : public BasicWidget {
 public:
  explicit TestWidget(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }
};

class DefaultActivity : public Activity {
 public:
  explicit DefaultActivity(ApplicationContext& context) : contents_(context) {}

  Widget& getContents() override { return contents_; }

 private:
  TestWidget contents_;
};

class TestActivity : public Activity {
 public:
  explicit TestActivity(ApplicationContext& context) : contents_(context) {}

  Widget& getContents() override { return contents_; }

  BackResult onBackRequested(BackSource source) override {
    ++back_request_count;
    last_source = source;
    return result;
  }

  BackResult result = BackResult::kUnhandled;
  int back_request_count = 0;
  BackSource last_source = BackSource::kProgrammatic;

 private:
  TestWidget contents_;
};

class ExitOnBackActivity : public TestActivity {
 public:
  explicit ExitOnBackActivity(ApplicationContext& context)
      : TestActivity(context) {}

  BackResult onBackRequested(BackSource source) override {
    TestActivity::onBackRequested(source);
    exit();
    return BackResult::kUnhandled;
  }
};

class ClearOnBackActivity : public TestActivity {
 public:
  explicit ClearOnBackActivity(ApplicationContext& context)
      : TestActivity(context) {}

  BackResult onBackRequested(BackSource source) override {
    TestActivity::onBackRequested(source);
    getTask()->clear();
    return BackResult::kUnhandled;
  }
};

class PushOnBackActivity : public TestActivity {
 public:
  PushOnBackActivity(ApplicationContext& context, Activity& next)
      : TestActivity(context), next_(next) {}

  BackResult onBackRequested(BackSource source) override {
    TestActivity::onBackRequested(source);
    getTask()->enterActivity(&next_);
    return BackResult::kUnhandled;
  }

 private:
  Activity& next_;
};

// Clears a borrowed activity stack before the activities leave scope.
class TaskStackCleanup {
 public:
  explicit TaskStackCleanup(Task& task) : task_(task) {}
  ~TaskStackCleanup() { task_.clear(); }

 private:
  Task& task_;
};

class TaskTest : public ::testing::Test {
 protected:
  TaskTest()
      : display_(device_),
        app_(&environment_, display_),
        task_(*app_.addTaskFullScreen()) {}

  roo::byte raster_[64 * 64 * 2] = {};
  roo_display::OffscreenDevice<Argb4444> device_{64, 64, raster_, Argb4444()};
  Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_{scheduler_};
  Application app_;
  Task& task_;
};

// Verifies that the default hook leaves a root activity in place.
TEST_F(TaskTest, RequestBackPreservesRootActivity) {
  DefaultActivity root(app_.context());
  TaskStackCleanup cleanup(task_);
  task_.enterActivity(&root);

  EXPECT_EQ(BackResult::kUnhandled, task_.requestBack(BackSource::kBackKey));
  EXPECT_EQ(1u, task_.activityCount());
  EXPECT_EQ(&root, task_.currentActivity());
}

// Verifies that destroying an attached borrowed activity fails in debug builds.
TEST_F(TaskTest, ActivityDestructionRequiresDetachment) {
  std::unique_ptr<DefaultActivity> activity(
      new DefaultActivity(app_.context()));
  TaskStackCleanup cleanup(task_);
  task_.enterActivity(activity.get());

  EXPECT_DEATH({ activity.reset(); }, "");
}

// Verifies that clearing a task detaches every borrowed activity.
TEST_F(TaskTest, ClearDetachesActivities) {
  TestActivity root(app_.context());
  TaskStackCleanup cleanup(task_);
  task_.enterActivity(&root);

  task_.clear();

  EXPECT_EQ(nullptr, root.getTask());
  EXPECT_EQ(0u, task_.activityCount());
}

// Verifies that an unhandled request pops exactly the current non-root
// activity.
TEST_F(TaskTest, RequestBackPopsOneNonRootActivity) {
  TestActivity root(app_.context());
  TestActivity child(app_.context());
  TaskStackCleanup cleanup(task_);
  task_.enterActivity(&root);
  task_.enterActivity(&child);

  EXPECT_EQ(BackResult::kHandled, task_.requestBack());
  EXPECT_EQ(1u, task_.activityCount());
  EXPECT_EQ(&root, task_.currentActivity());
  EXPECT_EQ(1, child.back_request_count);
}

// Verifies that activity-local handling prevents task fallback.
TEST_F(TaskTest, RequestBackLetsActivityConsumeRequest) {
  TestActivity root(app_.context());
  TestActivity child(app_.context());
  TaskStackCleanup cleanup(task_);
  child.result = BackResult::kHandled;
  task_.enterActivity(&root);
  task_.enterActivity(&child);

  EXPECT_EQ(BackResult::kHandled,
            task_.requestBack(BackSource::kNavigationButton));
  EXPECT_EQ(2u, task_.activityCount());
  EXPECT_EQ(&child, task_.currentActivity());
  EXPECT_EQ(1, child.back_request_count);
  EXPECT_EQ(BackSource::kNavigationButton, child.last_source);
}

// Verifies that a callback that pops itself is not followed by another pop.
TEST_F(TaskTest, RequestBackDoesNotDoublePopAfterReentrantExit) {
  TestActivity root(app_.context());
  ExitOnBackActivity child(app_.context());
  TaskStackCleanup cleanup(task_);
  task_.enterActivity(&root);
  task_.enterActivity(&child);

  EXPECT_EQ(BackResult::kHandled, task_.requestBack());
  EXPECT_EQ(1u, task_.activityCount());
  EXPECT_EQ(&root, task_.currentActivity());
}

// Verifies that a callback clearing the stack is handled without a stale pop.
TEST_F(TaskTest, RequestBackHandlesReentrantClear) {
  ClearOnBackActivity root(app_.context());
  TaskStackCleanup cleanup(task_);
  task_.enterActivity(&root);

  EXPECT_EQ(BackResult::kHandled, task_.requestBack());
  EXPECT_EQ(0u, task_.activityCount());
  EXPECT_EQ(nullptr, task_.currentActivity());
}

// Verifies that a callback pushing an activity is handled without popping it.
TEST_F(TaskTest, RequestBackHandlesReentrantPush) {
  TestActivity pushed(app_.context());
  PushOnBackActivity root(app_.context(), pushed);
  TaskStackCleanup cleanup(task_);
  task_.enterActivity(&root);

  EXPECT_EQ(BackResult::kHandled, task_.requestBack());
  EXPECT_EQ(2u, task_.activityCount());
  EXPECT_EQ(&pushed, task_.currentActivity());
}

}  // namespace
}  // namespace roo_windows
