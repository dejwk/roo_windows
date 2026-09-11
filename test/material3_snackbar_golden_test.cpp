#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/snackbar/snackbar.h"

namespace roo_windows::material3 {
namespace {
// Verifies Material surfaces, wrapped text, controls, RTL and wide placement.
TEST(Material3SnackbarGolden, Configurations) {
  struct Case {
    const char* name;
    const char* text;
    const char* action;
    bool close;
    bool rtl;
    int width;
  };
  const Case cases[] = {
      {"short", "Settings saved", "", false, false, 320},
      {"two_lines",
       "Controller settings were saved. Changes apply at the next scheduled "
       "cycle.",
       "", false, false, 320},
      {"action", "Settings saved", "Undo", false, false, 320},
      {"close", "Schedule restored", "", true, false, 320},
      {"stacked", "Connection settings saved", "View connection", true, false,
       240},
      {"rtl", "Schedule restored", "Undo", true, true, 320},
      {"wide", "Controller settings saved", "Undo", false, false, 720},
  };
  for (const Case& c : cases) {
    std::vector<roo::byte> raster(c.width * 240 * 2);
    roo_display::OffscreenDevice<roo_display::Argb4444> device(
        c.width, 240, raster.data(), roo_display::Argb4444());
    roo_display::Display display(device);
    roo_scheduler::Scheduler scheduler;
    Environment env(scheduler);
    Application app(&env, display);
    SnackbarHost host(app.context());
    Button body(app.context(), "Settings", ButtonVariant::kText);
    host.setBody(body);
    Task& task = app.addTaskFullScreen(host);
    host.setLayoutDirection(c.rtl ? LayoutDirection::kRightToLeft
                                  : LayoutDirection::kLeftToRight);
    host.snackbars().setAnimationsEnabled(false);
    SnackbarRequest request;
    ASSERT_TRUE(request.configure(c.text, c.action,
                                  SnackbarDuration::kPersistent, c.close));
    ASSERT_TRUE(app.refresh());
    ASSERT_EQ(SnackbarShowResult::kShown, host.snackbars().show(request));
    ASSERT_TRUE(app.refresh());
    auto capture =
        ::roo_windows::test::CaptureRgb(device.raster(), 0, 0, c.width, 240);
    EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
        capture,
        std::string("test/goldens/material3_snackbar/") + c.name + ".ppm",
        std::string("material3_snackbar_") + c.name));
    task.navigation().clear();
    host.setBody(WidgetRef());
  }
}
}  // namespace
}  // namespace roo_windows::material3
