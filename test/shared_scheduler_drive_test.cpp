#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"

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

}  // namespace
}  // namespace roo_windows
