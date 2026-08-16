#include "roo_windows/core/application.h"

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/destination.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/navigation_host.h"
#include "roo_windows/core/transient_presentation.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows {
namespace {

class TestWidget : public BasicWidget {
 public:
  explicit TestWidget(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }
};

class EmptyKeySource final : public KeySource {
 public:
  int drain(KeyEvent* out, int max_events) override { return 0; }

 private:
  bool hasPendingEvents() const override { return false; }
};

class TestDestination : public Destination {
 public:
  explicit TestDestination(ApplicationContext& context) : contents_(context) {}

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

class TextInputDestination : public Destination {
 public:
  explicit TextInputDestination(ApplicationContext& context)
      : field(context, font_body1(), "", roo_display::kLeft,
              TextField::NONE) {}

  Widget& getContents() override { return field; }

  TextField field;
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

TEST(Application, StartIsSingleUse) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  EmptyKeySource keys;
  Application app(&environment, display, keys, false);

  EXPECT_DEATH_IF_SUPPORTED(
      {
        app.start();
        app.start();
      },
      "");
}

// Verifies that task-local Back only changes its own navigation history.
TEST(Application, RequestBackUsesExplicitTargetTask) {
  roo::byte raster[64 * 64 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      64, 64, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  NavigationHost first_navigation;
  NavigationHost second_navigation;
  Application app(&environment, display);
  app.addTaskFullScreen(first_navigation);
  Task& second_task = app.addTaskFullScreen(second_navigation);
  TestDestination first_root(app.context());
  TestDestination first_child(app.context());
  TestDestination second_root(app.context());
  TestDestination second_child(app.context());
  first_navigation.push(first_root);
  first_navigation.push(first_child);
  second_navigation.push(second_root);
  second_navigation.push(second_child);

  EXPECT_EQ(BackResult::kHandled,
            second_task.requestBack(BackSource::kEscapeKey));
  EXPECT_EQ(2u, first_navigation.depth());
  EXPECT_EQ(1u, second_navigation.depth());
  EXPECT_EQ(0, first_child.back_request_count);
  EXPECT_EQ(1, second_child.back_request_count);
  EXPECT_EQ(BackSource::kEscapeKey, second_child.last_source);

  first_navigation.clear();
  second_navigation.clear();
}

// Verifies an eligible window presentation receives Back before its task.
TEST(Application, RequestBackPrioritizesTransientPresentation) {
  roo::byte raster[64 * 64 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      64, 64, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  NavigationHost navigation;
  Application app(&environment, display);
  app.addTaskFullScreen(navigation);
  TestDestination root(app.context());
  TestDestination child(app.context());
  navigation.push(root);
  navigation.push(child);
  BackPresentation presentation;

  EXPECT_EQ(PresentationStartResult::kStarted,
            app.root().transient_presentation_slot().show(
                presentation, TransientPresentationPolicy(true)));
  EXPECT_EQ(BackResult::kHandled,
            task.requestBack(BackSource::kNavigationButton));
  EXPECT_EQ(1, presentation.back_request_count);
  EXPECT_EQ(BackSource::kNavigationButton, presentation.last_source);
  EXPECT_EQ(2u, navigation.depth());
  EXPECT_EQ(0, child.back_request_count);

  navigation.clear();
}

TEST(Application, TextInputEmitterTargetsTheActiveEditor) {
  roo::byte raster[64 * 64 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      64, 64, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  NavigationHost navigation;
  Application app(&environment, display);
  Task& task = app.addTaskFullScreen(navigation);
  TextInputDestination destination(app.context());
  navigation.push(destination);
  TextInputEmitter emitter;

  EXPECT_FALSE(emitter.commitRune(U'A'));
  emitter.connect(app);
  EXPECT_FALSE(emitter.commitRune(U'A'));
  EXPECT_FALSE(emitter.deleteBackward());

  destination.field.edit();
  EXPECT_FALSE(emitter.commitRune(0xd800));
  EXPECT_TRUE(emitter.commitRune(U'A'));
  EXPECT_EQ("A", destination.field.content());
  EXPECT_TRUE(emitter.deleteBackward());
  EXPECT_EQ("", destination.field.content());
  EXPECT_TRUE(emitter.performAction(TextInputAction::kDone));
  EXPECT_FALSE(emitter.commitRune(U'B'));

  navigation.clear();
}

TEST(Application, TextInputActivationReplacesThePreviousEditor) {
  roo::byte raster[64 * 64 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      64, 64, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  NavigationHost first_navigation;
  NavigationHost second_navigation;
  Application app(&environment, display);
  app.addTaskFullScreen(first_navigation);
  app.addTaskFullScreen(second_navigation);
  TextInputDestination first(app.context());
  TextInputDestination second(app.context());
  first_navigation.push(first);
  second_navigation.push(second);
  TextInputEmitter emitter;
  emitter.connect(app);

  first.field.edit();
  ASSERT_TRUE(emitter.commitRune(U'A'));
  second.field.edit();
  EXPECT_TRUE(emitter.commitRune(U'B'));
  EXPECT_EQ("A", first.field.content());
  EXPECT_EQ("B", second.field.content());

  first_navigation.clear();
  second_navigation.clear();
}

TEST(Application, TextInputEmitterOutlivesItsDestination) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  TextInputEmitter emitter;
  {
    Application app(&environment, display);
    emitter.connect(app);
    ASSERT_TRUE(emitter.isConnected());
  }
  EXPECT_FALSE(emitter.isConnected());
  EXPECT_FALSE(emitter.performAction(TextInputAction::kDone));
}

}  // namespace
}  // namespace roo_windows
