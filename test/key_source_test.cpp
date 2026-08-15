#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/destination.h"
#include "roo_windows/core/navigation_host.h"
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
  void push(KeyEvent event) {
    events_.push_back(event);
    notifyReady();
  }

 private:
  bool hasPendingEvents() const override { return next_ < events_.size(); }

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

class KeyRecordingWidget : public BasicWidget {
 public:
  explicit KeyRecordingWidget(ApplicationContext& context) : BasicWidget(context) {}

  bool isFocusable() const override { return true; }
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }
  bool onKeyEvent(const KeyEvent& event) override {
    ++key_count;
    last_key = event.code;
    return true;
  }

  int key_count = 0;
  KeyCode last_key = KeyCode::kUnknown;
};

class BackDestination : public Destination {
 public:
  explicit BackDestination(ApplicationContext& context) : contents_(context) {}

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

class TextFieldDestination : public Destination {
 public:
  TextFieldDestination(ApplicationContext& context, TextFieldEditor& editor)
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

class TabsDestination : public Destination {
 public:
  explicit TabsDestination(ApplicationContext& context)
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

TEST(KeySource, RoutesEachSourceToItsDeclaredTask) {
  roo::byte raster[32 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  KeyRecordingWidget first_contents(app.context());
  KeyRecordingWidget second_contents(app.context());
  Task& first = app.addTaskFullScreen(first_contents);
  Task& second = app.addTaskFullScreen(second_contents);
  QueuedKeySource first_keys({{KeyPhase::kDown, KeyCode::kCharacter, 0, 0}});
  QueuedKeySource second_keys({{KeyPhase::kDown, KeyCode::kTab, 0, 0}});
  first_keys.connect(first);
  second_keys.connect(second);
  app.refresh();
  ASSERT_TRUE(first_contents.requestFocus());
  ASSERT_TRUE(second_contents.requestFocus());

  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  EXPECT_EQ(1, first_contents.key_count);
  EXPECT_EQ(KeyCode::kCharacter, first_contents.last_key);
  EXPECT_EQ(1, second_contents.key_count);
  EXPECT_EQ(KeyCode::kTab, second_contents.last_key);
}

TEST(KeySource, ReadySourceWakesItsDestinationApplication) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  KeyRecordingWidget contents(app.context());
  Task& task = app.addTaskFullScreen(contents);
  QueuedKeySource keys({});
  keys.connect(task);
  app.refresh();
  ASSERT_TRUE(contents.requestFocus());
  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  keys.push({KeyPhase::kDown, KeyCode::kCharacter, 0, 0});
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  EXPECT_EQ(1, contents.key_count);
  EXPECT_EQ(KeyCode::kCharacter, contents.last_key);
}

TEST(KeySource, HardwareEscapeUsesFocusedTask) {
  roo::byte raster[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  QueuedKeySource keys({{KeyPhase::kDown, KeyCode::kEscape, 0, 0}});
  NavigationHost navigation;
  Application app(&environment, display, keys, false);
  Task& task = app.addTaskFullScreen(navigation);
  BackDestination root(app.context());
  BackDestination child(app.context());
  navigation.push(root);
  navigation.push(child);
  app.refresh();
  ASSERT_TRUE(child.contents().requestFocus());

  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  EXPECT_EQ(1u, navigation.depth());
  EXPECT_EQ(1, child.request_count);
  EXPECT_EQ(BackSource::kEscapeKey, child.last_source);
  navigation.clear();
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
  EXPECT_FALSE(
      app.root().transient_presentation_slot().hasActivePresentation());
}

TEST(KeySource, UnhandledRootEscapeCancelsFocusedEditor) {
  roo::byte raster[64 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      64, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  QueuedKeySource keys({{KeyPhase::kDown, KeyCode::kEscape, 0, 0}});
  NavigationHost navigation;
  Application app(&environment, display, keys, false);
  Task& task = app.addTaskFullScreen(navigation);
  TextFieldDestination destination(app.context(), task.textFieldEditor());
  navigation.push(destination);
  app.refresh();
  ASSERT_TRUE(destination.field.requestFocus());
  ASSERT_TRUE(task.textFieldEditor().isEdited(&destination.field));

  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  EXPECT_EQ(1, destination.request_count);
  EXPECT_EQ(BackSource::kEscapeKey, destination.last_source);
  EXPECT_FALSE(task.textFieldEditor().isEdited(&destination.field));
  EXPECT_EQ(1u, navigation.depth());
  navigation.clear();
}

TEST(KeySource, UnhandledRootEscapeFromFocusedTabBubblesToTabHost) {
  roo::byte raster[64 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      64, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  QueuedKeySource keys({{KeyPhase::kDown, KeyCode::kEscape, 0, 0}});
  NavigationHost navigation;
  Application app(&environment, display, keys, false);
  Task& task = app.addTaskFullScreen(navigation);
  TabsDestination destination(app.context());
  navigation.push(destination);
  app.refresh();
  ASSERT_TRUE(destination.first.requestFocus());

  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  EXPECT_EQ(1u, navigation.depth());
  EXPECT_EQ(&destination.first, task.focus().focused());
  EXPECT_EQ(1, destination.tabs.escape_count);
  navigation.clear();
}

}  // namespace
}  // namespace roo_windows
