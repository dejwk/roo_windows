#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/text_field/text_field.h"

namespace roo_windows::material3 {
namespace {
TEST(Material3TextFieldGolden, VariantsAndStates) {
  for (bool outlined : {false, true})
    for (int state = 0; state < 8; ++state) {
      std::vector<roo::byte> raster(280 * 110 * 2);
      roo_display::OffscreenDevice<roo_display::Argb4444> device(
          280, 110, raster.data(), roo_display::Argb4444());
      roo_display::Display display(device);
      roo_scheduler::Scheduler scheduler;
      Environment env(scheduler);
      Application app(&env, display);
      TextField field(
          app.context(), "Account",
          outlined ? TextFieldVariant::kOutlined : TextFieldVariant::kFilled);
      field.setSupportingText("Use your account name");
      if (state >= 3) field.setText("sample account");
      if (state == 4) field.setEnabled(false);
      if (state == 5) field.setErrorText("Account not found");
      if (state == 6 || state == 7) {
        field.setPrefixText("@ ");
        field.setSuffixText(" .net");
        field.setErrorText(
            "A long error message that must end with an ellipsis");
      }
      if (state == 7) field.setLayoutDirection(LayoutDirection::kRightToLeft);
      Task& task = app.addTaskFullScreen(field);
      ASSERT_TRUE(app.refresh());
      if (state == 1) field.requestFocus();
      if (state == 2) {
        KeyEvent key;
        key.code = KeyCode::kEnter;
        key.phase = KeyPhase::kDown;
        field.requestFocus();
        field.onKeyEvent(key);
      }
      ASSERT_TRUE(app.refresh());
      std::string name = std::string(outlined ? "outlined_" : "filled_") +
                         std::to_string(state);
      auto capture =
          ::roo_windows::test::CaptureRgb(device.raster(), 0, 0, 280, 110);
      EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
          capture, "test/goldens/material3_text_field/" + name + ".ppm",
          "text_field_" + name));
      task.navigation().clear();
    }
}
}  // namespace
}  // namespace roo_windows::material3
