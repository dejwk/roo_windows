#include <memory>

#include "gtest/gtest.h"
#include "roo_windows/material2/dialogs/alert_dialog.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows::material2 {
namespace {
class AlertHelperTest
    : public test_support::RooWindowsRenderTestSized<320, 240> {};

// Verifies selected button delivery occurs after detachment and the helper
// destroys its dialog and handlers after the callback returns.
TEST_F(AlertHelperTest, SelectedActionDetachesThenDeletesDialog) {
  Task& owner = app_.addTaskFullScreen();
  refresh();
  int calls = 0;
  int selected = -2;
  Widget* root = nullptr;
  auto marker = std::make_shared<int>(1);
  std::weak_ptr<int> lifetime = marker;
  ASSERT_EQ(PresentationStartResult::kStarted,
            ShowAlertDialog(owner, "Continue?", "Apply the new settings?",
                            {"Cancel", "Continue"}, [&](int id) {
                              ++calls;
                              selected = id;
                              EXPECT_EQ(nullptr, root->parent());
                              EXPECT_FALSE(app_.root()
                                               .transient_presentation_slot()
                                               .hasActivePresentation());
                              EXPECT_FALSE(lifetime.expired());
                            }));
  root = owner.focus().scopeRoot();
  root->setOnInteractiveChange([marker]() {});
  marker.reset();
  refresh();
  Widget* action = nullptr;
  for (int i = 0; i < 4; ++i) {
    ASSERT_TRUE(owner.focus().moveFocus(*root, false));
    auto* button = static_cast<SimpleButton*>(owner.focus().focused());
    if (button->label() == "Continue") {
      action = button;
      break;
    }
  }
  ASSERT_NE(nullptr, action);
  action->onClicked();
  EXPECT_EQ(1, calls);
  EXPECT_EQ(1, selected);
  EXPECT_TRUE(lifetime.expired());
}

// Verifies busy admission neither delivers completion nor replaces the current
// alert; rejected callback storage is released and Back cleans up the first.
TEST_F(AlertHelperTest, BusyAdmissionReleasesRequest) {
  Task& owner = app_.addTaskFullScreen();
  refresh();
  int first_calls = 0;
  ASSERT_EQ(
      PresentationStartResult::kStarted,
      ShowAlertDialog(owner, "First", "Already open", {"OK"}, [&](int id) {
        EXPECT_EQ(-1, id);
        ++first_calls;
      }));
  Widget* first = owner.focus().scopeRoot();
  auto marker = std::make_shared<int>(1);
  std::weak_ptr<int> lifetime = marker;
  EXPECT_EQ(PresentationStartResult::kHostBusy,
            ShowAlertDialog(owner, "Second", "Rejected", {"OK"}, [marker](int) {
              ADD_FAILURE() << "Rejected alert must not complete";
            }));
  marker.reset();
  EXPECT_TRUE(lifetime.expired());
  EXPECT_EQ(first, owner.focus().scopeRoot());
  EXPECT_EQ(BackResult::kHandled, owner.requestBack());
  EXPECT_EQ(1, first_calls);
}

// Verifies completion may open a replacement alert in the now-idle slot;
// deleting the old heap-owned dialog must not remove the new presentation.
TEST_F(AlertHelperTest, CompletionMayOpenNextAlert) {
  Task& owner = app_.addTaskFullScreen();
  refresh();
  int calls = 0;
  ASSERT_EQ(
      PresentationStartResult::kStarted,
      ShowAlertDialog(owner, "First", "Close to continue", {"OK"}, [&](int) {
        ++calls;
        EXPECT_EQ(PresentationStartResult::kStarted,
                  ShowAlertDialog(owner, "Next", "Replacement alert", {"OK"},
                                  nullptr));
      }));
  EXPECT_EQ(BackResult::kHandled, owner.requestBack());
  EXPECT_EQ(1, calls);
  EXPECT_TRUE(
      app_.root().transient_presentation_slot().hasActivePresentation());
  EXPECT_EQ(BackResult::kHandled, owner.requestBack());
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
}

// Verifies application teardown finishes the heap-owned alert exactly once;
// the sanitizer run checks the dialog and its child storage are also released.
TEST_F(AlertHelperTest, ApplicationTeardownCompletesAndCleansUp) {
  int calls = 0;
  auto marker = std::make_shared<int>(1);
  std::weak_ptr<int> lifetime = marker;
  {
    Application application(&env_, display_);
    Task& owner = application.addTaskFullScreen();
    application.refresh();
    ASSERT_EQ(PresentationStartResult::kStarted,
              ShowAlertDialog(owner, "Open", "Pending at shutdown", {"OK"},
                              [marker, &calls](int id) {
                                EXPECT_EQ(-1, id);
                                ++calls;
                              }));
    marker.reset();
    EXPECT_FALSE(lifetime.expired());
  }
  EXPECT_EQ(1, calls);
  EXPECT_TRUE(lifetime.expired());
}
}  // namespace
}  // namespace roo_windows::material2
