#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/transient_presentation.h"
#include "roo_windows/material3/tabs/tabs.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows {
namespace {

class QueuedKeySource : public KeySource {
 public:
  explicit QueuedKeySource(std::vector<KeyEvent> events)
      : events_(std::move(events)), next_(0), drain_calls_(0) {}

  int drain(KeyEvent* out, int max_events) override {
    ++drain_calls_;
    max_events_.push_back(max_events);
    int count = 0;
    while (count < max_events && next_ < events_.size()) {
      out[count++] = events_[next_++];
    }
    return count;
  }

  int drain_calls() const { return drain_calls_; }
  const std::vector<int>& max_events() const { return max_events_; }
  size_t remaining() const { return events_.size() - next_; }

 private:
  std::vector<KeyEvent> events_;
  size_t next_;
  int drain_calls_;
  std::vector<int> max_events_;
};

class FocusableBackWidget : public BasicWidget {
 public:
  explicit FocusableBackWidget(ApplicationContext& context)
      : BasicWidget(context) {}

  bool isFocusable() const override { return true; }
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }
};

class BackActivity : public Activity {
 public:
  explicit BackActivity(ApplicationContext& context) : contents_(context) {}

  Widget& getContents() override { return contents_; }
  BackResult onBackRequested(BackSource source) override {
    ++request_count;
    last_source = source;
    return BackResult::kUnhandled;
  }

  FocusableBackWidget& contents() { return contents_; }

  int request_count = 0;
  BackSource last_source = BackSource::kProgrammatic;

 private:
  FocusableBackWidget contents_;
};

class BackPresentation final : public TransientPresentationRegistration {
 public:
  PresentationFinishReason reason = PresentationFinishReason::kAction;
  int detach_count = 0;

 protected:
  void detachPresentation(PresentationFinishReason finish_reason) override {
    reason = finish_reason;
    ++detach_count;
  }
};

class TextFieldActivity : public Activity {
 public:
  TextFieldActivity(ApplicationContext& context, TextFieldEditor& editor)
      : field(context, editor, font_body1(), "", roo_display::kLeft,
              TextField::NONE) {}

  Widget& getContents() override { return field; }
  BackResult onBackRequested(BackSource source) override {
    ++request_count;
    last_source = source;
    return BackResult::kUnhandled;
  }

  TextField field;
  int request_count = 0;
  BackSource last_source = BackSource::kProgrammatic;
};

class EscapeCountingTabs : public material3::Tabs {
 public:
  using material3::Tabs::Tabs;

  bool onKeyEvent(const KeyEvent& event) override {
    if (event.code == KeyCode::kEscape) ++escape_count;
    return material3::Tabs::onKeyEvent(event);
  }

  int escape_count = 0;
};

class TabsActivity : public Activity {
 public:
  explicit TabsActivity(ApplicationContext& context)
      : tabs(context), first(context, "First"), second(context, "Second") {
    tabs.addTab(first);
    tabs.addTab(second);
  }

  Widget& getContents() override { return tabs; }

  EscapeCountingTabs tabs;
  material3::Tab first;
  material3::Tab second;
};

// Verifies that acquisition is bounded to four four-event source reads and
// schedules an immediate follow-up only when the complete budget is consumed.
TEST(KeySource, ApplicationDrainsAtMostSixteenEventsPerTick) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  std::vector<KeyEvent> events(17, {KeyPhase::kDown, KeyCode::kTab, 0, 0});
  QueuedKeySource keys(std::move(events));
  Application app(&environment, display, keys, false);

  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  EXPECT_EQ(4, keys.drain_calls());
  EXPECT_EQ(1u, keys.remaining());
  EXPECT_EQ((std::vector<int>{4, 4, 4, 4}), keys.max_events());

  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
  EXPECT_EQ(5, keys.drain_calls());
  EXPECT_EQ(0u, keys.remaining());
}

// Verifies that an underfilled drain ends the current acquisition tick.
TEST(KeySource, ApplicationStopsAtTheFirstPartialBatch) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  QueuedKeySource keys({{KeyPhase::kUp, KeyCode::kEnter, 0, 0}});
  Application app(&environment, display, keys, false);

  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  EXPECT_EQ(1, keys.drain_calls());
  EXPECT_EQ(0u, keys.remaining());
  EXPECT_EQ((std::vector<int>{4}), keys.max_events());
}

TEST(KeySource, HardwareEscapeUsesFocusedTask) {
  roo::byte raster[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  QueuedKeySource keys({{KeyPhase::kDown, KeyCode::kEscape, 0, 0}});
  Application app(&environment, display, keys, false);
  Task* task = app.addTaskFullScreen();
  BackActivity root(app.context());
  BackActivity child(app.context());
  task->enterActivity(&root);
  task->enterActivity(&child);
  app.refresh();
  ASSERT_TRUE(child.contents().requestFocus());

  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  EXPECT_EQ(1u, task->activityCount());
  EXPECT_EQ(&root, task->currentActivity());
  EXPECT_EQ(1, child.request_count);
  EXPECT_EQ(BackSource::kEscapeKey, child.last_source);
  task->clear();
}

TEST(KeySource, HardwareBackDismissesTransientWithoutFocus) {
  roo::byte raster[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  QueuedKeySource keys({{KeyPhase::kDown, KeyCode::kBack, 0, 0}});
  Application app(&environment, display, keys, false);
  BackPresentation presentation;
  ASSERT_EQ(PresentationStartResult::kStarted,
            app.root().transient_presentation_slot().show(
                presentation, TransientPresentationPolicy(true, true)));

  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  EXPECT_EQ(1, presentation.detach_count);
  EXPECT_EQ(PresentationFinishReason::kBack, presentation.reason);
  EXPECT_FALSE(app.root().transient_presentation_slot().hasActivePresentation());
}

TEST(KeySource, UnhandledRootEscapeCancelsFocusedEditor) {
  roo::byte raster[64 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      64, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  QueuedKeySource keys({{KeyPhase::kDown, KeyCode::kEscape, 0, 0}});
  Application app(&environment, display, keys, false);
  Task* task = app.addTaskFullScreen();
  TextFieldActivity activity(app.context(), app.text_field_editor());
  task->enterActivity(&activity);
  app.refresh();
  ASSERT_TRUE(activity.field.requestFocus());
  ASSERT_TRUE(app.text_field_editor().isEdited(&activity.field));

  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  EXPECT_EQ(1, activity.request_count);
  EXPECT_EQ(BackSource::kEscapeKey, activity.last_source);
  EXPECT_FALSE(app.text_field_editor().isEdited(&activity.field));
  EXPECT_EQ(1u, task->activityCount());
  task->clear();
}

TEST(KeySource, UnhandledRootEscapeFromFocusedTabBubblesToTabHost) {
  roo::byte raster[64 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      64, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  QueuedKeySource keys({{KeyPhase::kDown, KeyCode::kEscape, 0, 0}});
  Application app(&environment, display, keys, false);
  Task* task = app.addTaskFullScreen();
  TabsActivity activity(app.context());
  task->enterActivity(&activity);
  app.refresh();
  ASSERT_TRUE(activity.first.requestFocus());

  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  EXPECT_EQ(1u, task->activityCount());
  EXPECT_EQ(&activity.first, app.context().focus().focused());
  EXPECT_EQ(1, activity.tabs.escape_count);
  task->clear();
}

}  // namespace
}  // namespace roo_windows
