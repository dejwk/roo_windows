#include "roo_windows/core/navigation_host.h"

#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace test {
class NavigationHostTestAccess {
 public:
  static size_t OverflowCapacity(const NavigationHost& host) {
    return host.history_.capacity();
  }
};
}  // namespace test
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

class LifecycleDestination : public TestDestination {
 public:
  LifecycleDestination(ApplicationContext& context, std::vector<char>& events,
                       char name)
      : TestDestination(context), events_(events), name_(name) {}

  void onStart() override { events_.push_back(name_); }
  void onResume() override {
    EXPECT_NE(nullptr, contents().parent());
    events_.push_back('R');
  }
  void onPause() override {
    EXPECT_NE(nullptr, contents().parent());
    events_.push_back('P');
  }
  void onStop() override {
    EXPECT_EQ(nullptr, contents().parent());
    events_.push_back('S');
  }

 private:
  std::vector<char>& events_;
  char name_;
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

  TestDestination* first = nullptr;
  TestDestination* second = nullptr;
  {
    Application app(&environment, display);
    first = new TestDestination(app.context());
    second = new TestDestination(app.context());
    NavigationHost& navigation = app.addTaskFullScreen().navigation();
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

  TestDestination* first = nullptr;
  TestDestination* second = nullptr;
  bool fallback_called = false;
  {
    Application app(&environment, display);
    first = new TestDestination(app.context());
    second = new TestDestination(app.context());
    Task& task = app.addTaskFullScreen();
    NavigationHost& navigation = task.navigation();
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

  TestDestination* next = nullptr;
  ReentrantDestination* current = nullptr;
  {
    Application app(&environment, display);
    next = new TestDestination(app.context());
    Task& task = app.addTaskFullScreen();
    NavigationHost& navigation = task.navigation();
    current = new ReentrantDestination(app.context(), navigation, *next);
    navigation.push(*current);

    EXPECT_EQ(BackResult::kHandled,
              task.requestBack(BackSource::kProgrammatic));
    EXPECT_EQ(2u, navigation.depth());
  }
  delete current;
  delete next;
}

// Verifies lifecycle order and attachment observations for
// destination push, pop, and teardown.
TEST(NavigationHost, DestinationLifecycleFollowsHistoryAndCurrentContent) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);

  std::vector<char> events;
  LifecycleDestination* first = nullptr;
  LifecycleDestination* second = nullptr;
  {
    Application app(&environment, display);
    first = new LifecycleDestination(app.context(), events, 'A');
    second = new LifecycleDestination(app.context(), events, 'B');
    NavigationHost& navigation = app.addTaskFullScreen().navigation();
    navigation.push(*first);
    navigation.push(*second);
    navigation.pop();
    navigation.clear();
  }
  EXPECT_EQ(
      (std::vector<char>{'A', 'R', 'P', 'B', 'R', 'P', 'S', 'R', 'P', 'S'}),
      events);
  delete second;
  delete first;
}

TEST(NavigationHost, RemovedCallbackFollowsReentrantStop) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);

  Application app_(&environment, display);
  NavigationHost& navigation_ = app_.addTaskFullScreen().navigation();
  TestWidget outgoing_content(app_.context());
  TestDestination next(app_.context());
  class NavigatingOnStop final : public Destination {
   public:
    NavigatingOnStop(Widget& content, Destination& next)
        : content_(content), next_(next) {}
    Widget& getContents() override { return content_; }
    void onStop() override { getNavigationHost()->push(next_); }
    void onRemoved() override {
      EXPECT_EQ(nullptr, getNavigationHost());
      ++removals;
    }
    int removals = 0;

   private:
    Widget& content_;
    Destination& next_;
  } outgoing(outgoing_content, next);
  navigation_.push(outgoing);
  navigation_.pop();
  EXPECT_EQ(1, outgoing.removals);
  EXPECT_EQ(nullptr, outgoing.getNavigationHost());
  EXPECT_NE(nullptr, next.contents().parent());
  navigation_.pop();
}

TEST(NavigationHost, RootUsesInlineStorageAndOverflowCapacityIsRetained) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  NavigationHost& navigation = app.addTaskFullScreen().navigation();
  TestDestination first(app.context());
  TestDestination second(app.context());
  TestDestination third(app.context());
  EXPECT_EQ(0u, test::NavigationHostTestAccess::OverflowCapacity(navigation));
  navigation.push(first);
  EXPECT_EQ(1u, navigation.depth());
  EXPECT_EQ(0u, test::NavigationHostTestAccess::OverflowCapacity(navigation));
  navigation.replace(second);
  EXPECT_EQ(nullptr, first.getNavigationHost());
  EXPECT_EQ(0u, test::NavigationHostTestAccess::OverflowCapacity(navigation));
  navigation.push(third);
  EXPECT_EQ(2u, navigation.depth());
  const size_t capacity =
      test::NavigationHostTestAccess::OverflowCapacity(navigation);
  EXPECT_GE(capacity, 1u);
  navigation.replace(first);
  EXPECT_EQ(capacity,
            test::NavigationHostTestAccess::OverflowCapacity(navigation));
  EXPECT_EQ(nullptr, third.getNavigationHost());
  navigation.pop();
  EXPECT_TRUE(navigation.isCurrent(second));
  navigation.pop();
  EXPECT_TRUE(navigation.empty());
  navigation.push(first);
  EXPECT_EQ(1u, navigation.depth());
  navigation.clear();
  EXPECT_EQ(capacity,
            test::NavigationHostTestAccess::OverflowCapacity(navigation));
}

TEST(NavigationHost, RootCanRedirectDuringStartWithoutAllocatingHistory) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  NavigationHost& navigation = app.addTaskFullScreen().navigation();
  TestDestination next(app.context());
  class RedirectingDestination final : public TestDestination {
   public:
    RedirectingDestination(ApplicationContext& context, Destination& next)
        : TestDestination(context), next_(next) {}
    void onStart() override { getNavigationHost()->replace(next_); }

   private:
    Destination& next_;
  } first(app.context(), next);
  navigation.push(first);
  EXPECT_EQ(nullptr, first.getNavigationHost());
  EXPECT_TRUE(navigation.isCurrent(next));
  EXPECT_EQ(1u, navigation.depth());
  EXPECT_EQ(0u, test::NavigationHostTestAccess::OverflowCapacity(navigation));
  navigation.clear();
}

TEST(NavigationHost, TaskTeardownDrainsHistoryAfterReentrantRemoval) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  auto app = std::make_unique<Application>(&environment, display);
  NavigationHost& navigation = app->addTaskFullScreen().navigation();
  TestDestination root(app->context());
  TestDestination middle(app->context());
  class RemovingDestination final : public TestDestination {
   public:
    RemovingDestination(ApplicationContext& context, NavigationHost& navigation)
        : TestDestination(context), navigation_(navigation) {}
    void onRemoved() override {
      EXPECT_FALSE(navigation_.isAvailable());
      navigation_.pop();
    }

   private:
    NavigationHost& navigation_;
  } top(app->context(), navigation);
  navigation.push(root);
  navigation.push(middle);
  navigation.push(top);
  app.reset();
  EXPECT_EQ(nullptr, top.getNavigationHost());
  EXPECT_EQ(nullptr, middle.getNavigationHost());
  EXPECT_EQ(nullptr, root.getNavigationHost());
  EXPECT_EQ(nullptr, root.contents().parent());
}

}  // namespace
}  // namespace roo_windows
