#include <deque>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/destination.h"
#include "roo_windows/core/navigation_host.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

class QueuedKeySource final : public KeySource {
 public:
  explicit QueuedKeySource(std::initializer_list<KeyEvent> events)
      : events_(events) {}

  int drain(KeyEvent* out, int max_events) override {
    int count = 0;
    while (count < max_events && !events_.empty()) {
      out[count++] = events_.front();
      events_.pop_front();
    }
    return count;
  }

 private:
  std::deque<KeyEvent> events_;
};

class RecordingWidget final : public BasicWidget {
 public:
  explicit RecordingWidget(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(16, 16);
  }
  bool isFocusable() const override { return true; }
  bool isClickable() const override { return true; }
  bool onKeyEvent(const KeyEvent& event) override {
    if (key_count == 0) first_event = event;
    last_event = event;
    ++key_count;
    return event.code == KeyCode::kCharacter;
  }
  KeyEvent last_event{KeyPhase::kDown, KeyCode::kUnknown, 0, 0};
  KeyEvent first_event{KeyPhase::kDown, KeyCode::kUnknown, 0, 0};
  int key_count = 0;
};

class RecordingDestination final : public Destination {
 public:
  explicit RecordingDestination(ApplicationContext& context) : contents(context) {}

  Widget& getContents() override { return contents; }
  void onStop() override { ++stop_count; }

  RecordingWidget contents;
  int stop_count = 0;
};

class ColorDestination final : public Destination {
 public:
  explicit ColorDestination(ApplicationContext& context) : contents(context) {}
  Widget& getContents() override { return contents; }

 private:
  class Contents final : public BasicSurfaceWidget {
   public:
    explicit Contents(ApplicationContext& context) : BasicSurfaceWidget(context) {}
    roo_display::Color background() const override { return roo_display::color::Green; }
    void paint(PaintContext& ctx) const override { ctx.clear(); }
    Dimensions getSuggestedMinimumDimensions() const override {
      return Dimensions(16, 16);
    }
  } contents;
};

// Verifies a public refresh paints the destination tree and a deadline-interrupted
// frame remains resumable through the same public one-shot entry point.
TEST(DisplayRuntimeCharacterization, RefreshPaintsAndResumesInterruptedFrame) {
  roo::byte raster[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  NavigationHost navigation;
  Application app(&environment, display);
  ColorDestination destination(app.context());
  app.addUiTaskFullScreen(navigation);
  navigation.push(destination);

  EXPECT_FALSE(app.refresh(roo_time::Uptime::Start()));
  EXPECT_TRUE(app.refresh());
  int16_t x = 4;
  int16_t y = 4;
  roo_display::Color pixel;
  device.raster().readColors(&x, &y, 1, &pixel);
  EXPECT_EQ(roo_display::Argb4444().toArgbColor(
                roo_display::Argb4444().fromArgbColor(roo_display::color::Green)),
            pixel);
  navigation.clear();
}

// Verifies a scheduled application tick preserves the full key sample while
// retaining normal Enter Down/Up press-lifecycle semantics.
TEST(DisplayRuntimeCharacterization, ScheduledTickRoutesKeySamplesAndActivation) {
  roo::byte raster[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  QueuedKeySource keys({{KeyPhase::kDown, KeyCode::kCharacter,
                         kKeyModifierControl, U'x'},
                        {KeyPhase::kDown, KeyCode::kEnter, 0, 0},
                        {KeyPhase::kUp, KeyCode::kEnter, 0, 0}});
  NavigationHost navigation;
  Application app(&environment, display, keys, false);
  RecordingDestination destination(app.context());
  app.addUiTaskFullScreen(navigation);
  navigation.push(destination);
  ASSERT_TRUE(app.refresh());
  ASSERT_TRUE(destination.contents.requestFocus());

  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  EXPECT_EQ(3, destination.contents.key_count);
  EXPECT_EQ(KeyCode::kCharacter, destination.contents.first_event.code);
  EXPECT_EQ(kKeyModifierControl, destination.contents.first_event.modifiers);
  EXPECT_EQ(U'x', destination.contents.first_event.rune);
  EXPECT_EQ(KeyPhase::kUp, destination.contents.last_event.phase);
  EXPECT_EQ(KeyCode::kEnter, destination.contents.last_event.code);
  EXPECT_FALSE(destination.contents.isPressed());
  navigation.clear();
}

// Verifies application destruction detaches borrowed destinations after a
// ticker has been scheduled, before the caller destroys them.
TEST(DisplayRuntimeCharacterization, DestructionStopsBorrowedDestinations) {
  roo::byte raster[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  NavigationHost navigation;
  RecordingDestination* destination = nullptr;
  {
    Application app(&environment, display);
    destination = new RecordingDestination(app.context());
    app.addUiTaskFullScreen(navigation);
    navigation.push(*destination);
    app.start();
  }
  EXPECT_EQ(nullptr, destination->getUiTask());
  EXPECT_EQ(1, destination->stop_count);
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
  delete destination;
}

}  // namespace
}  // namespace roo_windows
