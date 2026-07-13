#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"

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

}  // namespace
}  // namespace roo_windows
