#include "roo_windows/core/task.h"

#include <memory>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/scrollable_panel.h"
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

class EmptyKeySource : public KeySource {
 public:
  int drain(KeyEvent* out, int max_events) override { return 0; }

 private:
  bool hasPendingEvents() const override { return false; }
};

class DirectWidget : public BasicWidget {
 public:
  explicit DirectWidget(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(4, 4);
  }
};

class DestructionTrackingWidget : public FocusableWidget {
 public:
  DestructionTrackingWidget(ApplicationContext& context, bool& destroyed)
      : FocusableWidget(context), destroyed_(destroyed) {}

  ~DestructionTrackingWidget() override { destroyed_ = true; }

 private:
  bool& destroyed_;
};

// Verifies that a direct task borrows one fixed root and routes Back through
// its task-local callback without constructing an activity stack.
TEST(Task, DirectContentIsBorrowedAndHandlesBack) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  bool called = false;
  DirectWidget* contents = nullptr;
  {
    Application app(&environment, display);
    contents = new DirectWidget(app.context());
    contents->setOnInteractiveChange([]() {});
    Task& task = app.addTaskFullScreen(*contents);
    EXPECT_EQ(&task, contents->getTask());
    ASSERT_TRUE(app.refresh());
    ASSERT_TRUE(contents->requestFocus());
    EXPECT_TRUE(contents->isFocused());
    task.setBackCallback([&called](BackSource source) {
      called = source == BackSource::kBackKey;
      return BackResult::kHandled;
    });
    EXPECT_EQ(BackResult::kHandled, task.requestBack(BackSource::kBackKey));
    EXPECT_TRUE(called);
  }
  EXPECT_EQ(nullptr, contents->parent());
  EXPECT_FALSE(contents->isFocused());
  delete contents;
}

// Verifies that an application may release a caller-owned composite root
// before the caller destroys its owned descendants. The dead context is not
// consulted while the composite tears down its child links.
TEST(Task, CompositeContentMayBeDestroyedAfterApplication) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  bool child_destroyed = false;
  SimpleScrollablePanel* contents = nullptr;
  DestructionTrackingWidget* child = nullptr;
  {
    Application app(&environment, display);
    auto owned_child = std::make_unique<DestructionTrackingWidget>(
        app.context(), child_destroyed);
    child = owned_child.get();
    child->setOnInteractiveChange([]() {});
    contents =
        new SimpleScrollablePanel(app.context(), std::move(owned_child));
    app.addTaskFullScreen(*contents);
    ASSERT_TRUE(app.refresh());
    ASSERT_TRUE(child->requestFocus());
  }

  EXPECT_EQ(nullptr, contents->parent());
  EXPECT_FALSE(child->isFocused());
  delete contents;
  EXPECT_TRUE(child_destroyed);
}

// Verifies that independently attached UI tasks retain independent focus.
TEST(Task, FocusDoesNotCrossTaskBoundaries) {
  roo::byte raster[32 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  FocusableWidget first_contents(app.context());
  FocusableWidget second_contents(app.context());
  Task& first = app.addTaskFullScreen(first_contents);
  Task& second = app.addTaskFullScreen(second_contents);
  app.refresh();

  ASSERT_TRUE(first_contents.requestFocus());
  ASSERT_TRUE(second_contents.requestFocus());

  EXPECT_EQ(&first_contents, first.focus().focused());
  EXPECT_EQ(&second_contents, second.focus().focused());
}

// Verifies the one-source-per-task route contract and explicit disconnection.
TEST(Task, KeySourceConnectionIsExclusive) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  DirectWidget first_contents(app.context());
  DirectWidget second_contents(app.context());
  Task& first = app.addTaskFullScreen(first_contents);
  Task& second = app.addTaskFullScreen(second_contents);
  EmptyKeySource first_source;
  EmptyKeySource second_source;

  first_source.connect(first);
  EXPECT_TRUE(first_source.isConnected());
  EXPECT_DEATH_IF_SUPPORTED(first_source.connect(second), "");
  EXPECT_DEATH_IF_SUPPORTED(second_source.connect(first), "");

  first_source.disconnect();
  second_source.connect(first);
  EXPECT_TRUE(second_source.isConnected());
}

}  // namespace
}  // namespace roo_windows
