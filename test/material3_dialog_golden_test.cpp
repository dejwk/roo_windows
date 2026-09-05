#include <memory>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/alert.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_surface_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/dialog/basic_dialog.h"
#include "roo_windows/material3/dialog/full_screen_dialog.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_block.h"

namespace roo_windows::material3 {
namespace {

class Backdrop final : public BasicSurfaceWidget {
 public:
  explicit Backdrop(ApplicationContext& context)
      : BasicSurfaceWidget(context) {}

  Color background() const override { return Color(0xFFE8E4DC); }
  void paint(PaintContext& ctx) const override { ctx.clear(); }
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(320, 240);
  }
};

class Material3DialogGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 320;
  static constexpr int16_t kHeight = 240;

  Material3DialogGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        environment_(scheduler_),
        app_(&environment_, display_),
        backdrop_(app_.context()),
        owner_(app_.addTaskFullScreen(backdrop_)) {}

  roo_display::Offscreen<roo_display::Rgb888> capture() const {
    return ::roo_windows::test::CaptureRgb(offscreen_.raster(), 0, 0, kWidth,
                                           kHeight);
  }

  roo::byte raster_[kWidth * kHeight * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_;
  Application app_;
  Backdrop backdrop_;
  Task& owner_;
};

TEST_F(Material3DialogGoldenTest, BasicDialogWithIconAndActions) {
  TextBlock body(app_.context(),
                 "Changing this setting restarts the pool controller.",
                 text_style_body_medium());
  DialogActionSpec actions[] = {
      {1, "Cancel", DialogActionRole::kDismiss},
      {2, "Restart", DialogActionRole::kConfirm},
  };
  BasicDialog dialog(app_.context(), WidgetRef(body), actions, 2);
  dialog.setIcon(&ic_outlined_24_alert_warning());
  dialog.setHeadline("Restart controller?");
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  ASSERT_TRUE(app_.refresh());

  EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
      capture(), "test/goldens/material3_dialog/basic_icon_actions.ppm",
      "material3_dialog_basic_icon_actions"));
}

TEST_F(Material3DialogGoldenTest, AlertDialogOwnedProse) {
  DialogActionSpec action{1, "Got it", DialogActionRole::kAcknowledge};
  AlertDialog dialog(app_.context(), "Water level warning",
                     "The fill valve has remained open longer than expected.",
                     &action, 1);
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  ASSERT_TRUE(app_.refresh());

  EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
      capture(), "test/goldens/material3_dialog/alert_owned_prose.ppm",
      "material3_dialog_alert_owned_prose"));
}

TEST_F(Material3DialogGoldenTest, FullScreenDialogHeader) {
  TextBlock body(app_.context(),
                 "Step 2 of 3\n\nChoose the filtration schedule to apply after "
                 "the maintenance window.",
                 text_style_body_large());
  FullScreenDialog dialog(app_.context(), WidgetRef(body));
  dialog.setHeaderTitle("Edit schedule");
  dialog.setConfirmAction({7, "Save", DialogActionRole::kConfirm, true});
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  ASSERT_TRUE(app_.refresh());

  EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
      capture(), "test/goldens/material3_dialog/full_screen_header.ppm",
      "material3_dialog_full_screen_header"));
}

TEST_F(Material3DialogGoldenTest, FullScreenDialogRtlHeader) {
  TextBlock body(app_.context(), "RTL full-screen content",
                 text_style_body_large());
  FullScreenDialog dialog(app_.context(), WidgetRef(body));
  dialog.setHeaderTitle("Schedule");
  dialog.setConfirmAction({7, "Save", DialogActionRole::kConfirm, true});
  dialog.setLayoutDirection(LayoutDirection::kRightToLeft);
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  ASSERT_TRUE(app_.refresh());

  EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
      capture(), "test/goldens/material3_dialog/full_screen_rtl_header.ppm",
      "material3_dialog_full_screen_rtl_header"));
}

}  // namespace
}  // namespace roo_windows::material3
