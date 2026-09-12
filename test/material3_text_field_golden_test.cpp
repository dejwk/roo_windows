#include <algorithm>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/text_field/secure_text_field.h"
#include "roo_windows/material3/text_field/text_field.h"

namespace roo_windows::material3 {
namespace {
TEST(Material3TextFieldGolden, VariantsAndStates) {
  for (bool outlined : {false, true})
    for (int state = 0; state < 13; ++state) {
      const int height = std::max(110, Scaled(100));
      std::vector<roo::byte> raster(280 * height * 2);
      roo_display::OffscreenDevice<roo_display::Argb4444> device(
          280, height, raster.data(), roo_display::Argb4444());
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
      if (state == 5 || state == 12) field.setErrorText("Account not found");
      if (state == 12) field.setEnabled(false);
      if (state == 10) field.setHover(true);
      if (state == 11) field.setPressed(true);
      if (state == 9)
        field.setText("A long account name that scrolls beyond the viewport");
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
      if (state == 2 || state == 8 || state == 9) {
        KeyEvent key;
        key.code = KeyCode::kEnter;
        key.phase = KeyPhase::kDown;
        field.requestFocus();
        field.onKeyEvent(key);
        if (state == 8) task.textFieldEditor().setSelection(2, 7);
      }
      ASSERT_TRUE(app.refresh());
      std::string name = std::string(outlined ? "outlined_" : "filled_") +
                         std::to_string(state);
      if (ROO_WINDOWS_ZOOM != 100)
        name += "_" + std::to_string(ROO_WINDOWS_ZOOM);
      auto capture =
          ::roo_windows::test::CaptureRgb(device.raster(), 0, 0, 280, height);
      EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
          capture, "test/goldens/material3_text_field/" + name + ".ppm",
          "text_field_" + name));
      task.navigation().clear();
    }
}
}  // namespace
TEST(Material3TextFieldGolden, SecureMaskAndReveal) {
  for (bool revealed : {false, true})
    for (bool rtl : {false, true}) {
      const int height = std::max(90, Scaled(90));
      std::vector<roo::byte> raster(240 * height * 2);
      roo_display::OffscreenDevice<roo_display::Argb4444> device(
          240, height, raster.data(), roo_display::Argb4444());
      roo_display::Display display(device);
      roo_scheduler::Scheduler scheduler;
      Environment env(scheduler);
      Application app(&env, display);
      SecureTextField field(app.context(), "Password",
                            TextFieldVariant::kOutlined);
      field.setText(u8"aé secret");
      field.setRevealed(revealed);
      field.setErrorText("Check password");
      if (rtl) field.setLayoutDirection(LayoutDirection::kRightToLeft);
      Task& task = app.addTaskFullScreen(field);
      ASSERT_TRUE(app.refresh());
      std::string name = std::string("secure_") +
                         (revealed ? "revealed" : "masked") +
                         (rtl ? "_rtl" : "_ltr");
      if (ROO_WINDOWS_ZOOM != 100)
        name += "_" + std::to_string(ROO_WINDOWS_ZOOM);
      EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
          ::roo_windows::test::CaptureRgb(device.raster(), 0, 0, 240, height),
          "test/goldens/material3_text_field/" + name + ".ppm", name));
      task.navigation().clear();
    }
}
}  // namespace roo_windows::material3
