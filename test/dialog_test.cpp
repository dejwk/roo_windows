#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/dialogs/dialog.h"

namespace roo_windows {
namespace {

class TestContent final : public BasicWidget {
 public:
  explicit TestContent(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }
};

class TestDialog final : public Dialog {
 public:
  explicit TestDialog(ApplicationContext& context)
      : Dialog(context, {"Cancel", "OK"}) {}

  void chooseFirstAction() { actionTaken(0); }
  void setContent(WidgetRef content) {
    setPresentationContent(std::move(content));
  }

  int enter_count = 0;
  int exit_count = 0;
  int exit_result = -2;

 protected:
  void onEnter() override { ++enter_count; }
  void onExit(int result) override {
    ++exit_count;
    exit_result = result;
  }
};

class DialogTest : public ::testing::Test {
 protected:
  DialogTest()
      : device_(64, 64, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_) {}

  roo::byte raster_[64 * 64 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_;
  Application app_;
};

// Verifies dialog content is detached and the slot is clear before completion.
TEST_F(DialogTest, CloseDetachesContentBeforeCompletion) {
  TestDialog dialog(app_.context());
  TestContent content(app_.context());
  dialog.setContent(content);
  bool content_detached = false;
  bool slot_empty = false;

  EXPECT_EQ(PresentationStartResult::kStarted,
            app_.showDialog(dialog, [&](int result) {
              EXPECT_EQ(-1, result);
              content_detached = content.parent() == nullptr;
              slot_empty = !app_.root()
                                .transient_presentation_slot()
                                .hasActivePresentation();
            }));
  EXPECT_NE(nullptr, content.parent());

  app_.clearDialog();

  EXPECT_TRUE(content_detached);
  EXPECT_TRUE(slot_empty);
  EXPECT_EQ(nullptr, content.parent());
  EXPECT_EQ(1, dialog.exit_count);
  EXPECT_EQ(-1, dialog.exit_result);
}

// Verifies dialog actions finish the registered presentation exactly once.
TEST_F(DialogTest, ActionCompletesAndReleasesTheSlot) {
  TestDialog dialog(app_.context());

  EXPECT_EQ(PresentationStartResult::kStarted,
            app_.showDialog(dialog, nullptr));
  dialog.chooseFirstAction();

  EXPECT_EQ(1, dialog.exit_count);
  EXPECT_EQ(0, dialog.exit_result);
  EXPECT_FALSE(app_.root().transient_presentation_slot().hasActivePresentation());
}

// Verifies a busy dialog host leaves a second presenter untouched.
TEST_F(DialogTest, ShowRejectsOccupiedTransientSlot) {
  TestDialog first(app_.context());
  TestDialog second(app_.context());

  EXPECT_EQ(PresentationStartResult::kStarted,
            app_.showDialog(first, nullptr));
  EXPECT_EQ(PresentationStartResult::kHostBusy,
            app_.showDialog(second, nullptr));
  app_.clearDialog();
  EXPECT_EQ(1, first.exit_count);
  EXPECT_EQ(0, second.exit_count);
}

// Verifies dialog completion may immediately open the next root presentation.
TEST_F(DialogTest, CompletionCanShowAnotherDialog) {
  TestDialog first(app_.context());
  TestDialog second(app_.context());
  PresentationStartResult second_result = PresentationStartResult::kHostBusy;

  EXPECT_EQ(PresentationStartResult::kStarted,
            app_.showDialog(first, [&](int result) {
              second_result = app_.showDialog(second, nullptr);
            }));
  app_.clearDialog();

  EXPECT_EQ(PresentationStartResult::kStarted, second_result);
  EXPECT_TRUE(app_.root().transient_presentation_slot().hasActivePresentation());
  app_.clearDialog();
  EXPECT_EQ(1, second.exit_count);
}

// Verifies destroying a visible caller-owned dialog detaches it without callback.
TEST_F(DialogTest, DestructionCancelsWithoutCompletion) {
  int callback_count = 0;
  TestDialog* dialog = new TestDialog(app_.context());
  EXPECT_EQ(PresentationStartResult::kStarted,
            app_.showDialog(*dialog, [&](int result) { ++callback_count; }));

  delete dialog;

  EXPECT_EQ(0, callback_count);
  EXPECT_FALSE(app_.root().transient_presentation_slot().hasActivePresentation());
}

}  // namespace
}  // namespace roo_windows
