#include <memory>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/shape/basic.h"
#include "roo_scheduler.h"
#include "roo_windows/core/destination.h"
#include "roo_windows/core/navigation_host.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

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

class DefaultDestination : public Destination {
 public:
  explicit DefaultDestination(ApplicationContext& context) : contents_(context) {}

  Widget& getContents() override { return contents_; }

 private:
  TestWidget contents_;
};

class TestDestination : public Destination {
 public:
  explicit TestDestination(ApplicationContext& context) : contents_(context) {}

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

class ExitOnBackDestination : public TestDestination {
 public:
  explicit ExitOnBackDestination(ApplicationContext& context)
      : TestDestination(context) {}

  BackResult onBackRequested(BackSource source) override {
    TestDestination::onBackRequested(source);
    exit();
    return BackResult::kUnhandled;
  }
};

class ClearOnBackDestination : public TestDestination {
 public:
  explicit ClearOnBackDestination(ApplicationContext& context)
      : TestDestination(context) {}

  BackResult onBackRequested(BackSource source) override {
    TestDestination::onBackRequested(source);
    getNavigationHost()->clear();
    return BackResult::kUnhandled;
  }
};

class PushOnBackDestination : public TestDestination {
 public:
  PushOnBackDestination(ApplicationContext& context, Destination& next)
      : TestDestination(context), next_(next) {}

  BackResult onBackRequested(BackSource source) override {
    TestDestination::onBackRequested(source);
    getNavigationHost()->push(next_);
    return BackResult::kUnhandled;
  }

 private:
  Destination& next_;
};

// Clears borrowed destinations before they leave scope.
class NavigationCleanup {
 public:
  explicit NavigationCleanup(NavigationHost& navigation) : navigation_(navigation) {}
  ~NavigationCleanup() { navigation_.clear(); }

 private:
  NavigationHost& navigation_;
};

class NavigationTaskTest : public ::testing::Test {
 protected:
  NavigationTaskTest()
      : display_(device_),
        app_(&environment_, display_),
        task_(app_.addTaskFullScreen(navigation_)) {}

  roo::byte raster_[64 * 64 * 2] = {};
  roo_display::OffscreenDevice<Argb4444> device_{64, 64, raster_, Argb4444()};
  Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_{scheduler_};
  NavigationHost navigation_;
  Application app_;
  Task& task_;
};

// Verifies that the default hook leaves a root activity in place.
TEST_F(NavigationTaskTest, RequestBackPreservesRootDestination) {
  DefaultDestination root(app_.context());
  NavigationCleanup cleanup(navigation_);
  navigation_.push(root);

  EXPECT_EQ(BackResult::kUnhandled, task_.requestBack(BackSource::kBackKey));
  EXPECT_EQ(1u, navigation_.depth());
}

// Verifies that destroying an attached borrowed activity fails in debug builds.
TEST_F(NavigationTaskTest, DestinationDestructionRequiresDetachment) {
  std::unique_ptr<DefaultDestination> destination(
      new DefaultDestination(app_.context()));
  NavigationCleanup cleanup(navigation_);
  navigation_.push(*destination);

  EXPECT_DEATH({ destination.reset(); }, "");
}

// Verifies that clearing a task detaches every borrowed activity.
TEST_F(NavigationTaskTest, ClearDetachesDestinations) {
  TestDestination root(app_.context());
  NavigationCleanup cleanup(navigation_);
  navigation_.push(root);

  navigation_.clear();

  EXPECT_EQ(nullptr, root.getTask());
  EXPECT_EQ(0u, navigation_.depth());
}

// Verifies that an unhandled request pops exactly the current non-root
// activity.
TEST_F(NavigationTaskTest, RequestBackPopsOneNonRootDestination) {
  TestDestination root(app_.context());
  TestDestination child(app_.context());
  NavigationCleanup cleanup(navigation_);
  navigation_.push(root);
  navigation_.push(child);

  EXPECT_EQ(BackResult::kHandled, task_.requestBack());
  EXPECT_EQ(1u, navigation_.depth());
  EXPECT_EQ(1, child.back_request_count);
}

// Verifies that activity-local handling prevents task fallback.
TEST_F(NavigationTaskTest, RequestBackLetsDestinationConsumeRequest) {
  TestDestination root(app_.context());
  TestDestination child(app_.context());
  NavigationCleanup cleanup(navigation_);
  child.result = BackResult::kHandled;
  navigation_.push(root);
  navigation_.push(child);

  EXPECT_EQ(BackResult::kHandled,
            task_.requestBack(BackSource::kNavigationButton));
  EXPECT_EQ(2u, navigation_.depth());
  EXPECT_EQ(1, child.back_request_count);
  EXPECT_EQ(BackSource::kNavigationButton, child.last_source);
}

// Verifies that a callback that pops itself is not followed by another pop.
TEST_F(NavigationTaskTest, RequestBackDoesNotDoublePopAfterReentrantExit) {
  TestDestination root(app_.context());
  ExitOnBackDestination child(app_.context());
  NavigationCleanup cleanup(navigation_);
  navigation_.push(root);
  navigation_.push(child);

  EXPECT_EQ(BackResult::kHandled, task_.requestBack());
  EXPECT_EQ(1u, navigation_.depth());
}

// Verifies that a callback clearing the stack is handled without a stale pop.
TEST_F(NavigationTaskTest, RequestBackHandlesReentrantClear) {
  ClearOnBackDestination root(app_.context());
  NavigationCleanup cleanup(navigation_);
  navigation_.push(root);

  EXPECT_EQ(BackResult::kHandled, task_.requestBack());
  EXPECT_EQ(0u, navigation_.depth());
}

// Verifies that a callback pushing an activity is handled without popping it.
TEST_F(NavigationTaskTest, RequestBackHandlesReentrantPush) {
  TestDestination pushed(app_.context());
  PushOnBackDestination root(app_.context(), pushed);
  NavigationCleanup cleanup(navigation_);
  navigation_.push(root);

  EXPECT_EQ(BackResult::kHandled, task_.requestBack());
  EXPECT_EQ(2u, navigation_.depth());
}

}  // namespace
}  // namespace roo_windows
