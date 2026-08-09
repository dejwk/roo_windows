#include "roo_windows/core/ui_task.h"

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
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
};

class DirectWidget : public BasicWidget {
 public:
  explicit DirectWidget(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(4, 4);
  }
};

// Verifies that a direct task borrows one fixed root and routes Back through
// its task-local callback without constructing an activity stack.
TEST(UiTask, DirectContentIsBorrowedAndHandlesBack) {
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
    UiTask& task = app.addUiTaskFullScreen(*contents);
    EXPECT_EQ(&task, contents->getUiTask());
    task.setBackCallback([&called](BackSource source) {
      called = source == BackSource::kBackKey;
      return BackResult::kHandled;
    });
    EXPECT_EQ(BackResult::kHandled, task.requestBack(BackSource::kBackKey));
    EXPECT_TRUE(called);
  }
  EXPECT_EQ(nullptr, contents->parent());
  delete contents;
}

// Verifies that independently attached UI tasks retain independent focus.
TEST(UiTask, FocusDoesNotCrossTaskBoundaries) {
  roo::byte raster[32 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  FocusableWidget first_contents(app.context());
  FocusableWidget second_contents(app.context());
  UiTask& first = app.addUiTaskFullScreen(first_contents);
  UiTask& second = app.addUiTaskFullScreen(second_contents);
  app.refresh();

  ASSERT_TRUE(first_contents.requestFocus());
  ASSERT_TRUE(second_contents.requestFocus());

  EXPECT_EQ(&first_contents, first.focus().focused());
  EXPECT_EQ(&second_contents, second.focus().focused());
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
  DirectWidget first_contents(app.context());
  DirectWidget second_contents(app.context());
  UiTask& first = app.addUiTaskFullScreen(first_contents);
  UiTask& second = app.addUiTaskFullScreen(second_contents);
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
