#include "roo_windows.h"

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/aligned_layout.h"
#include "roo_windows/containers/holder.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/containers/navigation_panel.h"
#include "roo_windows/containers/navigation_rail.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/stacked_layout.h"
#include "roo_windows/containers/static_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/dialogs/alert_dialog.h"
#include "roo_windows/dialogs/radio_list_dialog.h"
#include "roo_windows/widgets/blank.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows/widgets/checkbox.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/icon_with_caption.h"
#include "roo_windows/widgets/image.h"
#include "roo_windows/widgets/progress_bar.h"
#include "roo_windows/widgets/radio_button.h"
#include "roo_windows/widgets/slider.h"
#include "roo_windows/widgets/switch.h"
#include "roo_windows/widgets/text_field.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows/widgets/toggle_buttons.h"
#include "roo_windows/core/press_overlay.h"

using namespace roo_display;
using namespace roo_windows;

namespace roo_windows {

TEST(Windows, BasicCompilation) {
  roo::byte raster[320 * 240 * 2];
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
}

TEST(PressOverlay, WideTopStripCrossingCenterIsNotUniformTransparent) {
  PressOverlay overlay(0, 0, 10, color::Red);

  Color uniform;
  EXPECT_FALSE(overlay.readUniformColorRect(-11, -9, 11, -9, &uniform));

  Color colors[23];
  EXPECT_FALSE(overlay.readColorRect(-11, -9, 11, -9, colors));
  EXPECT_NE(colors[11], color::Transparent);
}

}  // namespace roo_windows