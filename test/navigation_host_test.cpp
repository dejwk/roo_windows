#include "roo_windows/core/navigation_host.h"

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

class TestWidget : public BasicWidget {
 public:
  explicit TestWidget(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(4, 4);
  }
};

class TestDestination : public Destination {
 public:
  explicit TestDestination(ApplicationContext& context) : contents_(context) {}

  Widget& getContents() override { return contents_; }
  TestWidget& contents() { return contents_; }

 private:
  TestWidget contents_;
};

class ReentrantDestination : public TestDestination {
 public:
  ReentrantDestination(ApplicationContext& context, NavigationHost& navigation,
                       Destination& next)
      : TestDestination(context), navigation_(navigation), next_(next) {}

  BackResult onBackRequested(BackSource) override {
    navigation_.push(next_);
    return BackResult::kUnhandled;
  }

 private:
  NavigationHost& navigation_;
  Destination& next_;
};

// Verifies that history borrows destinations, attaching only the current root
// and restoring the previous root after a pop.
TEST(NavigationHost, PushPopAndClearBorrowDestinationContents) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  NavigationHost navigation;
  TestDestination* first = nullptr;
  TestDestination* second = nullptr;
  {
    Application app(&environment, display);
    first = new TestDestination(app.context());
    second = new TestDestination(app.context());
    app.addUiTaskFullScreen(navigation);
    navigation.push(*first);
    EXPECT_EQ(1u, navigation.depth());
    EXPECT_NE(nullptr, first->contents().parent());
    navigation.push(*second);
    EXPECT_EQ(2u, navigation.depth());
    EXPECT_EQ(nullptr, first->contents().parent());
    EXPECT_NE(nullptr, second->contents().parent());
    navigation.pop();
    EXPECT_EQ(1u, navigation.depth());
    EXPECT_NE(nullptr, first->contents().parent());
    EXPECT_EQ(nullptr, second->contents().parent());
    navigation.clear();
    EXPECT_TRUE(navigation.empty());
    EXPECT_EQ(nullptr, first->contents().parent());
  }
  delete second;
  delete first;
}

// Verifies that destination Back gets first refusal, stack traversal pops only
// above root, and the task callback handles the exhausted stack.
TEST(NavigationHost, BackRoutesDestinationThenHistoryThenTask) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  NavigationHost navigation;
  TestDestination* first = nullptr;
  TestDestination* second = nullptr;
  bool fallback_called = false;
  {
    Application app(&environment, display);
    first = new TestDestination(app.context());
    second = new TestDestination(app.context());
    UiTask& task = app.addUiTaskFullScreen(navigation);
    task.setBackCallback([&fallback_called](BackSource source) {
      fallback_called = source == BackSource::kNavigationButton;
      return BackResult::kHandled;
    });
    navigation.push(*first);
    navigation.push(*second);

    EXPECT_EQ(BackResult::kHandled,
              task.requestBack(BackSource::kNavigationButton));
    EXPECT_EQ(1u, navigation.depth());
    EXPECT_FALSE(fallback_called);
    EXPECT_EQ(BackResult::kHandled,
              task.requestBack(BackSource::kNavigationButton));
    EXPECT_TRUE(fallback_called);
    EXPECT_EQ(1u, navigation.depth());
  }
  delete second;
  delete first;
}

// Verifies that navigation performed from destination Back consumes the one
// semantic request rather than allowing a second fallback pop.
TEST(NavigationHost, ReentrantDestinationBackPerformsOnlyOneStep) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  NavigationHost navigation;
  TestDestination* next = nullptr;
  ReentrantDestination* current = nullptr;
  {
    Application app(&environment, display);
    next = new TestDestination(app.context());
    UiTask& task = app.addUiTaskFullScreen(navigation);
    current = new ReentrantDestination(app.context(), navigation, *next);
    navigation.push(*current);

    EXPECT_EQ(BackResult::kHandled,
              task.requestBack(BackSource::kProgrammatic));
    EXPECT_EQ(2u, navigation.depth());
  }
  delete current;
  delete next;
}

}  // namespace
}  // namespace roo_windows
