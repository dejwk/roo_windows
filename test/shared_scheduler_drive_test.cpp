#include "gtest/gtest.h"

#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/navigation_host.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows {
namespace {

class OneKeySource final : public KeySource {
 public:
  int drain(KeyEvent* out, int max_events) override {
    if (delivered_ || max_events == 0) return 0;
    out[0] = {KeyPhase::kDown, KeyCode::kTab, 0, 0};
    delivered_ = true;
    return 1;
  }

  bool delivered() const { return delivered_; }

 private:
  bool hasPendingEvents() const override { return !delivered_; }

  bool delivered_ = false;
};

class TextInputDestination final : public Destination {
 public:
  TextInputDestination(ApplicationContext& context, TextFieldEditor& editor)
      : field(context, editor, font_body1(), "", roo_display::kLeft,
              TextField::NONE) {}

  Widget& getContents() override { return field; }

  TextField field;
};

TEST(SharedSchedulerDrive, StartedApplicationsEachReceiveTheirCallback) {
  roo::byte first_raster[16 * 16 * 2] = {};
  roo::byte second_raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> first_device(
      16, 16, first_raster, roo_display::Argb4444());
  roo_display::OffscreenDevice<roo_display::Argb4444> second_device(
      16, 16, second_raster, roo_display::Argb4444());
  roo_display::Display first_display(first_device);
  roo_display::Display second_display(second_device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  OneKeySource first_keys;
  OneKeySource second_keys;
  Application first(&environment, first_display, first_keys, false);
  Application second(&environment, second_display, second_keys, false);

  first.start();
  second.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 2);

  EXPECT_TRUE(first_keys.delivered());
  EXPECT_TRUE(second_keys.delivered());
}

TEST(SharedSchedulerDrive, KeyboardCanTargetAnotherApplicationEditor) {
  roo::byte source_raster[64 * 64 * 2] = {};
  roo::byte destination_raster[64 * 64 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> source_device(
      64, 64, source_raster, roo_display::Argb4444());
  roo_display::OffscreenDevice<roo_display::Argb4444> destination_device(
      64, 64, destination_raster, roo_display::Argb4444());
  roo_display::Display source_display(source_device);
  roo_display::Display destination_display(destination_device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  NavigationHost destination_navigation;
  Application source(&environment, source_display);
  Application destination(&environment, destination_display);
  Task& destination_task =
      destination.addTaskFullScreen(destination_navigation);
  TextInputDestination target(destination.context(),
                              destination_task.textFieldEditor());
  destination_navigation.push(target);

  source.start();
  destination.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 2);

  ASSERT_TRUE(destination.refresh());
  target.field.edit();
  EXPECT_TRUE(destination.refresh());

  // The source keyboard remains a source-owned presenter, while its semantic
  // operations synchronously target the destination application's editor.
  source.keyboard().connect(destination);
  ASSERT_TRUE(source.refresh());
  source.keyboard().show();
  EXPECT_TRUE(source.refresh());
  Panel& keyboard_contents =
      static_cast<Panel&>(source.keyboard().getContents());
  Panel& letter_page = static_cast<Panel&>(keyboard_contents.child_at(0));
  Widget& q_key = letter_page.child_at(0);
  q_key.onSingleTapUp(0, 0);

  EXPECT_EQ("q", target.field.content());
  destination_navigation.clear();
}

}  // namespace
}  // namespace roo_windows
