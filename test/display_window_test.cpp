#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

// Verifies the application-owned window borrows the constructor display and
// compatibility accessors preserve the same root and gesture instances.
TEST(DisplayWindow, OwnsDisplayLocalRuntimeAndCompatibilityForwarders) {
  roo::byte raster[32 * 24 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 24, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);

  EXPECT_EQ(&display, &app.window().display());
  EXPECT_EQ(&app.window().root(), &app.root());
  EXPECT_EQ(&app.window().gestureDetector(), &app.gesture_detector());
  EXPECT_EQ(display.extents(), app.window().root().bounds().asBox());
  EXPECT_EQ(app.window().refresh(), app.refresh());
}

}  // namespace
}  // namespace roo_windows
